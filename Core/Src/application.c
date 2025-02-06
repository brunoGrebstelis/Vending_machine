/*
 * application.c
 *
 *  Created on: Jan 30, 2025
 *      Author: bruno
 */

#include "application.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "rht_sensor.h"

/* USER CODE BEGIN PV */
bool error_flag = false;
bool tempSensFailed = false;
bool tempBelowZero = false;
uint8_t error_locker = 0;
SystemErrorState errorState = NO_ERROR;

// Global “flag” indicating at least one locker is opened or waiting for check.
bool lockerFlag = false;

// Track state for each of the 24 lockers
bool lockerOpened[24] = { false };
bool checkPending[24] = { false };
uint32_t openTimestamp[24] = { 0 };
uint32_t lastTime = 0;

// Used for SPI color commands
uint8_t red = 255;
uint8_t green = 255;
uint8_t blue = 255;
uint8_t mode = 0xFF;
/* USER CODE END PV */

/* USER CODE BEGIN Includes */
/* References to hardware resources from main.c */
extern SPI_HandleTypeDef hspi1;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c4;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

/* Also reference SPI buffers if needed */
extern uint8_t spi_rx_buffer[];
extern uint8_t spi_tx_buffer[];
/* USER CODE END Includes */

/* USER CODE BEGIN 0 */

//typedef struct{
//	uint8_t address;
//	float temperature;
//	float humidity;
//	uint8_t raw_data[6];
//}  temperature_sensor_t ;
//
//temperature_sensor_t sensors[8];
//sensors[0].address = 0x01;

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

// SPI receive complete callback
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        Process_SPI_Command(spi_rx_buffer, SPI_BUFFER_SIZE);
        HAL_SPI_Receive_IT(&hspi1, spi_rx_buffer, SPI_BUFFER_SIZE);
    }
}

// Function to handle received SPI data
void Process_SPI_Command(uint8_t *data, uint16_t size) {
    // Debug: Print received data over UART
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
        mode = data[5];  // single space around '='

        if(locker_id == 255) {
            printf("Set all LED to Color: R=%d, G=%d, B=%d\r\n", red, green, blue);
            for(int i = 1; i <= 14; i++) {
                Send_RGB(i + 100, red, green, blue, mode);
            }
        } else {
            Send_RGB(locker_id + 100, red, green, blue, mode);
            printf("Set LED Color: Locker %d, R=%d, G=%d, B=%d\r\n", locker_id, red, green, blue);
        }
    }

    // Example: Handle price command (0x02)
    if (data[0] == 0x02) {
        uint8_t locker_id = data[1];
        uint16_t price = (data[2] << 8) | data[3];
        printf("Set Price: Locker %d, Price=%d euro\r\n", locker_id, price);
        Send_Price(locker_id, data[2], data[3]);
    }

    // Example: Handle unlock command (0x03)
    if (data[0] == 0x03) {
        uint8_t locker_id = data[1];
        printf("Unlock: Locker %d\r\n", locker_id);
        open_cabinet(locker_id);
        Send_RGB(locker_id + 100, 0, 0, 0, 0);
    }

    // Example: Echo command (0xFF)
    if (data[0] == 0xFF) {
        HAL_SPI_Transmit(&hspi1, spi_tx_buffer, SPI_BUFFER_SIZE, 100);
    }
}

// Send one byte via I2C
void I2C_Master_Send_Byte(uint16_t address, uint8_t value) {
    if (HAL_I2C_Master_Transmit(&hi2c1, address << 1, &value, 1, HAL_MAX_DELAY) != HAL_OK) {
        // Could handle error here
    }
}

// Send RGB data over I2C
void Send_RGB(uint16_t address, uint8_t red, uint8_t green, uint8_t blue, uint8_t mode) {
    uint8_t RGB_Buffer[4];
    RGB_Buffer[0] = red;
    RGB_Buffer[1] = green;
    RGB_Buffer[2] = blue;
    RGB_Buffer[3] = mode;

    // Transmit RGB values to the slave
    if (HAL_I2C_Master_Transmit(&hi2c1, (address << 1), RGB_Buffer, 4, HAL_MAX_DELAY) != HAL_OK) {
        //errorState = STATE_LED_DRIVER;
        //error_locker = address - 100;
        //error_flag = true;
    }
}

// Send price values to the slave
void Send_Price(uint16_t address, uint8_t byte1, uint8_t byte2) {
    uint8_t Price_Buffer[2];
    Price_Buffer[0] = byte1;
    Price_Buffer[1] = byte2;

    // Transmit price values to the slave
    if (HAL_I2C_Master_Transmit(&hi2c1, (address << 1), Price_Buffer, 2, HAL_MAX_DELAY) != HAL_OK) {
        //errorState = STATE_PRICE_TAG;
        //error_locker = address;
        //error_flag = true;
    }
}




// Send a message over SPI to the master
void SPI_SendMessage(uint8_t command, uint8_t locker_id, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4) {
    spi_tx_buffer[0] = command;
    spi_tx_buffer[1] = locker_id;
    spi_tx_buffer[2] = data1;
    spi_tx_buffer[3] = data2;
    spi_tx_buffer[4] = data3;
    spi_tx_buffer[5] = data4;

    // Trigger the interrupt pin (PG10) to signal the Raspberry Pi
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_RESET);
}

// Function to calculate XOR checksum
uint8_t calculate_checksum(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
    return byte0 ^ byte1 ^ byte2 ^ byte3;
}

// Function to open a cabinet
void open_cabinet(uint8_t locker_id) {
    if (locker_id < 1 || locker_id > 24) {
        printf("Invalid locker ID. Must be between 1 and 24.\n");
        return;
    }

    // Check the cabinet status first
    int status = read_cabinet_status(locker_id);
    if (status == 1) {
        printf("Locker %d is already open. No action required.\n", locker_id);
        return;
    } else if (status == 0) {
        uint8_t command[5];
        uint8_t response[5];

        // Build the command to open the cabinet
        command[0] = 0x8A;
        command[1] = 0x01;
        command[2] = locker_id;
        command[3] = 0x11;
        command[4] = calculate_checksum(command[0], command[1], command[2], command[3]);

        // Transmit the command
        RS485_Transmit(command, sizeof(command));

        // Wait for the response
        if (HAL_UART_Receive(&huart2, response, sizeof(response), 1000) == HAL_OK) {
            printf("Response received: ");
            for (int i = 0; i < sizeof(response); i++) {
                printf("0x%02X ", response[i]);
            }
            printf("\n");

            uint8_t expected_checksum = calculate_checksum(response[0], response[1], response[2], response[3]);
            if (response[4] != expected_checksum) {
                printf("Response checksum error.\n");
                return;
            }

            if (response[3] == 0x11) {
                printf("Locker %d opened successfully.\n", locker_id);
                lockerOpened[locker_id - 1] = true;
                openTimestamp[locker_id - 1] = HAL_GetTick();
                checkPending[locker_id - 1] = true;
                lockerFlag = true;
            } else {
                printf("Unexpected response when opening locker %d.\n", locker_id);
                errorState = STATE_JAMMED;
                error_locker = locker_id;
                error_flag = true;
            }
        } else {
            printf("No response received when opening the cabinet.\n");
        }
    } else {
        printf("Failed to determine the status of locker %d. Aborting open operation.\n", locker_id);
    }
}

// Function to read the cabinet status
int read_cabinet_status(uint8_t locker_id) {
    if (locker_id < 1 || locker_id > 24) {
        printf("Invalid locker ID. Must be between 1 and 24.\n");
        return -1;
    }

    uint8_t command[5];
    uint8_t response[5];

    // Build the command to read the cabinet status
    command[0] = 0x80;
    command[1] = 0x01;
    command[2] = locker_id;
    command[3] = 0x33;
    command[4] = calculate_checksum(command[0], command[1], command[2], command[3]);

    // Transmit the command
    RS485_Transmit(command, sizeof(command));

    // Wait for the response
    if (HAL_UART_Receive(&huart2, response, sizeof(response), 1000) == HAL_OK) {
        // Print the raw response
        printf("Response received: ");
        for (int i = 0; i < sizeof(response); i++) {
            printf("0x%02X ", response[i]);
        }
        printf("\n");

        // Validate the response
        uint8_t expected_checksum = calculate_checksum(response[0], response[1], response[2], response[3]);
        if (response[4] != expected_checksum) {
            printf("Response checksum error.\n");
            return -1;
        }

        // Interpret the response
        if (response[3] == 0x11) {
            printf("Locker %d is open.\n", locker_id);
            return 1; // Locker is open
        } else if (response[3] == 0x00) {
            printf("Locker %d is closed.\n", locker_id);
            return 0; // Locker is closed
        } else {
            printf("Unexpected response for locker %d.\n", locker_id);
            return -1; // Error or unexpected
        }
    } else {
        printf("No response received when reading the cabinet status.\n");
        return -1;
    }
}

// Set RS485 to transmit mode
void RS485_SetTransmitMode(void) {
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_11, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_SET);
}

// Set RS485 to receive mode
void RS485_SetReceiveMode(void) {
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_11, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);
}

// Transmit data via RS485
void RS485_Transmit(uint8_t *data, uint16_t size) {
    RS485_SetTransmitMode();
    HAL_UART_Transmit(&huart2, data, size, HAL_MAX_DELAY);
    RS485_SetReceiveMode();
}

// Receive data via RS485
void RS485_Receive(uint8_t *data, uint16_t size, uint32_t timeout) {
    RS485_SetReceiveMode();
    HAL_UART_Receive(&huart2, data, size, timeout);
}

// Handle error states
void HandleState(SystemErrorState state) {
    switch (state) {
        case STATE_JAMMED:
            printf("Case 1: JAMMED\n");
            SPI_SendMessage(0xF1, error_locker, 150, 0xFF, 0xFF, 0xFF);
            errorState = NO_ERROR;
            error_locker = 0;
            error_flag = false;
            break;
        case STATE_OPENED:
            printf("Case 2: OPENED\n");
            SPI_SendMessage(0xF1, error_locker, 50, 0xFF, 0xFF, 0xFF);
            errorState = NO_ERROR;
            error_locker = 0;
            error_flag = false;
            break;
        case STATE_CLOSED:
            printf("Case 3: CLOSED\n");
            break;
        case STATE_PRICE_TAG:
            printf("Case 4: PRICE_TAG\n");
            SPI_SendMessage(0xF2, error_locker, 50, 0xFF, 0xFF, 0xFF);
            errorState = NO_ERROR;
            error_locker = 0;
            error_flag = false;
            break;
        case STATE_LED_DRIVER:
            printf("Case 5: LED_DRIVER\n");
            SPI_SendMessage(0xF2, error_locker, 100, 0xFF, 0xFF, 0xFF);
            errorState = NO_ERROR;
            error_locker = 0;
            error_flag = false;
            break;
        case STATE_TEMPERATURE:
            printf("Case 6: TEMPERATURE\n");
            SPI_SendMessage(0xF3, error_locker, 50, 0xFF, 0xFF, 0xFF);
            errorState = NO_ERROR;
            error_locker = 0;
            error_flag = false;
            break;
        case STATE_SENSOR:
            printf("Case 7: SENSOR\n");
            SPI_SendMessage(0xF3, error_locker, 100, 0xFF, 0xFF, 0xFF);
            errorState = NO_ERROR;
            error_locker = 0;
            error_flag = false;
            break;
        default:
            printf("Invalid state\n");
            break;
    }
}

// Check all lockers after delay (5 minutes) to see if they remain open
void CheckAllLockersAfterDelay(void) {
    bool anyOpenedOrPending = false;
    uint32_t currentTime = HAL_GetTick();

    for (int i = 0; i < 24; i++) {
        // If we had marked a locker for a re-check
        if (checkPending[i]) {
            anyOpenedOrPending = true;
            // Has 5 minutes passed since we opened it?
            if ((currentTime - openTimestamp[i]) >= LOCKER_CHECK_DELAY) {
                int status = read_cabinet_status(i + 1);
                if (status == 1) {
                    errorState = STATE_OPENED;
                    error_locker = i + 1;
                    error_flag = true;
                    printf("Locker %d is still open after 5 minutes!\n", i + 1);
                } else if (status == 0) {
                    // It's now closed
                    lockerOpened[i] = false;
                }
                checkPending[i] = false;
            }
        } else {
            if (lockerOpened[i]) {
                anyOpenedOrPending = true;
            }
        }
    }
    lockerFlag = anyOpenedOrPending;
}

// Periodic temperature check
void CheckTemperature(void) {

//	for (int i =0; i<8; i++){
//		AHT20_ReadData_PCA9548A(&hi2c2, PCA9548A_ADDRESS, sensors[i]);
//	}
    float temperature, humidity;
    if (AHT20_ReadData_PCA9548A(&hi2c2, PCA9548A_ADDRESS, CHANNEL_2_MASK, &temperature, &humidity) == HAL_OK) {
        printf("Temperature: %.2f C\r\n", temperature);
        printf("Humidity: %.2f %%RH\r\n", humidity);

        int16_t tRaw = (int16_t)(temperature * 100.0f);
        int16_t hRaw = (int16_t)(humidity * 100.0f);
        uint8_t tHigh = (uint8_t)((tRaw >> 8) & 0xFF);
        uint8_t tLow  = (uint8_t)( tRaw       & 0xFF);
        uint8_t hHigh = (uint8_t)((hRaw >> 8) & 0xFF);
        uint8_t hLow  = (uint8_t)( hRaw       & 0xFF);
        SPI_SendMessage(0xF4, 3, tHigh, tLow, hHigh, hLow);

    } else {
        printf("Failed to read data from AHT20 sensor.\r\n");
        if (!tempSensFailed) {
            errorState = STATE_SENSOR;
            error_locker = 3;
            error_flag = true;
            tempSensFailed = true;
        }
    }

    if (temperature < 0.0 && !tempBelowZero) {
        tempBelowZero = true;
        errorState = STATE_TEMPERATURE;
        error_locker = 3;
        error_flag = true;
    } else if (temperature >= 0.0) {
        tempBelowZero = false;
    }
}


#ifdef __GNUC__
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif
/* USER CODE END 1 */
