/*
 * commands.c
 *
 *  Created on: Nov 10, 2024
 *      Author: drozd
 */

#include "commands.h"

void TOGGLELED() {
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}
