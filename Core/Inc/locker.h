/*
 * lock.h
 *
 *  Created on: Feb 6, 2025
 *      Author: bruno
 */

#ifndef INC_LOCKER_H_
#define INC_LOCKER_H_

#include "print.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "error_handler.h"



uint8_t calculate_checksum(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3);
void open_cabinet(uint8_t locker_id);
int read_cabinet_status(uint8_t locker_id);
void RS485_SetTransmitMode(void);
void RS485_SetReceiveMode(void);
void RS485_Transmit(uint8_t *data, uint16_t size);
void RS485_Receive(uint8_t *data, uint16_t size, uint32_t timeout);


#endif /* INC_LOCKER_H_ */
