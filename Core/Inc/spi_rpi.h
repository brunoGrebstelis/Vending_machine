/*
 * spi_rpi.h
 *
 *  Created on: Feb 7, 2025
 *      Author: bruno
 */

#ifndef INC_SPI_RPI_H_
#define INC_SPI_RPI_H_

#include "print.h"
#include "i2c_devices.h"
#include "locker.h"
#include <stdint.h>
#include <stdio.h>


#define SPI_BUFFER_SIZE 6


extern uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];
extern uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);
void Process_SPI_Command(uint8_t *data, uint16_t size);
void SPI_SendMessage(uint8_t command, uint8_t locker_id, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4);


#endif /* INC_SPI_RPI_H_ */
