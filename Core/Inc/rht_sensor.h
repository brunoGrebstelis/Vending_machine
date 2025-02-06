/*
 * rht_sensor.h
 *
 *  Created on: Feb 6, 2025
 *      Author: bruno
 */

#ifndef INC_RHT_SENSOR_H_
#define INC_RHT_SENSOR_H_

#include <stdint.h>
#include "stm32f7xx_hal.h"


void AHT20_CalculateValues(uint8_t *data, float *temperature, float *humidity);
HAL_StatusTypeDef AHT20_ReadData(I2C_HandleTypeDef *hi2c, float *temperature, float *humidity);
HAL_StatusTypeDef AHT20_ReadData_PCA9548A(I2C_HandleTypeDef *hi2c, uint8_t pca9548a_address, uint8_t channel, float *temperature, float *humidity) ;

#endif /* INC_RHT_SENSOR_H_ */
