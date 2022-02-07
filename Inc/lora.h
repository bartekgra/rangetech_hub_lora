/*
 * lora.h
 *
 *  Created on: 08.02.2021
 *      Author: Bartosz
 */

#ifndef LORA_H_
#define LORA_H_

#include "main.h"

#define START_BAUDRATE 				9600
#define WORKING_BAUDRATE			115200

#define COM_OK						1
#define COM_ERROR 					2

#define LORA_UART_PERIPH			huart2
extern UART_HandleTypeDef 			LORA_UART_PERIPH;

#define LORA_BUFFER_RX_SIZE				1000
#define LORA_BUFFER_TX_SIZE				1000


struct{
	uint8_t data_rx;
	uint8_t buffer_rx[LORA_BUFFER_RX_SIZE];
	uint16_t buffer_rx_head;
	uint16_t buffer_rx_tail;

	uint8_t buffer_tx[LORA_BUFFER_TX_SIZE];
	uint16_t buffer_tx_head;

	uint8_t configured_flag;

	uint8_t receiveItFlag;
}lora_port;


#define TIME_ACTIVE_LED			50

typedef struct{
	uint8_t state;
	uint8_t counter;

	uint16_t pin;
	GPIO_TypeDef * port;
}led_communication;

led_communication led_rx, led_tx;

void init_lora(void);
void loop_lora(void);
void set_lora_buffer_to_transmit(uint8_t * buffer_tx, uint16_t * len);
void receive_byte_lora(void);


#endif /* LORA_H_ */
