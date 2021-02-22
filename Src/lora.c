/*
 * lora.c
 *
 *  Created on: 08.02.2021
 *      Author: Bartosz
 */

#include "lora.h"

const uint8_t new_line_character = 0x0A;
const uint8_t carriage_return_character = 0x0D;
const char* ok_config_response = "OK";
static const char* config_lora_command[] = {
		"AT+SPR=9",
		"AT+SYNL=8",
		"AT+SYNW=3426578564721137",
		"AT+LRCRC=1",
		"AT+LRSBW=9",
		"AT+LRSF=9",
		"AT+POWER=0",
		"AT+LRPL=32",
		"AT+LRFSV=1638",
		"AT+MODE=0",
		"AT+BAND=0"
};

static uint16_t get_number_of_ready_bytes_rx(void){
	if(lora_port.buffer_rx_tail > lora_port.buffer_rx_head){
		return (uint16_t)(LORA_BUFFER_RX_SIZE - lora_port.buffer_rx_tail + lora_port.buffer_rx_head);
	} else if(lora_port.buffer_rx_tail < lora_port.buffer_rx_head){
		return (uint16_t)(lora_port.buffer_rx_head - lora_port.buffer_rx_tail);
	} else {
		return 0;
	}
}

static void set_uart_baudrate(uint32_t baudrate){
	LORA_UART_PERIPH.Init.BaudRate = baudrate;
	HAL_UART_Init(&LORA_UART_PERIPH);
	HAL_UART_Receive_IT(&LORA_UART_PERIPH, &lora_port.data_rx, 1);
}

static void set_new_line_at_end_buffer_tx(void){
	lora_port.buffer_tx[lora_port.buffer_tx_head] = carriage_return_character;
	lora_port.buffer_tx_head++;
	lora_port.buffer_tx[lora_port.buffer_tx_head] = new_line_character;
	lora_port.buffer_tx_head++;
}

static void remove_last_byte_rx(void){
	lora_port.buffer_rx_tail++;
	if(lora_port.buffer_rx_tail == LORA_BUFFER_RX_SIZE) lora_port.buffer_rx_tail = 0;
}

static void remove_all_bytes_rx(void){
	lora_port.buffer_rx_tail = lora_port.buffer_rx_head;
}

static uint8_t get_config_response_ok(void){
	if(get_number_of_ready_bytes_rx() >= 4){
		if(lora_port.buffer_rx[lora_port.buffer_rx_tail] == ok_config_response[0] &&
				lora_port.buffer_rx[(lora_port.buffer_rx_tail + 1) % LORA_BUFFER_RX_SIZE] == ok_config_response[1] &&
				lora_port.buffer_rx[(lora_port.buffer_rx_tail + 2) % LORA_BUFFER_RX_SIZE] == carriage_return_character &&
				lora_port.buffer_rx[(lora_port.buffer_rx_tail + 3) % LORA_BUFFER_RX_SIZE] == new_line_character){
			remove_all_bytes_rx();
			return COM_OK;
		}else{
			remove_last_byte_rx();
		}
	}
	return COM_ERROR;
}

static void set_config_command_to_buffer_tx(char* table){
	uint8_t iter = 0;
	while(table[iter] != 0){
		lora_port.buffer_tx[iter] = table[iter];
		iter++;
	}
	lora_port.buffer_tx_head = iter;
}

static void calibration_module(void){
	static uint8_t config_module_state = 0;
	static uint8_t tmp_counter = 0;
	static uint8_t waiting_for_response_ok_flag = 0;
	static uint8_t actual_tab_position = 0;
	static uint8_t time_out_msg = 0;
	static uint8_t retransmission_msg = 0;

	switch(config_module_state){
	case 0:
		HAL_GPIO_WritePin(CONFIG_COM_MODULE_GPIO_Port, CONFIG_COM_MODULE_Pin, GPIO_PIN_SET);
		config_module_state++;
		break;
	case 1:
		if(tmp_counter++ == 20){
			tmp_counter = 0;
			if(HAL_GPIO_ReadPin(STATUS_COM_MODULE_GPIO_Port, STATUS_COM_MODULE_Pin) == GPIO_PIN_SET){
				HAL_GPIO_WritePin(CONFIG_COM_MODULE_GPIO_Port, CONFIG_COM_MODULE_Pin, GPIO_PIN_RESET);
				config_module_state++;
			}else{
				set_uart_baudrate(WORKING_BAUDRATE);
				config_module_state = 0;
			}
		}
		break;
	case 2:
		if(HAL_GPIO_ReadPin(STATUS_COM_MODULE_GPIO_Port, STATUS_COM_MODULE_Pin) == GPIO_PIN_RESET){
			config_module_state++;
		}
		break;
	case 3:
		if(waiting_for_response_ok_flag == 0){
			set_config_command_to_buffer_tx((char *)config_lora_command[0]);
			set_new_line_at_end_buffer_tx();
			HAL_UART_Transmit_IT(&LORA_UART_PERIPH, lora_port.buffer_tx, lora_port.buffer_tx_head);
			waiting_for_response_ok_flag = 1;
		}else{
			if(get_config_response_ok() == COM_OK){
				waiting_for_response_ok_flag = 0;
				config_module_state++;
				time_out_msg = 0;
				retransmission_msg = 0;
			}else{
				if(time_out_msg++ == 3){
					time_out_msg = 0;
					if(retransmission_msg++ == 3){
						retransmission_msg = 0;
						config_module_state++;
					}
					waiting_for_response_ok_flag = 0;
				}
			}
		}
		break;
	case 4:
		if(tmp_counter++ == 20){
			tmp_counter = 0;
			set_uart_baudrate(WORKING_BAUDRATE);
			actual_tab_position = 0;
			config_module_state++;
		}
		break;
	case 5:
		if(waiting_for_response_ok_flag == 0){
			set_config_command_to_buffer_tx((char *)config_lora_command[actual_tab_position + 1]);
			set_new_line_at_end_buffer_tx();
			HAL_UART_Transmit_IT(&LORA_UART_PERIPH, lora_port.buffer_tx, lora_port.buffer_tx_head);
			waiting_for_response_ok_flag = 1;
		}else{
			if(get_config_response_ok() == COM_OK){
				waiting_for_response_ok_flag = 0;
				actual_tab_position++;
				if(actual_tab_position >= sizeof(config_lora_command) / 4 - 1){
					config_module_state++;
				}
				time_out_msg = 0;
				retransmission_msg = 0;
			}else{
				if(time_out_msg++ == 3){
					time_out_msg = 0;
					if(retransmission_msg++ == 3){
						retransmission_msg = 0;
						//TODO error state
					}
					waiting_for_response_ok_flag = 0;
				}
			}
		}
		break;
	case 6:
		HAL_GPIO_WritePin(CONFIG_COM_MODULE_GPIO_Port, CONFIG_COM_MODULE_Pin, GPIO_PIN_SET);
		if(HAL_GPIO_ReadPin(STATUS_COM_MODULE_GPIO_Port, STATUS_COM_MODULE_Pin) == GPIO_PIN_SET){
			config_module_state = 0;
			lora_port.configured_flag = 1;
		}
		break;
	}
}

static void active_led(led_communication * led){
	HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
	led->counter = TIME_ACTIVE_LED;
	led->state = 1;
}

static void update_led(led_communication * led){
	if(led->state == 1){
		if(led->counter > 0){
			led->counter--;
		}else{
			HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
			led->state = 0;
		}
	}
}

void init_lora(void){
	led_rx.port = GREEN_LED_GPIO_Port;
	led_rx.pin = GREEN_LED_Pin;

	led_tx.port = RED_LED_GPIO_Port;
	led_tx.pin = RED_LED_Pin;

	HAL_UART_Receive_IT(&LORA_UART_PERIPH, &lora_port.data_rx, 1);
}

void loop_lora(void){
	if(lora_port.configured_flag){
		update_led(&led_tx);
		update_led(&led_rx);

		if(lora_port.buffer_tx_head){
			HAL_UART_Transmit_IT(&LORA_UART_PERIPH, lora_port.buffer_tx, lora_port.buffer_tx_head);
			active_led(&led_tx);

			lora_port.buffer_tx_head = 0;
		}
	}else{
		calibration_module();
	}
}

void set_lora_buffer_to_transmit(uint8_t * buffer_tx, uint16_t * len){
	uint16_t tmp_data_len = get_number_of_ready_bytes_rx();
	if(tmp_data_len && lora_port.configured_flag){
		for(uint16_t i = 0; i < tmp_data_len; i++){
			buffer_tx[i] = lora_port.buffer_rx[(lora_port.buffer_rx_tail + i) % LORA_BUFFER_RX_SIZE];
		}
		lora_port.buffer_rx_tail = (lora_port.buffer_rx_tail + tmp_data_len) % LORA_BUFFER_RX_SIZE;
		*len = tmp_data_len;
	}else{
		*len = 0;
	}
}

void receive_byte_lora(void){
	lora_port.buffer_rx[lora_port.buffer_rx_head] = lora_port.data_rx;
	lora_port.buffer_rx_head ++;
	if(lora_port.buffer_rx_head >= LORA_BUFFER_RX_SIZE){
		lora_port.buffer_rx_head = 0;
	}
	if(lora_port.configured_flag){
		active_led(&led_rx);
	}
	HAL_UART_Receive_IT(&LORA_UART_PERIPH, &lora_port.data_rx, 1);
}

