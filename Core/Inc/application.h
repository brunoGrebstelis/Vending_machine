/*
 * application.h
 *
 *  Created on: Jan 30, 2025
 *      Author: bruno
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

/* Original #defines from the code ------------------------------------------*/
#define SPI_BUFFER_SIZE 6
#define SLAVE_ADDRESS 0x0B
#define DELAY_MS 370

#define PCA9548A_ADDRESS 0x70
#define CHANNEL_0_MASK 0x01
#define CHANNEL_1_MASK 0x02
#define CHANNEL_2_MASK 0x04
#define CHANNEL_3_MASK 0x08
#define CHANNEL_4_MASK 0x10
#define CHANNEL_5_MASK 0x20
#define CHANNEL_6_MASK 0x40
#define CHANNEL_7_MASK 0x80

#define LOCKER_CHECK_DELAY 60000  // e.g. 5 minutes in ms

/* Enum from original code */
typedef enum {
    STATE_JAMMED = 1,     // Case 1
    STATE_OPENED,         // Case 2
    STATE_CLOSED,         // Case 3
    STATE_PRICE_TAG,      // Case 4
    STATE_LED_DRIVER,     // Case 5
    STATE_TEMPERATURE,    // Case 6
    STATE_SENSOR,         // Case 7
    NO_ERROR
} SystemErrorState;

/* Global variables that need to be visible project-wide ---------------------*/
extern bool error_flag;
extern bool tempSensFailed;
extern bool tempBelowZero;
extern uint8_t error_locker;
extern SystemErrorState errorState;
extern bool lockerFlag;
extern bool lockerOpened[24];
extern bool checkPending[24];
extern uint32_t openTimestamp[24];
extern uint32_t lastTime;

/* Also the color/mode variables used in Process_SPI_Command */
extern uint8_t red;
extern uint8_t green;
extern uint8_t blue;
extern uint8_t mode;

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
void HandleState(SystemErrorState state);
void CheckAllLockersAfterDelay(void);
void CheckTemperature(void);
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi);

#endif /* INC_APPLICATION_H_ */
