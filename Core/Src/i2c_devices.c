/*
 * i2c_devices.c – I²C helpers for RGB‑LED drivers and price‑tags
 *
 *  Re‑written 2025‑05‑06 ‑ Bruno, based on earlier draft.
 *  – non‑blocking DMA with busy flag + timeout
 *  – automatic peripheral reset on error or hang
 *  – single‑shot error latching
 */

#include "i2c_devices.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c4; /* kept for future use */

/* ----- configuration ---------------------------------------------------- */
#define LED_BASE_ADDR   100u  /* first LED driver address */
#define LED_COUNT        15u

#define PRICE_BASE_ADDR    1u /* first price‑tag address (avoid 0x00) */
#define PRICE_COUNT       15u

#define I2C_TIMEOUT_MS    50u /* max time we wait for a transfer */

/* ----- static storage --------------------------------------------------- */
static uint8_t  RGB_Buffer[4];
static uint8_t  Price_Buffer[2];

static bool     rgb_error_sent [LED_COUNT];
static bool     price_error_sent[PRICE_COUNT];

static volatile bool i2c1_tx_busy = false; /* set before DMA start, cleared in IRQ */

/* ----- helpers ---------------------------------------------------------- */
static inline uint8_t led_idx  (uint16_t addr){return (uint8_t)(addr  - LED_BASE_ADDR);}  /* 0…LED_COUNT‑1 */
static inline uint8_t price_idx(uint16_t addr){return (uint8_t)(addr  - PRICE_BASE_ADDR);}/* 0…PRICE_COUNT‑1 */

static void i2c1_reset(void){HAL_I2C_DeInit(&hi2c1); HAL_I2C_Init(&hi2c1); i2c1_tx_busy=false;}

/* Busy‑wait w/ timeout so a wedged bus doesn’t hang forever */
static bool i2c1_wait_done(uint32_t t0){
    while(i2c1_tx_busy){
        if(HAL_GetTick() - t0 > I2C_TIMEOUT_MS){i2c1_reset(); return false;}
    }
    return true;
}

static bool i2c1_tx_dma(uint16_t addr, uint8_t *buf, uint16_t len){
    uint32_t t_start = HAL_GetTick();

    /* wait if a previous TX is still running */
    if(i2c1_tx_busy && !i2c1_wait_done(t_start)) return false;

    i2c1_tx_busy = true;
    if(HAL_I2C_Master_Transmit_DMA(&hi2c1, addr << 1, buf, len) != HAL_OK){
        i2c1_reset();
        return false;
    }

    return i2c1_wait_done(t_start);
}

/* ----- public API ------------------------------------------------------- */
void ErrorLatch_Init(void){
    for(uint8_t i=0;i<LED_COUNT;i++)  rgb_error_sent [i]=true;
    for(uint8_t i=0;i<PRICE_COUNT;i++)price_error_sent[i]=true;
}

void I2C_Master_Send_Byte(uint16_t addr, uint8_t val){HAL_I2C_Master_Transmit(&hi2c1, addr<<1, &val, 1, 10);} /* tiny payload – just block 10 ms */

void Send_RGB(uint16_t addr, uint8_t r, uint8_t g, uint8_t b, uint8_t mode){
    uint8_t idx = led_idx(addr); if(idx >= LED_COUNT) return;
    RGB_Buffer[0]=r; RGB_Buffer[1]=g; RGB_Buffer[2]=b; RGB_Buffer[3]=mode;
    if(!i2c1_tx_dma(addr, RGB_Buffer, 4)){
        if(rgb_error_sent[idx]){setErrorState(STATE_LED_DRIVER); error_locker=idx; error_flag=true; rgb_error_sent[idx]=false;}
    }else rgb_error_sent[idx]=true;
}

void Send_Price(uint16_t addr, uint8_t b1, uint8_t b2){
    uint8_t idx = price_idx(addr); if(idx >= PRICE_COUNT) return;
    Price_Buffer[0]=b1; Price_Buffer[1]=b2;
    if(!i2c1_tx_dma(addr, Price_Buffer, 2)){
        if(price_error_sent[idx]){setErrorState(STATE_PRICE_TAG); error_locker=addr; error_flag=true; price_error_sent[idx]=false;}
    }else price_error_sent[idx]=true;
}

/* ----- HAL callbacks ---------------------------------------------------- */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c){if(hi2c==&hi2c1) i2c1_tx_busy=false;}
void HAL_I2C_ErrorCallback   (I2C_HandleTypeDef *hi2c){if(hi2c==&hi2c1) i2c1_reset ();}

/*
 * Notes
 * -----
 * 1) DMA is arguably overkill for 2–4‑byte payloads but frees the CPU while the
 *    bus clocks out; cost is one stream.
 * 2) General‑Call (0x00) avoided – most slaves NACK it.  Move real modules to
 *    ≥0x01 and update PRICE_BASE_ADDR/COUNT accordingly.
 */
