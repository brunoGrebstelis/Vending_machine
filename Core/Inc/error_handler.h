/*
 * error_handler.h
 *
 *  Created on: Feb 7, 2025
 *      Author: bruno
 */

#ifndef INC_ERROR_HANDLER_H_
#define INC_ERROR_HANDLER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "spi_rpi.h"
#include "main.h"


#define LOCKER_CHECK_DELAY 60000  // e.g. 5 minutes in ms



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


extern SystemErrorState errorState;
extern uint8_t error_locker;
extern bool error_flag;

extern bool lockerFlag;
extern bool lockerOpened[24];
extern bool checkPending[24];
extern uint64_t openTimestamp[24];




bool get_error_flag();
bool get_locker_flag();
void setError(SystemErrorState state, uint8_t err_locker);
void HandleState();
void CheckAllLockersAfterDelay(void);



#endif /* INC_ERROR_HANDLER_H_ */
