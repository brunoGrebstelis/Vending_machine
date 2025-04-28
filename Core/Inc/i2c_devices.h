/*
 * rgb_led.h
 *
 *  Created on: Feb 6, 2025
 *      Author: bruno
 */

#ifndef INC_I2C_DEVICES_H_
#define INC_I2C_DEVICES_H_

#include "print.h"
#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include <stdbool.h>
#include "error_handler.h"

void I2C_Master_Send_Byte(uint16_t address, uint8_t value);
void Send_RGB(uint16_t address, uint8_t red, uint8_t green, uint8_t blue, uint8_t mode);
void Send_Price(uint16_t address, uint8_t byte1, uint8_t byte2);
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);


#endif /* INC_I2C_DEVICES_H_ */
