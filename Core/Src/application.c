/*
 * application.c
 *
 *  Created on: Jan 30, 2025
 *      Author: bruno
 */

#include "application.h"

uint64_t lastTime = 0;

void init_application(){
	printf("STM32 SPI Slave Ready\r\n"); // Print ready message
	HAL_SPI_Receive_IT(&hspi1, spi_rx_buffer, SPI_BUFFER_SIZE);
}

void loop_application(){

	if(getSPIFlag()){
		Process_SPI_Command(spi_rx_buffer, SPI_BUFFER_SIZE);
	}else if(get_error_flag()){
		HandleState();
	}else if(get_locker_flag()){
		CheckAllLockersAfterDelay();
	}else if(!getSPIFlag() && HAL_GetTick() - lastTime >= 5000) {
      lastTime = HAL_GetTick();
      CheckTemperature();
    }

}




//typedef struct{
//	uint8_t address;
//	float temperature;
//	float humidity;
//	uint8_t raw_data[6];
//}  temperature_sensor_t ;
//
//temperature_sensor_t sensors[8];
//sensors[0].address = 0x01;









