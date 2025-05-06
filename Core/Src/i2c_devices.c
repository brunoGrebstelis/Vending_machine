/*
 * rgb_led.c
 *
 *  Created on: Feb 6, 2025
 *      Author: bruno
 */

#include "i2c_devices.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c4;


/*
 * Sane once‑per‑fault latching for RGB LED drivers and price‑tags.
 * All braces on one line, minimal comments.
 */

#define LED_BASE_ADDR  100u   /* first LED driver I²C address */
#define LED_COUNT      15u

#define PRICE_BASE_ADDR 0u    /* first price‑tag I²C address (see note below) */
#define PRICE_COUNT     15u

static uint8_t RGB_Buffer[4];
static uint8_t Price_Buffer[2];

static bool rgb_error_sent[LED_COUNT];
static bool price_error_sent[PRICE_COUNT];

static inline uint8_t led_idx(uint16_t addr){return (uint8_t)(addr - LED_BASE_ADDR);}   /* 0 … LED_COUNT‑1 */
static inline uint8_t price_idx(uint16_t addr){return (uint8_t)(addr - PRICE_BASE_ADDR);} /* 0 … PRICE_COUNT‑1 */

void ErrorLatch_Init(void){
    for(uint8_t i = 0; i < LED_COUNT; ++i)  rgb_error_sent[i]   = true;
    for(uint8_t i = 0; i < PRICE_COUNT; ++i)price_error_sent[i] = true;
}

void I2C_Master_Send_Byte(uint16_t addr, uint8_t val){
    HAL_I2C_Master_Transmit(&hi2c1, addr << 1, &val, 1, 10);
}

void Send_RGB(uint16_t addr, uint8_t r, uint8_t g, uint8_t b, uint8_t mode){
    uint8_t idx = led_idx(addr); if(idx >= LED_COUNT) return;
    RGB_Buffer[0] = r; RGB_Buffer[1] = g; RGB_Buffer[2] = b; RGB_Buffer[3] = mode;
    //if(HAL_I2C_Master_Transmit_DMA(&hi2c1, addr << 1, RGB_Buffer, 4) != HAL_OK){
    if(HAL_I2C_Master_Transmit(&hi2c1, addr << 1, RGB_Buffer, 4, 10) != HAL_OK){
    	HAL_I2C_DeInit(&hi2c1);
    	HAL_I2C_Init(&hi2c1);
        if(rgb_error_sent[idx]){setErrorState(STATE_LED_DRIVER); error_locker = idx; error_flag = true; rgb_error_sent[idx] = false;}
    }else rgb_error_sent[idx] = true;
}

void Send_Price(uint16_t addr, uint8_t b1, uint8_t b2){
    uint8_t idx = price_idx(addr); if(idx >= PRICE_COUNT) return;
    Price_Buffer[0] = b1; Price_Buffer[1] = b2;
    //if(HAL_I2C_Master_Transmit_DMA(&hi2c1, addr << 1, Price_Buffer, 2) != HAL_OK){
    if(HAL_I2C_Master_Transmit(&hi2c1, addr << 1, Price_Buffer, 2, 10) != HAL_OK){
    	HAL_I2C_DeInit(&hi2c1);
    	HAL_I2C_Init(&hi2c1);
        if(price_error_sent[idx]){setErrorState(STATE_PRICE_TAG); error_locker = addr; error_flag = true; price_error_sent[idx] = false;}
    }else price_error_sent[idx] = true;
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c){if(hi2c->Instance == I2C1){/* optional debug */}}

/*
 * Note on PRICE_BASE_ADDR = 0u
 * -------------------------
 * I²C address 0x00 is reserved for "general call" and most slaves NACK it.
 * If your price‑tag modules are really configured at 0, consider moving them
 * to 1…127 instead; otherwise keep PRICE_BASE_ADDR as the lowest valid tag
 * address and adjust PRICE_COUNT accordingly.
 */
