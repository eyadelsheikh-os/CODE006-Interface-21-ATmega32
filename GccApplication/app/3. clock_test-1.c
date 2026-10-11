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

#define EEPROM_COUNTER			0x00		/**< Counter address in EEPROM */
 
/* ============================================
 * Application Enums
 * ============================================ */
typedef enum {
    REQ_NULL,				/**< Null signal */
    REQ_CLK1_INC,			/**< Increase clk1 by 1 */
    REQ_CLK1_DEC,			/**< Decrease clk1 by 1 */
    REQ_CLK2_ACC_INC,		/**< Increase clk2 by 1 due to accumulation */
    REQ_CLK2_INC,			/**< Increase clk2 by 1 */
    REQ_CLK2_DEC,			/**< Decrease clk2 by 1 */
    REQ_CLK3_INC,			/**< Increase clk3 by 1 */
    REQ_CLK3_DEC,			/**< Decrease clk3 by 1 */
    REQ_CLK4_INC,			/**< Increase clk4 by 1 */
    REQ_CLK4_DEC,			/**< Decrease clk4 by 1 */
    REQ_CLK5_ACC_INC,		/**< Increase clk5 by 1 due to accumulation */
    REQ_CLK5_INC,			/**< Increase clk5 by 1 */
    REQ_CLK5_DEC,			/**< Decrease clk5 by 1 */
    REQ_CLK6_INC,			/**< Increase clk6 by 1 */
    REQ_CLK6_DEC,			/**< Decrease clk6 by 1 */
    REQ_CLK7_INC,			/**< Increase clk7 by 1 */
    REQ_CLK7_DEC,			/**< Decrease clk7 by 1 */
    REQ_CLK8_INC,			/**< Increase clk8 by 1 */
    REQ_CLK8_DEC,			/**< Decrease clk8 by 1 */
    REQ_RE_SEND,			/**< Data receiving failed, re-send last request */
} Request_t;
 
 /* ============================================
 * Application Variables
 * ============================================ */
 
 /* ===== Counters ===== */
 volatile u8_t counter	= 0;	// clk1
 
 /* ===== Signals ===== */
 volatile u8_t last_key	= 0;

/* ============================================
 * Application Objects
 * ============================================ */

/* ===== Define 7seg Pins ===== */
SevenSeg_t seg = {
	.segments = {
	GPIO_PIN(PORT_C, PIN_0),  // A
	GPIO_PIN(PORT_C, PIN_1),  // B
	GPIO_PIN(PORT_C, PIN_2),  // C
	GPIO_PIN(PORT_C, PIN_3),  // D
	GPIO_PIN(PORT_C, PIN_4),  // E
	GPIO_PIN(PORT_C, PIN_5),  // F
	GPIO_PIN(PORT_C, PIN_6)   // G
	}
};

/* ===== Define Keypad Pins ===== */
Keypad_t keypad = {.port = PORT_A};

/* ============================================
 * Application Functions
 * ============================================ */
static inline void boundary_check(void)
{
	/* ===== Counter 1 ===== */
	if(counter > 1)
 	{
	 	counter = 0;
		USART_SendByte(REQ_CLK2_ACC_INC);
	}

	/* Save Counters */
	EEPROM_Write(EEPROM_COUNTER, counter);
}

static inline void key_handler(u8_t key)
{
	switch (key)
	{
		
		/* ===== clk1 ===== */
		case '1':
			if (counter == 0)	counter++;
			else				counter = 0;
			TC1_SetCounterValue(0);
			SEG_DisplayDigit(&seg, counter);
			break;
			
		case '4':
			if (counter == 1)	counter--;
			else				counter = 1;
			TC1_SetCounterValue(0);
			SEG_DisplayDigit(&seg, counter);
			break;
		
		/* ===== clk2 ===== */
		case '2':
			USART_SendByte(REQ_CLK2_INC);
			break;
		
		case '5':
			USART_SendByte(REQ_CLK2_DEC);
			break;
		
		/* ===== clk3 ===== */
		case '3':
			USART_SendByte(REQ_CLK3_INC);
			break;

		case '6':
			USART_SendByte(REQ_CLK3_DEC);
			break;

		/* ===== clk4 ===== */
		case 'A':
			USART_SendByte(REQ_CLK4_INC);
			break;

		case 'B':
			USART_SendByte(REQ_CLK4_DEC);
			break;

		/* =====clk5 ===== */
		case '7':
			USART_SendByte(REQ_CLK5_INC);
			break;
		
		case '*':
			USART_SendByte(REQ_CLK5_DEC);
			break;
		
		/* =====clk6 ===== */
		case '8':
			USART_SendByte(REQ_CLK6_INC);
			break;
		
		case '0':
			USART_SendByte(REQ_CLK6_DEC);
			break;
		
		/* =====clk7 ===== */
		case '9':
			USART_SendByte(REQ_CLK7_INC);
			break;

		case '#':
			USART_SendByte(REQ_CLK7_DEC);
			break;

		/* =====clk8 ===== */
		case 'C':
			USART_SendByte(REQ_CLK8_INC);
			break;

		case 'D':
			USART_SendByte(REQ_CLK8_DEC);
			break;
	}
}

/* ============================================
 * Main Application
 * ============================================ */
int main(void) {
    /* ===== Initialize Display ===== */
    SEG_Init(&seg);
    
    /* ===== Initialize Keypad ===== */
    KEYPAD_Init(&keypad);
	
    /* ===== Load Counters ===== */
    if (EEPROM_Read(EEPROM_COUNTER) != 0xFF)
    {
	    counter = EEPROM_Read(EEPROM_COUNTER);
    }
	
	SEG_DisplayDigit(&seg, counter);
	
	/* ===== USART Configuration ===== */
	USART_vReset();
	USART_vSetSpeed(USART_SPEED_DOUBLE);
	USART_vSetBaudRegister(416);
	USART_vSetParity(USART_PARITY_EVEN);
	USART_vSetStopBits(USART_STOP_2BIT);
	USART_vSetReceiverInt(TRUE);
	
	/* Initialize USART */
	USART_Init();
	
	/* ===== Timers Configuration ===== */
	TC1_vReset();
	TC1_vSetMode(TC1_MODE_CTC_OCR1A);
	TC1_vSetClock(TC1_CLK_DIV256);
	TC1_vSetCompareAValue(31249);	// 1 interrupt per second
	TC1_vSetCompareAInt(TRUE);
	
	/* Initialize timers */
	TC1_Init();
	
	/* ===== Interrupt Configuration ===== */
	INT_GlobalEnable();
	
	/* ===== Main Loop ===== */
    while(TRUE)
    {
	    /* ===== Read Keypad ===== */
	    u8_t key = KEYPAD_JustPressed(&keypad);
		
		if (key)
		{
			key_handler(key);
			last_key = key;
		}
		
	    _delay_ms(DELAY_MS);
    }
}

/* ============================================
 * Interrupts
 * ============================================ */

/* USART Interrupts */
ISR(USART_RXC_vect)		// USART receive complete
{	
	Request_t request;
	
	if (USART_IsError())	return;
	
	request = USART_ReceiveByte();
	
	if (request == REQ_RE_SEND) key_handler(last_key);
}

/* Timer Interrupts*/
ISR(TIMER1_COMPA_vect)	// Timer1 compare A match interrupt
{
	counter++;
	boundary_check();
	SEG_DisplayDigit(&seg, counter);
}