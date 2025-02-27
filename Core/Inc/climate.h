/*
 * climate.h
 *
 *  Created on: Feb 26, 2025
 *      Author: bruno
 */

#ifndef SRC_CLIMATE_H_
#define SRC_CLIMATE_H_



#endif /* SRC_CLIMATE_H_ */


#include <stdint.h>
#include "stm32f7xx_hal.h"
#include "print.h"
#include "main.h"
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include "aht20_sensor.h"


// Convection/Conduction constants
#define K_ACRYLIC          0.20f   // W/m·K (approx. for acrylic)
#define L_ACRYLIC          0.01f   // 10 mm thick => 0.01 m
#define H_IN               8.0f    // W/m²·K (indoor conv. approx)
#define H_OUT              15.0f   // W/m²·K (outdoor conv. approx)

//Chamber areas
#define CH1_WINDOW_AREA    0.054f * 0.057f  // in m²
#define CH2_WINDOW_AREA    0.026f * 0.026f
#define CH3_WINDOW_AREA    0.054f * 0.057f

// Comfort values
#define TEMP_MIN           15.0f   // °C (minimum comfortable temperature)
#define TEMP_MAX           22.0f   // °C (maximum comfortable temperature)
#define HUMIDITY_MAX       85.0f   // %RH (example maximum humidity for flowers)
#define HUMIDITY_MIN       60.0f   // %RH (example minimum humidit

extern uint8_t fanMode;

void setClimateFlag(bool flag);
bool getClimateFlag();
void setAutoFlag(bool flag);
bool getAutoFlag();
void setFanMode();
float CalculateWindowInsideSurface(float T_in, float T_out, float area);
void ControlClimate(void);

