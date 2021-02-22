/*
 * rangetech_hub_lora.h
 *
 *  Created on: 08.02.2021
 *      Author: Bartosz
 */

#ifndef RANGETECH_HUB_LORA_H_
#define RANGETECH_HUB_LORA_H_

#include "main.h"
#include "lora.h"
#include "rs485.h"


void init_hub_lora(void);
void loop_hub_lora_1ms(void);

#endif /* RANGETECH_HUB_LORA_H_ */
