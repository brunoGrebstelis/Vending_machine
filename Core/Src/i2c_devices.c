/*
 * rgb_led.c
 *
 *  Created on: Feb 6, 2025
 *      Author: bruno
 */

#include "i2c_devices.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c4;

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
        //setErrorState(STATE_LED_DRIVER);
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
        //setErrorState(STATE_PRICE_TAG);
        //error_locker = address;
        //error_flag = true;
    }
}
