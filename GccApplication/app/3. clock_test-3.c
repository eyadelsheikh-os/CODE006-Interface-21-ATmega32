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

#define EEPROM_COUNTER0			0x00		/**< Counter0 address in EEPROM */
#define EEPROM_COUNTER1			0x01		/**< Counter1 address in EEPROM */
#define EEPROM_COUNTER2			0x02		/**< Counter2 address in EEPROM */
#define EEPROM_COUNTER3			0x03		/**< Counter3 address in EEPROM */

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
 volatile u8_t counter[4]	= {0, 0, 0, 0};	// clk5, clk6, clk7, clk8

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

/* ===== Display Units Controller Pins ===== */
LED_t ctrl[4] = {
	{ .pin = GPIO_PIN(PORT_A, PIN_0), .active = LED_ACTIVE_LOW },
	{ .pin = GPIO_PIN(PORT_A, PIN_1), .active = LED_ACTIVE_LOW },
	{ .pin = GPIO_PIN(PORT_A, PIN_2), .active = LED_ACTIVE_LOW },
	{ .pin = GPIO_PIN(PORT_A, PIN_3), .active = LED_ACTIVE_LOW }
};

/* ============================================
 * Application Functions
 * ============================================ */
static inline void boundary_check(void)
{
	/* ===== Counter 5 ===== */
	if (counter[0] > 5)
	{
		counter[0] = 0;
		counter[1]++;
	}
	
	/* ===== Counter 6 ===== */
	if (counter[1] > 6)
	{
		counter[1] = 0;
		counter[2]++;
	}

	/* ===== Counter 7 ===== */
	if (counter[2] > 7)
	{
		counter[2] = 0;
		counter[3]++;
	}
	
	/* ===== Counter 8 ===== */
	if (counter[3] > 8)
	{
		counter[3] = 0;
	}
	
	/* Save Counters */
	EEPROM_Write(EEPROM_COUNTER0, counter[0]);
	EEPROM_Write(EEPROM_COUNTER1, counter[1]);
	EEPROM_Write(EEPROM_COUNTER2, counter[2]);
	EEPROM_Write(EEPROM_COUNTER3, counter[3]);
}

/* ============================================
 * Main Application
 * ============================================ */
int main(void) {
    /* ===== Initialize Display ===== */
    SEG_Init(&seg);
    
    /* ===== Initialize Controller Pins ===== */
	for (u8_t i = 0; i < 4; i++) LED_Init(&ctrl[i]);
	
    /* ===== Load Counters ===== */
   if (EEPROM_Read(EEPROM_COUNTER0) != 0xFF)
	{
		counter[0] = EEPROM_Read(EEPROM_COUNTER0);
		counter[1] = EEPROM_Read(EEPROM_COUNTER1);
		counter[2] = EEPROM_Read(EEPROM_COUNTER2);
		counter[3] = EEPROM_Read(EEPROM_COUNTER3);
	}
	
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
	
    /* fps timer */
    TC2_vReset();
	TC2_vSetMode(TC2_MODE_CTC);
	TC2_vSetClock(TC2_CLK_DIV1024);
	TC2_vSetCompareValue(28);	// 70 fps
	TC2_vSetOverflowInt(TRUE);

    /* Initialize timers */
	TC2_Init();
	
	/* ===== Interrupt Configuration ===== */
	INT_GlobalEnable();
	
    /* ===== Main Loop ===== */
    while(TRUE);
}
	
/* ============================================
 * Interrupts
 * ============================================ */
	
/* USART Interrupts */
ISR(USART_RXC_vect)		// USART receive complete
{
	Request_t request;
	
	if (USART_IsError())
	{	
		USART_SendByte(REQ_RE_SEND);
		return;
	}
	
	request = USART_ReceiveByte();

	switch (request)
	{ 
		/* ===== Counter 5 ===== */
		case REQ_CLK5_ACC_INC:
			counter[0]++;
			boundary_check();
			break;
		
	    case REQ_CLK5_INC:
			if (counter[0] != 5)	counter[0]++;
			else					counter[0] = 0;
			break;
			
		case REQ_CLK5_DEC:
			if (counter[0])	counter[0]--;
			else			counter[0] = 5;
			break;
			
		/* ===== Counter 6 ===== */
		case REQ_CLK6_INC:
			if (counter[1] != 6)	counter[1]++;
			else					counter[1] = 0;
			break;
			
		case REQ_CLK6_DEC:
			if (counter[1])	counter[1]--;
			else			counter[1] = 6;
			break;
			
		/* ===== Counter 7 ===== */
		case REQ_CLK7_INC:
			if (counter[2] != 7)	counter[2]++;
			else					counter[2] = 0;
			break;
		
		case REQ_CLK7_DEC:
			if (counter[2])	counter[2]--;
			else			counter[2] = 7;
			break;
		
		/* ===== Counter 8 ===== */
		case REQ_CLK8_INC:
			if (counter[3] != 8)	counter[3]++;
			else					counter[3] = 0;
			break;
		
		case REQ_CLK8_DEC:
			if (counter[3])	counter[3]--;
			else			counter[3] = 8;
			break;
		
		/* ===== Fail ===== */
		default:
			USART_SendByte(request);
			break;
	}
}

/* Timer Interrupts*/
ISR(TIMER2_OVF_vect) // Timer2 overflow interrupt
{
	static u8_t index = 0;
	static u8_t index_sub = 0;
	
	index_sub = index++;
	if (index > 3) index = 0;
	
	LED_Off(&ctrl[index_sub]);	// disable segment
	SEG_DisplayDigit(&seg, counter[index]);
	LED_On(&ctrl[index]);	// enable segment
}
