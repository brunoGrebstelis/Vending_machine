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
#define CHANNEL_0 0x01
#define CHANNEL_1 0x02
#define CHANNEL_2 0x04
#define CHANNEL_3 0x08
#define CHANNEL_4 0x10
#define CHANNEL_5 0x20
#define CHANNEL_6 0x40
#define CHANNEL_7 0x80

typedef enum {
    SENSOR_AHT20_1 = 1,
    SENSOR_AHT20_2,
    SENSOR_AHT20_3,
    SENSOR_AHT20_4,
    SENSOR_AHT20_5,
    SENSOR_AHT20_6,
    SENSOR_AHT20_7,
    SENSOR_AHT20_8
} AHT20_Sensor_t;

void CheckTemperature(AHT20_Sensor_t sensor);
void AHT20_CalculateValues(uint8_t *data, float *temperature, float *humidity);
HAL_StatusTypeDef AHT20_ReadData(I2C_HandleTypeDef *hi2c, float *temperature, float *humidity);
HAL_StatusTypeDef AHT20_ReadData_PCA9548A(I2C_HandleTypeDef *hi2c, uint8_t pca9548a_address, uint8_t channel, float *temperature, float *humidity) ;

#endif /* INC_RHT_SENSOR_H_ */
