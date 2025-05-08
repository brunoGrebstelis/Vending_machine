/*
 * error_handler.c
 *
 *  Created on: Feb 7, 2025
 *      Author: bruno
 */
#include "error_handler.h"



SystemErrorState errorState = NO_ERROR;
uint8_t error_locker = 0;
bool error_flag = false;

bool lockerFlag = false;
bool lockerOpened[24] = { false };
bool checkPending[24] = { false };
uint64_t openTimestamp[24] = { 0 };

static uint8_t command = 0;


bool get_error_flag(){
	return error_flag;
}
bool get_locker_flag(){
	return lockerFlag;
}

static void reSetError(){
    errorState = NO_ERROR;
    error_locker = 0;
    error_flag = false;
}

void sendLogUART(uint8_t err_locker, uint8_t cmd){
	command = cmd;
	setError(STATE_UART, err_locker);
}


void setError(SystemErrorState state, uint8_t err_locker){
	errorState = state;
    error_locker = err_locker;
    error_flag = true;
}

// Handle error states
void HandleState() {
	SystemErrorState state = errorState;
    switch (state) {
        case STATE_JAMMED:
            printf("Case 1: JAMMED\n");
            SPI_SendMessage(0xF1, error_locker, 150, 0xFF, 0xFF, 0xFF);
            reSetError();
            break;
        case STATE_OPENED:
            printf("Case 2: OPENED\n");
            SPI_SendMessage(0xF1, error_locker, 50, 0xFF, 0xFF, 0xFF);
            reSetError();
            break;
        case STATE_CLOSED:
            printf("Case 3: CLOSED\n");
            reSetError();
            break;
        case STATE_PRICE_TAG:
            printf("Case 4: PRICE_TAG\n");
            SPI_SendMessage(0xF2, error_locker, 50, 0xFF, 0xFF, 0xFF);
            reSetError();
            break;
        case STATE_LED_DRIVER:
            printf("Case 5: LED_DRIVER\n");
            SPI_SendMessage(0xF2, error_locker, 100, 0xFF, 0xFF, 0xFF);
            reSetError();
            break;
        case STATE_TEMPERATURE:
            printf("Case 6: TEMPERATURE\n");
            SPI_SendMessage(0xF3, error_locker, 50, 0xFF, 0xFF, 0xFF);
            reSetError();
            break;
        case STATE_SENSOR:
            printf("Case 7: SENSOR\n");
            SPI_SendMessage(0xF3, error_locker, 100, 0xFF, 0xFF, 0xFF);
            reSetError();
            break;
        case STATE_UART:
            printf("Case 7: UART\n");
            SPI_SendMessage(0xF6, error_locker, command, 0xFF, 0xFF, 0xFF);
            reSetError();
            break;
        default:
            printf("Invalid state\n");
            reSetError();
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
                    //printf("Locker %d is still open after 1 minutes!\n", i + 1);
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
