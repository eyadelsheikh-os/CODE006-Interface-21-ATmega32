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
#include "_ud_library.h"			/* hal_lcd4.h */

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

/* ===== Define LCD Pins ===== */
LCD_4Bit_t lcd = {
	.rs = GPIO_PIN(PORT_B, PIN_0),
	.rw = GPIO_PIN(PORT_B, PIN_1),
	.en = GPIO_PIN(PORT_B, PIN_2),
	
	.d4 = GPIO_PIN(PORT_A, PIN_4),
	.d5 = GPIO_PIN(PORT_A, PIN_5),
	.d6 = GPIO_PIN(PORT_A, PIN_6),
	.d7 = GPIO_PIN(PORT_A, PIN_7)
};

/* ===== Define Keypad Pins ===== */
Keypad_t keypad = {.port = PORT_D};

/* ============================================
 * Application Functions
 * ============================================ */
static inline void print_lcd(LCD_4Bit_t* lcd, u8_t key)
{
	static u8_t current_row = 0;
	static u8_t current_col = 0;

	if (key >= '0' && key <= '9') {
		LCD_SendChar(lcd, key);
		current_col++;
	}
	else if (key == 'A') { LCD_SendChar(lcd, 'A'); current_col++; }
	else if (key == 'B') { LCD_SendChar(lcd, 'B'); current_col++; }
	else if (key == 'C') { LCD_SendChar(lcd, 'C'); current_col++; }
	else if (key == 'D') { LCD_SendChar(lcd, 'D'); current_col++; }
	else if (key == '#') { LCD_SendChar(lcd, 'E'); current_col++; }
	else if (key == '*') { LCD_SendChar(lcd, 'F'); current_col++; }

	if (current_col > 15)
	{
		if (current_row == 0)
		{
			LCD_GoTo(lcd, 1, 0);
			current_row = 1;
		}
		else
		{
			LCD_GoTo(lcd, 0, 0);
			current_row = 0;
		}
		current_col = 0;
	}
}

/* ============================================
 * Main Application
 * ============================================ */
int main(void) {
    /* ===== Initialize Display ===== */
    LCD_Init(&lcd);
    
    /* ===== Initialize Keypad ===== */
    KEYPAD_Init(&keypad);

    /* ===== Main Loop ===== */
    while(TRUE)
    {
	    /* ===== Read Keypad ===== */
	    u8_t key = KEYPAD_JustPressed(&keypad);
		
		print_lcd(&lcd, key);
	    
	    _delay_ms(DELAY_MS);
    }
}