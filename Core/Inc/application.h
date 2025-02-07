/*
 * application.h
 *
 *  Created on: Jan 30, 2025
 *      Author: bruno
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#include "main.h"
#include "aht20_sensor.h"
#include "error_handler.h"
#include "print.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Original #defines from the code ------------------------------------------*/

#define SLAVE_ADDRESS 0x0B
#define DELAY_MS 370



/* Function prototypes (from original code) ---------------------------------*/
void Process_SPI_Command(uint8_t *data, uint16_t size);
void I2C_Master_Send_Byte(uint16_t address, uint8_t value);
void Send_RGB(uint16_t address, uint8_t red, uint8_t green, uint8_t blue, uint8_t mode);
void Send_Price(uint16_t address, uint8_t byte1, uint8_t byte2);
HAL_StatusTypeDef AHT20_ReadData(I2C_HandleTypeDef *hi2c, float *temperature, float *humidity);
HAL_StatusTypeDef AHT20_ReadData_PCA9548A(I2C_HandleTypeDef *hi2c, uint8_t pca9548a_address, uint8_t channel, float *temperature, float *humidity);
void SPI_SendMessage(uint8_t command, uint8_t locker_id, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4);
uint8_t calculate_checksum(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);
void open_cabinet(uint8_t locker_id);
int read_cabinet_status(uint8_t locker_id);
void RS485_SetTransmitMode(void);
void RS485_SetReceiveMode(void);
void RS485_Transmit(uint8_t *data, uint16_t size);
void RS485_Receive(uint8_t *data, uint16_t size, uint32_t timeout);
void HandleState();
void CheckAllLockersAfterDelay(void);
void CheckTemperature(void);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);

#endif /* INC_APPLICATION_H_ */
