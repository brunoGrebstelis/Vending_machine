/*
 * print.c
 *
 *  Created on: Feb 7, 2025
 *      Author: bruno
 */

#include "print.h"

extern UART_HandleTypeDef huart3;


#ifdef __GNUC__
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
#endif
