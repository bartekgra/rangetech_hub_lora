/*
 * rangetech_hub_lora.c
 *
 *  Created on: 08.02.2021
 *      Author: Bartosz
 */

#include "rangetech_hub_lora.h"

static void state_led_update(void){
	if(lora_port.configured_flag){
		HAL_GPIO_WritePin(YELLOW_LED_GPIO_Port, YELLOW_LED_Pin, GPIO_PIN_SET);
	}
}

void init_hub_lora(void){
	init_lora();
	init_rs485();
}

void loop_hub_lora_1ms(void){
	if(lora_port.configured_flag){
		set_rs485_buffer_to_transmit(lora_port.buffer_tx, &lora_port.buffer_tx_head);
	}
	set_lora_buffer_to_transmit(rs485_port.buffer_tx, &rs485_port.buffer_tx_head);

	loop_rs485();
	loop_lora();

	state_led_update();
}
