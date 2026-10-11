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
u8_t rank[3] = {1, 2, 3};

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
	
/* ===== Display Units Controller Pins ===== */
LED_t ctrl[3] = {
	{ .pin = GPIO_PIN(PORT_D, PIN_2), .active = LED_ACTIVE_LOW },
	{ .pin = GPIO_PIN(PORT_D, PIN_1), .active = LED_ACTIVE_LOW },
	{ .pin = GPIO_PIN(PORT_D, PIN_0), .active = LED_ACTIVE_LOW },
};


/* ============================================
 * Application Functions
 * ============================================ */
static inline void PrintTemperature(void)
{
	u8_t temp = temperature;
	
	LCD_GoTo(&lcd, 0, 12);
	for (u8_t i = 0; i < 2; i++)
	{
		if (temp == 0)
		{
			LCD_SendChar(&lcd, ' ');
			continue;
		}
		
		LCD_SendChar(&lcd, temp % 10 + '0');
		temp /= 10;
	}
}

static u8_t TransferData(const LED_t* slave, u8_t rank)
{
	u8_t data;
    LED_On(slave);
    data = SPI_Transfer(rank);
    LED_Off(slave);
	return data;
}

void bubble_sort(u8_t *arr, u8_t size)
{
	u8_t flag, temp;
	
	for(u8_t i = size - 1; i > 0; i--)
	{
		flag=0;
		for(u8_t j = 0; j < i; j++)
		{
			if(arr[j+1] < arr[j])
			{
				temp = arr[j];
				arr[j] = arr[j+1];
				arr[j+1] = temp;
				flag=1;
			}
		}

		if(flag==0)
		{
			break;
		}
	}
}

u8_t find_outlier(u8_t x1, u8_t x2, u8_t x3)
{
	u8_t gap_low  = x2 - x1;
	u8_t gap_high = x3 - x2;
	   
	// Check if x1 or x3 is far from the middle
	if (gap_high > 5 && gap_high > (3 * gap_low))	return x3;  // high outlier
	if (gap_low  > 5 && gap_low  > (3 * gap_high))	return x1;  // low outlier
	return 0;  // no clear outlier
}


/* ============================================
 * Main Application
 * ============================================ */
int main(void) {
    /* ===== Initialize Display ===== */
    LCD_Init(&lcd);
	
    LCD_SendString(&lcd, "Temp.");
	LCD_GoTo(&lcd, 0, 13);
	LCD_SendString(&lcd, " ");
	LCD_SendChar(&lcd, 0xDF);
	LCD_SendString(&lcd, "C");
	 
    LCD_SendCommand(&lcd, LCD_CMD_ENTRY_ARABIC);
    
    /* ===== Initialize Controller Pins ===== */
    for (u8_t i = 0; i < 3; i++) { LED_Init(&ctrl[i]); LED_Off(&ctrl[i]); }
	
	/* ===== SPI Configuration ===== */
	SPI_vReset();
	SPI_vSetMode(SPI_MODE_MASTER);
	SPI_vSetClockMode(SPI_MODE1);
	SPI_vSetClockRate(SPI_CLK_DIV128);
	
	/* Initialize SPI */
	SPI_Init();
	
	/* ===== Timers Configuration ===== */
	TC1_vReset();
	TC1_vSetMode(TC1_MODE_CTC_OCR1A);
	TC1_vSetClock(TC1_CLK_DIV256);
	TC1_vSetCompareAValue(15624);	// 2 interrupts per second
	TC1_vSetCompareAInt(TRUE);
    
	/* Initialize Timers */
	TC1_Init();
			
	/* ===== Interrupt Configuration ===== */
	INT_GlobalEnable();
	
    /* ===== Main Loop ===== */
    while(TRUE);
}

/* ============================================
 * Interrupts
 * ============================================ */

/* Timer Interrupts*/
ISR(TIMER1_COMPA_vect)	// Timer1 compare A match interrupt
{
    /* ===== Transfer Data ===== */
    u8_t reading1 = TransferData(&ctrl[0], rank[0]);
    u8_t reading2 = TransferData(&ctrl[1], rank[1]);
    u8_t reading3 = TransferData(&ctrl[2], rank[2]);
	u8_t arr[3] = {reading1, reading2, reading3};
	
	bubble_sort(arr, 3);
    /* ===== Outlier ===== */
	u8_t outlier = find_outlier(arr[0], arr[1], arr[2]);
	
	if (reading1 == arr[2]) rank[0] = 1;
	else if (reading1 == arr[1]) rank[0] = 2;
	else if (reading1 == arr[0]) rank[0] = 3;
	if (reading1 == outlier) rank[0] = 0;
		
	if (reading2 == arr[2]) rank[1] = 1;
	else if (reading2 == arr[1]) rank[1] = 2;
	else if (reading2 == arr[0]) rank[1] = 3;
	if (reading2 == outlier) rank[1] = 0;
	
		
	if (reading3 == arr[2]) rank[2] = 1;
	else if (reading3 == arr[1]) rank[2] = 2;
	else if (reading3 == arr[0]) rank[2] = 3;
	if (reading3 == outlier) rank[2] = 0;
	
	if (!outlier) temperature = (u8_t)(reading1 + reading2 + reading3) / 3;
	else temperature = (u8_t)(reading1 + reading2 + reading3 - outlier) / 2;
	
	PrintTemperature();
}

