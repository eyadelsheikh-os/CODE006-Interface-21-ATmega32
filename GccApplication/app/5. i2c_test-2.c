/**
 * @file    main.c
 * @brief   LED Chaser with Toggle-on-Button-Release Application
 * @author  eyads
 * @date    2026-05-07
 *
 * @note    This application implements a bouncing LED chaser.
 *          Pressing and releasing a button toggles the LED pattern
 *          between two banks of LEDs (left and right).
 */

/* ============================================
 * User Defined Library Include
 * ============================================ */
#include "_ud_library.h"

/* ============================================
 *  AVR Library Includes
 * ============================================ */
#include <util/delay.h>
#include <avr/interrupt.h>

/* ============================================
 * Configuration Constants
 * ============================================ */
#define DELAY_MS	200		/**< Simple fps delay */

/* ============================================
 * Application Constants
 * ============================================ */

/* 7-bit I2C addresses — all outside reserved ranges
 * (0x00-0x07 general call, 0x50-0x57 EEPROM, 0x78-0x7F reserved) */
#define ID_MMC0		0x0A
#define ID_SMC0		0x10
#define ID_SMC1		0x11
#define ID_SMC2		0x12

/* ============================================
 * Application Variables
 * ============================================ */
volatile u8_t temperature = 30;		// (°C)
volatile u8_t rank = 0;

/* ============================================
 * Application Objects
 * ============================================ */
	
/* ===== Display Units Controller Pins ===== */
LED_t ctrl[4] = {
	{ .pin = GPIO_PIN(PORT_A, PIN_7), .active = LED_ACTIVE_LOW },
	{ .pin = GPIO_PIN(PORT_A, PIN_4), .active = LED_ACTIVE_LOW },
	{ .pin = GPIO_PIN(PORT_A, PIN_5), .active = LED_ACTIVE_LOW },
	{ .pin = GPIO_PIN(PORT_A, PIN_6), .active = LED_ACTIVE_LOW }
};

/* ============================================
 * Application Functions
 * ============================================ */


/* ============================================
 * Main Application
 * ============================================ */
int main(void) {
    /* ===== Initialize Controller Pins ===== */
    for (u8_t i = 0; i < 4; i++) LED_Init(&ctrl[i]);
	
	/* ===== ADC Configuration (for heat sensor) ===== */
	ADC_vReset();
	ADC_vSetChannel(ADC_CH0);
	ADC_vSetReference(ADC_REF_AREF);
	ADC_vSetClock(ADC_CLK_DIV128);
	ADC_vSetConversionInt(TRUE);
	
    /* Initialize ADC */
	ADC_Init();
	ADC_StartConversion();
	
	/* ===== I2C Configuration ===== */
	I2C_vReset();
	I2C_vSetAddress(ID_SMC0);
	I2C_vSetInterrupt(TRUE);
	
	/* Initialize I2C */
	I2C_Init();
	

	/* ===== Interrupt Configuration ===== */
	INT_GlobalEnable();
	
    /* ===== Main Loop ===== */
    while(TRUE);
}

/* ============================================
 * Interrupts
 * ============================================ */

 /* ADC Interrupts */
 ISR(ADC_vect)		// ADC conversion complete
 {	
	float temp = (float)100 * ADC_Read() * 5 / 1024.0f;
	temperature = (u8_t) temp; 
}

/* I2C Interrupts */
ISR(TWI_vect) // TWI event action
{
    switch (I2C_GetStatus()) {

	    /* ---- Data byte received ---- */
	    case I2C_SR_DATA_ACK:       /* 0x80 */
			rank = I2C_SlaveReadByte();   /* returns TWDR */
			for (u8_t i = 0; i < 4; i++) LED_Off(&ctrl[i]);
			if (rank < 4) LED_On(&ctrl[rank]);
			I2C_Ack();
			break;

	    /* ---- Master wants to read from us ---- */
	    case I2C_SR_SLA_R_ACK:      /* 0xA8 */
	    case I2C_SR_DATA_TX_ACK:    /* 0xB8 */
			I2C_SlaveWriteByte(temperature);
			ADC_StartConversion();
			I2C_Ack();
			break;

	    /* ---- Anything else: keep the bus alive ---- */
	    default:
			I2C_Ack();
			break;
    }
}
