/*
 * spi_rpi.c
 *
 *  Created on: Feb 7, 2025
 *      Author: bruno
 */
#include "spi_rpi.h"


uint8_t spi_rx_buffer[SPI_BUFFER_SIZE];
uint8_t spi_tx_buffer[SPI_BUFFER_SIZE];
uint8_t rpi_msg[SPI_BUFFER_SIZE];

static bool spi_flag = false;
static bool send_spi_flag = true;

extern SPI_HandleTypeDef hspi1;


bool getSPIFlag(){
	return spi_flag;
}
bool getSendSPIFlag(){
	return send_spi_flag;
}


// SPI receive complete callback
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {

    	send_spi_flag = false;
    	if(spi_rx_buffer[0] != 0xFF){
    		spi_flag = true;
    		memcpy(rpi_msg, spi_rx_buffer, SPI_BUFFER_SIZE);
    	}
        HAL_SPI_TransmitReceive_IT(&hspi1, spi_tx_buffer, spi_rx_buffer, SPI_BUFFER_SIZE);


    }
}




// Function to handle received SPI data
void Process_SPI_Command(uint8_t *data, uint16_t size) {
	uint8_t red = 255;
	uint8_t green = 255;
	uint8_t blue = 255;
	uint8_t mode = 0xFF;

    printf("SPI Data Received: ");
    for (uint16_t i = 0; i < size; i++) {
        printf("0x%02X ", data[i]);
    }
    printf("\r\n");

    // Example: Handle LED color command (0x01)
    if (data[0] == 0x01) {
        uint8_t locker_id = data[1];
        red = data[2];
        green = data[3];
        blue = data[4];
        mode = data[5];

        if(locker_id == 255) {
        	//HAL_Delay(1);
            printf("Set all LED to Color: R=%d, G=%d, B=%d\r\n", red, green, blue);
            for(int i = 1; i <= 14; i++) {
            	HAL_Delay(1);
                Send_RGB(i + 100, red, green, blue, mode);
                //HAL_Delay(10);

            }
        } else {
            Send_RGB(locker_id + 100, red, green, blue, mode);
            printf("Set LED Color: Locker %d, R=%d, G=%d, B=%d\r\n", locker_id, red, green, blue);
        }
    }

    // Example: Handle price command (0x02)
    if (data[0] == 0x02) {
        uint8_t  locker_id = data[1];
        uint16_t price     = (data[2] << 8) | data[3];
        printf("Set Price: Locker %d, Price = %d EUR\r\n", locker_id, price);
        /* Zero is a special case: send 10 first, then 0                       *
         * 10 = 0x000A  → high-byte 0, low-byte 10 (decimal)                   */
        if (price == 0) {
            Send_Price(locker_id, 0x00, 0x0A);       // send 10
            HAL_Delay(10);
            Send_Price(locker_id, 0x00, 0x00);       // now send 0
        } else { Send_Price(locker_id, data[2], data[3]); }
    }

    // Example: Handle unlock command (0x03)
    if (data[0] == 0x03) {
        uint8_t locker_id = data[1];
        printf("Unlock: Locker %d\r\n", locker_id);
        Send_RGB(locker_id + 100, 0, 0, 0, 0);
        open_cabinet(locker_id);

    }

    if (data[0] == 0x04) {
    	fanMode = data[1];
        printf("Mode: %d\r\n", fanMode);
        setClimateFlag(true);


    }

    if (data[0] == 0x00) {
        for (int i = 0; i < size; i++) {
            spi_tx_buffer[i] = 0x00;
        }
    }
    send_spi_flag = true;
    spi_flag = false;


}

// Send a message over SPI to the master
void SPI_SendMessage(uint8_t command, uint8_t locker_id, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4) {
	//printf("1\r\n");
	send_spi_flag = false;
    spi_tx_buffer[0] = command;
    spi_tx_buffer[1] = locker_id;
    spi_tx_buffer[2] = data1;
    spi_tx_buffer[3] = data2;
    spi_tx_buffer[4] = data3;
    spi_tx_buffer[5] = data4;

    HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_10);

}
