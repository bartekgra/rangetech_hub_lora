/*
 * rs485.c
 *
 *  Created on: 15.12.2020
 *      Author: Bartosz
 */


#include "rs485.h"

static uint16_t get_number_of_ready_bytes_rx(void){
	if(rs485_port.buffer_rx_tail > rs485_port.buffer_rx_head){
		return (uint16_t)(RS485_BUFFER_RX_SIZE - rs485_port.buffer_rx_tail + rs485_port.buffer_rx_head);
	} else if(rs485_port.buffer_rx_tail < rs485_port.buffer_rx_head){
		return (uint16_t)(rs485_port.buffer_rx_head - rs485_port.buffer_rx_tail);
	} else {
		return 0;
	}
}

void init_rs485(void){
	HAL_UART_Receive_IT(&RS485_UART_PERIPH, &rs485_port.data_rx, 1);
}

void set_rs485_buffer_to_transmit(uint8_t * buffer_tx, uint16_t * len){
	uint16_t tmp_data_len = get_number_of_ready_bytes_rx();
	if(tmp_data_len){
		for(uint16_t i = 0; i < tmp_data_len; i++){
			buffer_tx[i] = rs485_port.buffer_rx[(rs485_port.buffer_rx_tail + i) % RS485_BUFFER_RX_SIZE];
		}
		*len = tmp_data_len;
		rs485_port.buffer_rx_tail = (rs485_port.buffer_rx_tail + tmp_data_len) % RS485_BUFFER_RX_SIZE;
	}else{
		*len = 0;
	}
}

void loop_rs485(void){
	if(rs485_port.buffer_tx_head){
		HAL_GPIO_WritePin(USART1_EN_GPIO_Port, USART1_EN_Pin, GPIO_PIN_SET);
		HAL_UART_Transmit_IT(&RS485_UART_PERIPH, rs485_port.buffer_tx, rs485_port.buffer_tx_head);
		rs485_port.buffer_tx_head = 0;
	}
}

void receive_byte_rs485(void){
	rs485_port.buffer_rx[rs485_port.buffer_rx_head] = rs485_port.data_rx;
	rs485_port.buffer_rx_head ++;
	if(rs485_port.buffer_rx_head >= RS485_BUFFER_RX_SIZE){
		rs485_port.buffer_rx_head = 0;
	}
	HAL_UART_Receive_IT(&RS485_UART_PERIPH, &rs485_port.data_rx, 1);
}
