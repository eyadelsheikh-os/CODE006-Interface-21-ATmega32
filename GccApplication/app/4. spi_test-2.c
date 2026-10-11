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
 * Application Variables
 * ============================================ */
volatile u8_t temperature = 30;		// (°C)

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
	
	/* ===== SPI Configuration ===== */
	SPI_vReset();
	SPI_vSetMode(SPI_MODE_SLAVE);
	SPI_vSetClockMode(SPI_MODE1);
	SPI_vSetTransferInt(TRUE);
	
	/* Initialize SPI */
	SPI_Init();

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
	float temp = (float) 100 * ADC_Read() * 5 / (float) 1024; 
	temperature = (u8_t) temp; 
	SPI_SlaveSend(temperature);
}
	 
/* SPI Interrupts */
ISR(SPI_STC_vect)	// SPI transfer complete
{
	u8_t rank = SPI_SlaveReceive();
	
	if (rank < 4)
	{	
		for (u8_t i = 0; i < 4; i++) LED_Off(&ctrl[i]);
		LED_On(&ctrl[rank]);
	}
	
	ADC_StartConversion();
}