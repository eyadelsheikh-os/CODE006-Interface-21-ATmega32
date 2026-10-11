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

/* ============================================
 * Configuration Constants
 * ============================================ */
#define DELAY_MS	200		/**< Simple fps delay */

/* ============================================
 * Application Variables
 * ============================================ */

/* ============================================
 * Application Objects
 * ============================================ */

/* ===== Define 7seg Pins ===== */
SevenSeg_t seg1 = {
	.segments = {
		GPIO_PIN(PORT_A, PIN_0),  // A
		GPIO_PIN(PORT_A, PIN_1),  // B
		GPIO_PIN(PORT_A, PIN_2),  // C
		GPIO_PIN(PORT_A, PIN_3),  // D
		GPIO_PIN(PORT_A, PIN_4),  // E
		GPIO_PIN(PORT_A, PIN_5),  // F
		GPIO_PIN(PORT_A, PIN_6)   // G	
	}
};
	
SevenSeg_t seg2 = {
	.segments = {
		GPIO_PIN(PORT_B, PIN_0),  // A
		GPIO_PIN(PORT_B, PIN_1),  // B
		GPIO_PIN(PORT_B, PIN_2),  // C
		GPIO_PIN(PORT_B, PIN_3),  // D
		GPIO_PIN(PORT_B, PIN_4),  // E
		GPIO_PIN(PORT_B, PIN_5),  // F
		GPIO_PIN(PORT_B, PIN_6)   // G
	}
};

/* ===== Define Keypad Pins ===== */
Keypad_t keypad1 = {.port = PORT_C};
Keypad_t keypad2 = {.port = PORT_D};


/* ============================================
 * Application Functions
 * ============================================ */
static inline void print_seg(const SevenSeg_t* seg, u8_t key)
{
	if (key >= '0' && key <= '9')
	{
		SEG_DisplayDigit(seg, key - '0');
	}
	else if (key == 'A')
	{
		SEG_DisplayHex(seg, 0xA);
	}
	else if (key == 'B')
	{
		SEG_DisplayHex(seg, 0xB);
	}
	else if (key == 'C')
	{
		SEG_DisplayHex(seg, 0xC);
	}
	else if (key == 'D')
	{
		SEG_DisplayHex(seg, 0xD);
	}
	else if (key == '#')
	{
		SEG_DisplayHex(seg, 0xE);
	}
	else if (key == '*')
	{
		SEG_DisplayHex(seg, 0xF);
	}
}


/* ============================================
 * Main Application
 * ============================================ */
int main(void) {
    /* ===== Initialize Display (Common Cathode) ===== */
    SEG_Init(&seg1);
    SEG_Init(&seg2);
    
    /* ===== Initialize Keypad ===== */
    KEYPAD_Init(&keypad1);
    KEYPAD_Init(&keypad2);

    /* ===== Main Loop ===== */
    while(TRUE)
    {
	    /* ===== Read Keypad ===== */
	    u8_t key1 = KEYPAD_JustPressed(&keypad1);
	    u8_t key2 = KEYPAD_JustReleased(&keypad2);
		
		print_seg(&seg1, key1);
		print_seg(&seg2, key2);
	    
	    _delay_ms(DELAY_MS);
    }
}