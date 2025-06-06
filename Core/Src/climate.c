/*
 * climate.c
 *
 *  Created on: Feb 26, 2025
 *      Author: bruno
 */
#include "climate.h"

uint8_t fanMode = 0;
bool climate_flag = false;
bool auto_flag = false;



void setClimateFlag(bool flag){
	climate_flag = flag;
}
bool getClimateFlag(){
	return climate_flag;
}

void setAutoFlag(bool flag){
	auto_flag = flag;
}
bool getAutoFlag(){
	return auto_flag;
}

void setFanMode() {

	sendLogUART(fanMode, 2);
	//printf("Mode in setFanMode() in climate.c: %d\r\n", fanMode);

    // If 'fanMode' is 255, that indicates Auto => all OFF
    if (fanMode == 255)
    {
        printf("AUTO ON\r\n");
        // Turn everything OFF
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, RESET); // Fan1
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, RESET); // Fan2
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, RESET); // Fan3
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, RESET); // Heat1
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, RESET); // Heat2
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, RESET); // Heat3

        climate_flag = false;
        auto_flag = true;    // We are in auto mode
    }
    else
    {
        // For 0..63, each bit indicates whether a particular output is ON (SET) or OFF (RESET).
        // bit0 => Fan1, bit1 => Fan2, bit2 => Fan3
        // bit3 => Heat1, bit4 => Heat2, bit5 => Heat3

        // 1) Fans
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, (fanMode & (1 << 0)) ? SET : RESET);  // Fan1
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, (fanMode & (1 << 1)) ? SET : RESET);  // Fan2
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, (fanMode & (1 << 2)) ? SET : RESET);  // Fan3

        // 2) Heats
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, (fanMode & (1 << 3)) ? SET : RESET);  // Heat1
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, (fanMode & (1 << 4)) ? SET : RESET);  // Heat2
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, (fanMode & (1 << 5)) ? SET : RESET);  // Heat3

        climate_flag = false;  // Keep same logic as original (all manual modes => climate_flag = false)
        auto_flag = false;     // Auto is off in this branch
    }
}

float CalculateWindowInsideSurface(float T_in, float T_out, float area){
    // 1. Convection Resistances & Conduction
    float R_conv_in  = 1.0f / (H_IN  * area);
    float R_conv_out = 1.0f / (H_OUT * area);
    float R_cond     = L_ACRYLIC / (K_ACRYLIC * area);

    // 2. Total thermal resistance
    float R_total = R_conv_in + R_cond + R_conv_out;

    // 3. Heat flux (W) through the window
    float Q = (T_in - T_out) / R_total;

    // 4. Inside surface temperature
    float T_surf_in = T_in - Q * R_conv_in;

    return T_surf_in;
}

static float CalculateDewPointMagnus(float tempC, float humidity){
    // Example constants for the Magnus-Tetens approximation
    float A = 17.27f;
    float B = 237.7f;  // °C

    // Avoid zero or negative humidity
    if (humidity < 1.0f)  humidity = 1.0f;
    if (humidity > 100.0f) humidity = 100.0f;

    // alpha = [A * T / (B + T)] + ln(RH/100)
    float alpha = (A * tempC) / (B + tempC) + logf(humidity / 100.0f);

    float dewPoint = (B * alpha) / (A - alpha);
    return dewPoint;
}

void ControlClimate(void) {

    // 1) Variables for measured T/H
    float T_outdoor=0.0f, H_outdoor=0.0f;
    float T_ch1=0.0f,    H_ch1=0.0f;
    float T_ch2=0.0f,    H_ch2=0.0f;
    float T_ch3=0.0f,    H_ch3=0.0f;

    // 2) Read from each sensor (PCA9548A channels)
    AHT20_ReadData_PCA9548A(&hi2c2, PCA9548A_ADDRESS, CHANNEL_3, &T_outdoor, &H_outdoor); // outside
    AHT20_ReadData_PCA9548A(&hi2c2, PCA9548A_ADDRESS, CHANNEL_1, &T_ch1,     &H_ch1);     // chamber 1
    AHT20_ReadData_PCA9548A(&hi2c2, PCA9548A_ADDRESS, CHANNEL_0, &T_ch2,     &H_ch2);     // chamber 2
    AHT20_ReadData_PCA9548A(&hi2c2, PCA9548A_ADDRESS, CHANNEL_2, &T_ch3,     &H_ch3);     // chamber 3

    // 3) CHAMBER 1 CONTROL
    {
        // a) Calculate dew point & window inside surface
        float dewCh1 = CalculateDewPointMagnus(T_ch1, H_ch1);
        float T_surf_ch1 = CalculateWindowInsideSurface(T_ch1, T_outdoor, CH1_WINDOW_AREA);

        // b) Decide fan & heater states
        // Start with default OFF
        GPIO_PinState fanCh1 = GPIO_PIN_RESET;
        GPIO_PinState heatCh1 = GPIO_PIN_RESET;

        // (i) Prevent condensation
        if(T_surf_ch1 < dewCh1) {
            fanCh1 = GPIO_PIN_SET; // fan ON to reduce humidity / mix air
        }

        // (ii) Temperature checks
        if(T_ch1 < TEMP_MIN) {
            heatCh1 = GPIO_PIN_SET; // too cold => heater ON
        } else if(T_ch1 > TEMP_MAX) {
            fanCh1 = GPIO_PIN_SET;  // too hot => fan ON
        }

        // (iii) Humidity checks
        //   If humidity is above HUMIDITY_MAX => also run fan
        //   If humidity is below HUMIDITY_MIN => prefer fan OFF unless forced for condensation or high temp
        if(H_ch1 > HUMIDITY_MAX) {
            fanCh1 = GPIO_PIN_SET;
        } else if(H_ch1 < HUMIDITY_MIN && T_surf_ch1 >= dewCh1 && T_ch1 <= TEMP_MAX) {
            // Keep fan OFF to preserve moisture, but only if no condensation or high temp
            fanCh1 = GPIO_PIN_RESET;
        }

        // c) Write final states
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, fanCh1);
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, heatCh1);
    }

    // 4) CHAMBER 2 CONTROL
    {
        float dewCh2 = CalculateDewPointMagnus(T_ch2, H_ch2);
        float T_surf_ch2 = CalculateWindowInsideSurface(T_ch2, T_outdoor, CH2_WINDOW_AREA);

        GPIO_PinState fanCh2 = GPIO_PIN_RESET;
        GPIO_PinState heatCh2 = GPIO_PIN_RESET;

        if(T_surf_ch2 < dewCh2) { fanCh2 = GPIO_PIN_SET; }

        if(T_ch2 < TEMP_MIN) { heatCh2 = GPIO_PIN_SET; }
        else if(T_ch2 > TEMP_MAX) { fanCh2 = GPIO_PIN_SET; }

        if(H_ch2 > HUMIDITY_MAX) { fanCh2 = GPIO_PIN_SET; }
        else if(H_ch2 < HUMIDITY_MIN && T_surf_ch2 >= dewCh2 && T_ch2 <= TEMP_MAX) {
            fanCh2 = GPIO_PIN_RESET;
        }

        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_3, fanCh2);
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, heatCh2);
    }

    // 5) CHAMBER 3 CONTROL
    {
        float dewCh3 = CalculateDewPointMagnus(T_ch3, H_ch3);
        float T_surf_ch3 = CalculateWindowInsideSurface(T_ch3, T_outdoor, CH3_WINDOW_AREA);

        GPIO_PinState fanCh3 = GPIO_PIN_RESET;
        GPIO_PinState heatCh3 = GPIO_PIN_RESET;

        if(T_surf_ch3 < dewCh3) { fanCh3 = GPIO_PIN_SET; }

        if(T_ch3 < TEMP_MIN) { heatCh3 = GPIO_PIN_SET; }
        else if(T_ch3 > TEMP_MAX) { fanCh3 = GPIO_PIN_SET; }

        if(H_ch3 > HUMIDITY_MAX) { fanCh3 = GPIO_PIN_SET; }
        else if(H_ch3 < HUMIDITY_MIN && T_surf_ch3 >= dewCh3 && T_ch3 <= TEMP_MAX) {
            fanCh3 = GPIO_PIN_RESET;
        }

        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, fanCh3);
        HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, heatCh3);
    }
}
