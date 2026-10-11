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
#define ID_MMC0		0x0A		/* central's own address */
#define ID_SMC0		0x10
#define ID_SMC1		0x11
#define ID_SMC2		0x12

/* ============================================
 * Application Variables
 * ============================================ */
volatile u8_t temperature[3] = {30, 30, 30};	// (°C)
volatile u8_t rank[3] = {0, 0, 0};

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


/* ============================================
 * Application Functions
 * ============================================ */
static inline void PrintTemperature(u8_t temp)
{
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

static bool_t SendRankToSlave(u8_t addr7, u8_t r)
{
	if (!I2C_Start())                               return FALSE;
	if (!I2C_WriteAddress(addr7, I2C_DIR_WRITE))    { I2C_Stop(); return FALSE; }
	if (!I2C_WriteByte(r))                          { I2C_Stop(); return FALSE; }
	I2C_Stop();
	return TRUE;
}

static bool_t ReadTempFromSlave(u8_t addr7, u8_t* out)
{
	if (!I2C_Start())                               return FALSE;
	if (!I2C_WriteAddress(addr7, I2C_DIR_READ))     { I2C_Stop(); return FALSE; }
	*out = I2C_ReadByte(I2C_NACK);
	I2C_Stop();
	return TRUE;
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
    
	/* ===== I2C Configuration ===== */
	I2C_vReset();
	I2C_vSetAddress(ID_MMC0);
	
	/* Initialize I2C */
	I2C_Init();
	
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
    u8_t addr[3] = { ID_SMC0, ID_SMC1, ID_SMC2 };

    /* ---- 1. Read all temperatures ---- */
    for (u8_t i = 0; i < 3; i++) {
	    u8_t t;
	    if (ReadTempFromSlave(addr[i], &t)) {
		    temperature[i] = t;
	    }
    }
	
	/* ---- 2. Compute ranks (1 = hottest, 3 = coldest, 0 = outlier) ---- */
	u8_t sorted[3];
	for (u8_t i = 0; i < 3; i++) sorted[i] = temperature[i];
	bubble_sort(sorted, 3);
		
    /* ===== Outlier ===== */
	u8_t outlier = find_outlier(sorted[0], sorted[1], sorted[2]);
	
	for (u8_t i = 0; i < 3; i++)
	{
		if		(temperature[i] == sorted[2]) rank[i] = 1;
		else if (temperature[i] == sorted[1]) rank[i] = 2;
		else if (temperature[i] == sorted[0]) rank[i] = 3;
		if		(temperature[i] == outlier)   rank[i] = 0;
	}
	
    /* ---- 3. Send ranks back ---- */
    for (u8_t i = 0; i < 3; i++) {
	    for (u8_t try = 0; try < 3; try++) {
		    if (SendRankToSlave(addr[i], rank[i])) break;
	    }
    }
	
    /* ===== Average Temperature ===== */
	u8_t temp;
	
	if (!outlier) temp = (u8_t)(temperature[0] + temperature[1] + temperature[2]) / 3;
	else temp = (u8_t)(temperature[0] + temperature[1] + temperature[2] - outlier) / 2;
	
	PrintTemperature(temp);
}
