/*
 * application.c
 *
 *  Created on: Jan 30, 2025
 *      Author: bruno
 */

#include "application.h"

static uint64_t lastTime1 = 0;
static uint64_t lastTime2 = 0;
static int sensorIndex = 0;

void init_application(){
	printf("STM32 SPI Slave Ready\r\n"); // Print ready message
	//HAL_SPI_Receive_IT(&hspi1, spi_rx_buffer, SPI_BUFFER_SIZE);
	HAL_SPI_TransmitReceive_IT(&hspi1, spi_tx_buffer, spi_rx_buffer, SPI_BUFFER_SIZE);
	setFanMode();

}

void loop_application(){

	if(getSPIFlag()){
		Process_SPI_Command(rpi_msg, SPI_BUFFER_SIZE);
	} else {
		if(getSendSPIFlag() && get_error_flag()) HandleState();
		if(getSendSPIFlag() && get_locker_flag()) CheckAllLockersAfterDelay();
		if(getClimateFlag()) setFanMode();
		if(HAL_GetTick() - lastTime1 >= 5000 && getAutoFlag()) {
			lastTime1 = HAL_GetTick();
			//ControlClimate();
		}
		if (getSendSPIFlag() && HAL_GetTick() - lastTime2 >= 60000) {
		    lastTime2 = HAL_GetTick();
		    switch (sensorIndex) {
		        case 0:
		            CheckTemperature(SENSOR_AHT20_1);
		            CheckTemperature(SENSOR_AHT20_2);
		            break;
		        case 1:
		        	CheckTemperature(SENSOR_AHT20_1);
		            CheckTemperature(SENSOR_AHT20_3);
		            break;
		        case 2:
		        	CheckTemperature(SENSOR_AHT20_1);
		            CheckTemperature(SENSOR_AHT20_4);
		            break;
		    }
		    sensorIndex = (sensorIndex + 1) % 3;
		}
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









