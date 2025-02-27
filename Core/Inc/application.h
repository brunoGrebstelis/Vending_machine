/*
 * application.h
 *
 *  Created on: Jan 30, 2025
 *      Author: bruno
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#include "main.h"
#include "aht20_sensor.h"
#include "error_handler.h"
#include "print.h"
#include "spi_rpi.h"
#include "climate.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>



void init_application();
void loop_application();


#endif /* INC_APPLICATION_H_ */
