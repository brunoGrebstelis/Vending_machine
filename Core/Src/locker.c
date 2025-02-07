/*
 * lock.c
 *
 *  Created on: Feb 6, 2025
 *      Author: bruno
 */

#include "locker.h"

extern UART_HandleTypeDef huart2;

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
                setErrorState(STATE_JAMMED);
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
