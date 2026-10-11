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

#define SOUND		34892	/**< Speed of Sound */

#define EEPROM_COUNTER100	0x00	/**< Counter100 address in EEPROM */
#define EEPROM_COUNTER010	0x01	/**< Counter010 address in EEPROM */
#define EEPROM_COUNTER001	0x02	/**< Counter001 address in EEPROM */

/* ============================================
 * Application Variables
 * ============================================ */
volatile u8_t counter100 = 4;
volatile u8_t counter010 = 5;
volatile u8_t counter001 = 0;

volatile u16_t distance = 450;
volatile u16_t frequency = 38;
volatile u16_t top = 25793;

/* ============================================
 * Application Objects
 * ============================================ */

/* ===== Define Keypad Pins ===== */
Keypad_t keypad = {.port = PORT_C};

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

/* ============================================
 * Application Functions
 * ============================================ */
static inline void Key2Action(u8_t key)
{
	switch (key)
	{
		/* ===== counter100 ===== */
		case '1':
			if (counter100 == 4)	counter100 = 0;
			else					counter100++;
			EEPROM_Write(EEPROM_COUNTER100, counter100);	/* Save Counter */
			break;
		
		case '4':
			if (counter100 == 0)	counter100 = 4;
			else					counter100--;
			EEPROM_Write(EEPROM_COUNTER100, counter100);	/* Save Counter */
			break;
			
		/* ===== counter010 ===== */
		case '2':
			if (counter010 == 9)	counter010 = 0;
			else					counter010++;
			EEPROM_Write(EEPROM_COUNTER010, counter010);	/* Save Counter */
			break;
		
		case '5':
			if (counter010 == 0)	counter010 = 9;
			else					counter010--;
			EEPROM_Write(EEPROM_COUNTER010, counter010);	/* Save Counter */
			break;


		/* ===== counter001 ===== */
		case '3':
			if (counter001 == 9)	counter001 = 0;
			else					counter001++;
			EEPROM_Write(EEPROM_COUNTER001, counter001);	/* Save Counter */
			break;

		case '6':
			if (counter001 == 0)	counter001 = 9;
			else					counter001--;
			EEPROM_Write(EEPROM_COUNTER001, counter001);	/* Save Counter */
			break;
	}
}

static inline void UpdateDistance(void)
{
	distance = counter001 + counter010 * 10 + counter100 * 100;
	if (distance > 450) {
		distance = 450;
		counter100 = 4;
		counter010 = 5;
		counter001 = 0;
		EEPROM_Write(EEPROM_COUNTER100, counter100);	/* Save Counter */
		EEPROM_Write(EEPROM_COUNTER010, counter010);	/* Save Counter */
		EEPROM_Write(EEPROM_COUNTER001, counter001);	/* Save Counter */
	} else if (distance < 2) {
		distance = 2;
		counter100 = 0;
		counter010 = 0;
		counter001 = 2;
		EEPROM_Write(EEPROM_COUNTER100, counter100);	/* Save Counter */
		EEPROM_Write(EEPROM_COUNTER010, counter010);	/* Save Counter */
		EEPROM_Write(EEPROM_COUNTER001, counter001);	/* Save Counter */
	}
}

static inline void UpdateFreq(void)
{
	/* Guard against divide-by-zero */
    if (distance == 0) {
        frequency = 0;
        top       = 0xFFFF;         /* effectively no output */
        return;
    }
	float freq = (float)SOUND / ((float)distance * 2.0f * 2.0f);
	if (freq > 9000.0f) freq = 9000.0f;
	frequency = (u16_t)(freq + 0.5f);
	top = (u16_t)((float)F_CPU / 8.0f / (freq * 2.0f) + 0.5f) - 1;
}

static inline void UpdateTMR(void)
{
	TC1_SetTopValue(top);
}

static inline void UpdateLCD(void)
{
	u16_t temp = distance;
	LCD_GoTo(&lcd, 0, 12);
	for (u8_t i = 0; i < 3; i++)
	{
		u8_t ch;
		if (temp == 0) ch = ' '; 
		else ch = temp % 10 + '0';
		LCD_SendChar(&lcd, ch);
		temp /= 10;
	}
	
	temp = frequency;
	LCD_GoTo(&lcd, 1, 12);
	for (u8_t i = 0; i < 4; i++)
	{
		u8_t ch;
		if (temp == 0) ch = ' ';
		else ch = temp % 10 + '0';
		LCD_SendChar(&lcd, ch);
		temp /= 10;
	}
}

static void UpdateAll(void)
{
	UpdateDistance();
	UpdateFreq();
	UpdateTMR();
	UpdateLCD();
}

/* ============================================
 * Main Application
 * ============================================ */
int main(void) {
	/* ===== Load Counters ===== */
    if (EEPROM_Read(EEPROM_COUNTER100) != 0xFF) counter100 = EEPROM_Read(EEPROM_COUNTER100);
    if (EEPROM_Read(EEPROM_COUNTER010) != 0xFF) counter010 = EEPROM_Read(EEPROM_COUNTER010);
    if (EEPROM_Read(EEPROM_COUNTER001) != 0xFF) counter001 = EEPROM_Read(EEPROM_COUNTER001);
	
    /* ===== Initialize Keypad ===== */
    KEYPAD_Init(&keypad);
	
	/* ===== Initialize Display ===== */
    LCD_Init(&lcd);
	
	LCD_GoTo(&lcd, 0, 0);
	LCD_SendString(&lcd, "D.Dist.");
	LCD_GoTo(&lcd, 1, 0);
	LCD_SendString(&lcd, "G.Freq.");
	LCD_GoTo(&lcd, 0, 13);
	LCD_SendString(&lcd, " cm");
	LCD_GoTo(&lcd, 1, 13);
	LCD_SendString(&lcd, " Hz");
    
	LCD_Home(&lcd);
    LCD_SendCommand(&lcd, LCD_CMD_ENTRY_ARABIC);
    
	/* ===== Timers Configuration ===== */
	TC1_vReset();
	TC1_vSetClock(TC1_CLK_DIV8);
	TC1_vSetMode(TC1_MODE_CTC_ICR1);
	TC1_vSetTopValue(top);
	TC1_vSetOutputAMode(OC1_MODE_TOGGLE);
	
	/* Initialize timers */
	TC1_Init();

	/* ===== Interrupt Configuration ===== */
	INT_GlobalEnable();
	
    /* ===== Main Loop ===== */
	UpdateAll();
    while(TRUE)
	{
		/* ===== Read Keypad ===== */
	    u8_t key = KEYPAD_JustPressed(&keypad);
		if (key)
		{
			Key2Action(key);
			UpdateAll();
		}
	    _delay_ms(DELAY_MS);
    }
}

