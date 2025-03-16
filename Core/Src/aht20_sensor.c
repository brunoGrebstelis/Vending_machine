/*
 * rht_sensor.c
 *
 *  Created on: Feb 6, 2025
 *      Author: bruno
 */
#include "aht20_sensor.h"

#define AHT20_ADDRESS (0x38 << 1) // Shifted left for STM32 HAL

extern I2C_HandleTypeDef hi2c2;

static bool tempSensFailed[5] = {false, false, false, false, false};
static bool tempBelowZero[5] = {false, false, false, false, false};


// Periodic temperature check
void CheckTemperature(AHT20_Sensor_t sensor) {
    if (sensor < SENSOR_AHT20_1 || sensor > SENSOR_AHT20_8) {
        printf("Invalid sensor: %d. Must be between SENSOR_AHT20_1 and SENSOR_AHT20_8.\r\n", sensor);
        return;
    }

    // Mapping sensor to PCA9548A channel bitmask
    uint8_t channelMasks[] = {CHANNEL_0, CHANNEL_1, CHANNEL_2, CHANNEL_3, CHANNEL_4, CHANNEL_5, CHANNEL_6, CHANNEL_7};
    uint8_t selectedChannel = channelMasks[sensor - 1];  // Adjust for 0-based indexing

    float temperature, humidity;
    if (AHT20_ReadData_PCA9548A(&hi2c2, PCA9548A_ADDRESS, selectedChannel, &temperature, &humidity) == HAL_OK) {
        printf("Sensor %d - Temperature: %.2f C\r\n", sensor, temperature);
        printf("Sensor %d - Humidity: %.2f %%RH\r\n", sensor, humidity);

        int16_t tRaw = (int16_t)(temperature * 100.0f);
        int16_t hRaw = (int16_t)(humidity * 100.0f);
        uint8_t tHigh = (uint8_t)((tRaw >> 8) & 0xFF);
        uint8_t tLow  = (uint8_t)( tRaw       & 0xFF);
        uint8_t hHigh = (uint8_t)((hRaw >> 8) & 0xFF);
        uint8_t hLow  = (uint8_t)( hRaw       & 0xFF);
        SPI_SendMessage(0xF4, sensor, tHigh, tLow, hHigh, hLow);
        tempSensFailed[sensor - 1] = false;

        switch (sensor) {
            case SENSOR_AHT20_1: // Chamber 1
                if (temperature < TEMP_MIN) {
                    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, GPIO_PIN_SET);   // Heater ON
                } else if (temperature > TEMP_MAX) {
                    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, GPIO_PIN_RESET); // Heater OFF
                }
                // (If within [TEMP_MIN, TEMP_MAX], do nothing => keep previous state
                //  or explicitly turn OFF if you want that behavior)
                break;

            case SENSOR_AHT20_2: // Chamber 2
                if (temperature < TEMP_MIN) {
                    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET);   // Heater ON
                } else if (temperature > TEMP_MAX) {
                    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET); // Heater OFF
                }
                break;

            case SENSOR_AHT20_3: // Chamber 3
                if (temperature < TEMP_MIN) {
                    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_SET);   // Heater ON
                } else if (temperature > TEMP_MAX) {
                    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_RESET); // Heater OFF
                }
                break;
            default:
                break;
        }

    } else {
        printf("Failed to read data from sensor %d.\r\n", sensor);
        if (!tempSensFailed[sensor - 1]) {
            setErrorState(STATE_SENSOR);
            error_locker = sensor;
            error_flag = true;
            tempSensFailed[sensor - 1] = true;
        }
    }

    if (temperature < 0.0 && !tempBelowZero[sensor - 1]) {
        tempBelowZero[sensor - 1] = true;
        setErrorState(STATE_TEMPERATURE);
        error_locker = sensor;
        error_flag = true;
    } else if (temperature >= 0.0) {
        tempBelowZero[sensor - 1] = false;
    }


}



// Read data from AHT20
HAL_StatusTypeDef AHT20_ReadData(I2C_HandleTypeDef *hi2c, float *temperature, float *humidity) {
    uint8_t cmd_measure[] = {0xAC, 0x33, 0x00};
    uint8_t raw_data[6];
    uint8_t status;

    // Check status
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c, AHT20_ADDRESS, (uint8_t[]){0x71}, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    ret = HAL_I2C_Master_Receive(hi2c, AHT20_ADDRESS, &status, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // Check if the sensor is calibrated (status should be 0x18)
    if ((status & 0x18) != 0x18) {
        // Initialize the sensor if not calibrated
        uint8_t init_cmd[] = {0xBE, 0x08, 0x00};
        ret = HAL_I2C_Master_Transmit(hi2c, AHT20_ADDRESS, init_cmd, 3, HAL_MAX_DELAY);
        if (ret != HAL_OK) return ret;
        HAL_Delay(10);
    }

    // Trigger measurement
    ret = HAL_I2C_Master_Transmit(hi2c, AHT20_ADDRESS, cmd_measure, sizeof(cmd_measure), HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // Wait for measurement to complete
    HAL_Delay(80);

    // Read measurement data
    ret = HAL_I2C_Master_Receive(hi2c, AHT20_ADDRESS, raw_data, sizeof(raw_data), HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // Calculate temperature and humidity
    AHT20_CalculateValues(raw_data, temperature, humidity);
    return HAL_OK;
}

// Function to calculate temperature and humidity from raw data
void AHT20_CalculateValues(uint8_t *data, float *temperature, float *humidity) {
    uint32_t raw_humidity = (data[1] << 12) | (data[2] << 4) | (data[3] >> 4);
    uint32_t raw_temperature = ((data[3] & 0xF) << 16) | (data[4] << 8) | data[5];
    *humidity = (raw_humidity * 100.0) / 1048576;
    *temperature = ((raw_temperature * 200.0) / 1048576) - 50.0;
}


// Read data from AHT20 via PCA9548A
HAL_StatusTypeDef AHT20_ReadData_PCA9548A(I2C_HandleTypeDef *hi2c, uint8_t pca9548a_address, uint8_t channel, float *temperature, float *humidity) {
    uint8_t cmd_measure[] = {0xAC, 0x33, 0x00};
    uint8_t raw_data[6];
    uint8_t status;

    // Select the appropriate channel on PCA9548A
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c, pca9548a_address << 1, &channel, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // Check status of AHT20
    ret = HAL_I2C_Master_Transmit(hi2c, AHT20_ADDRESS, (uint8_t[]){0x71}, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    ret = HAL_I2C_Master_Receive(hi2c, AHT20_ADDRESS, &status, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // Check if the sensor is calibrated
    if ((status & 0x18) != 0x18) {
        uint8_t init_cmd[] = {0xBE, 0x08, 0x00};
        ret = HAL_I2C_Master_Transmit(hi2c, AHT20_ADDRESS, init_cmd, 3, HAL_MAX_DELAY);
        if (ret != HAL_OK) return ret;
        HAL_Delay(10);
    }

    // Trigger measurement
    ret = HAL_I2C_Master_Transmit(hi2c, AHT20_ADDRESS, cmd_measure, sizeof(cmd_measure), HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // Wait for measurement
    HAL_Delay(80);

    // Read measurement data
    ret = HAL_I2C_Master_Receive(hi2c, AHT20_ADDRESS, raw_data, sizeof(raw_data), HAL_MAX_DELAY);
    if (ret != HAL_OK) return ret;

    // Calculate temperature and humidity
    AHT20_CalculateValues(raw_data, temperature, humidity);
    return HAL_OK;
}
