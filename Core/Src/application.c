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
	HAL_SPI_TransmitReceive_IT(&hspi1, spi_tx_buffer, spi_rx_buffer, SPI_BUFFER_SIZE);
	HAL_Delay(10);
	SPI_SendMessage(0xF5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
	//setFanMode();

	//uint8_t red = 255;
	//uint8_t green = 255;
	//uint8_t blue = 255;
	//uint8_t mode = 0xFF;

	//CheckTemperature(SENSOR_AHT20_2);



    //for(int i = 1; i <= 14; i++) {
    	//Send_Price(1, 0x00, i);
    	//CheckTemperature(i);
        //Send_RGB(i + 100, red, green, blue, mode);
        //open_cabinet(i);
    //}


}

void loop_application(){


	if(getSPIFlag()){
		Process_SPI_Command(rpi_msg, SPI_BUFFER_SIZE);
	} else {
		if(getSendSPIFlag() && get_error_flag()) HandleState();
		if(getSendSPIFlag() && get_locker_flag()) CheckAllLockersAfterDelay();
		if(getClimateFlag()) setFanMode();

/*
		if (getSendSPIFlag() && HAL_GetTick() - lastTime2 >= 300000) {
		    lastTime2 = HAL_GetTick();
		    CheckTemperature(SENSOR_AHT20_1);
		}
*/

		if(HAL_GetTick() - lastTime1 >= 33000 && getAutoFlag()) {
			lastTime1 = HAL_GetTick();
			ControlClimate();
		}
		if (getSendSPIFlag() && HAL_GetTick() - lastTime2 >= 30000) {
		    lastTime2 = HAL_GetTick();
		    switch (sensorIndex) {
		        case 0:
		            CheckTemperature(SENSOR_AHT20_1);
		            break;
		        case 1:
		        	CheckTemperature(SENSOR_AHT20_2);
		            break;
		        case 2:
		        	CheckTemperature(SENSOR_AHT20_1);
		            break;
		        case 3:
		            CheckTemperature(SENSOR_AHT20_3);
		            break;
		        case 4:
		        	CheckTemperature(SENSOR_AHT20_1);
		            break;
		        case 5:
		            CheckTemperature(SENSOR_AHT20_4);
		            break;
		        case 6:
		        	CheckTemperature(SENSOR_AHT20_1);
		            break;
		        case 7:
		            CheckTemperature(SENSOR_AHT20_5);
		            break;
		    }
		    sensorIndex = (sensorIndex + 1) % 8;
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









