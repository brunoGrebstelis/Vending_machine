/*
 * aht20_sensor.h
 *
 *  Created on: Feb 6, 2025
 *      Author: bruno
 */

#ifndef INC_RHT_SENSOR_H_
#define INC_RHT_SENSOR_H_

#include <stdint.h>
#include "stm32f7xx_hal.h"
#include "print.h"
#include "spi_rpi.h"
#include "main.h"
#include <stdbool.h>
#include "error_handler.h"

#define PCA9548A_ADDRESS 0x70
#define CHANNEL_0_MASK 0x01
#define CHANNEL_1_MASK 0x02
#define CHANNEL_2_MASK 0x04
#define CHANNEL_3_MASK 0x08
#define CHANNEL_4_MASK 0x10
#define CHANNEL_5_MASK 0x20
#define CHANNEL_6_MASK 0x40
#define CHANNEL_7_MASK 0x80



void CheckTemperature(void);
void AHT20_CalculateValues(uint8_t *data, float *temperature, float *humidity);
HAL_StatusTypeDef AHT20_ReadData(I2C_HandleTypeDef *hi2c, float *temperature, float *humidity);
HAL_StatusTypeDef AHT20_ReadData_PCA9548A(I2C_HandleTypeDef *hi2c, uint8_t pca9548a_address, uint8_t channel, float *temperature, float *humidity) ;

#endif /* INC_RHT_SENSOR_H_ */
