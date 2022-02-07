/*
 * rs485.h
 *
 *  Created on: 15.12.2020
 *      Author: Bartosz
 */

#ifndef INC_RS485_H_
#define INC_RS485_H_

#include "main.h"

#define RS485_UART_PERIPH			huart1
extern UART_HandleTypeDef 			RS485_UART_PERIPH;


#define RS485_BUFFER_RX_SIZE		1000
#define RS485_BUFFER_TX_SIZE		1000

struct{
	uint8_t data_rx;
	uint8_t buffer_rx[RS485_BUFFER_RX_SIZE];
	uint16_t buffer_rx_head;
	uint16_t buffer_rx_tail;

	uint8_t buffer_tx[RS485_BUFFER_TX_SIZE];
	uint16_t buffer_tx_head;

	uint8_t receiveItFlag;
}rs485_port;


void init_rs485(void);
void set_rs485_buffer_to_transmit(uint8_t * buffer_tx, uint16_t * len);
void loop_rs485(void);
void receive_byte_rs485(void);


#endif /* INC_RS485_H_ */
