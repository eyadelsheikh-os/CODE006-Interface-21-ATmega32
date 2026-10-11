/**
 * @file    hal_sevenseg_bcd.c
 * @brief   HAL 7-Segment Display Driver Implementation (BCD Decoder)
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    For 74LS47/74HC47 with common cathode or common anode.
 *          Handles polarity automatically via active type.
 *          Common pin is hardwired - no software control needed.
 */

#include "hal_sevenseg_bcd.h"
/* ============================================
 * Private Helper Functions
 * ============================================ */

/**
 * @brief   Write BCD value to the 4 data pins (handles active type).
 * @param   seg     Pointer to the SevenSeg_BCD_t object.
 * @param   value   4-bit BCD value (0-15).
 */
static inline void SEG_BCD_WriteData(const SevenSeg_BCD_t* seg, u8_t value) 
{
    for (u8_t i = 0; i < 4; i++) 
	{
        if (READ_BIT(value, i)) 
		{
            /* Bit is 1 - write based on active type */
            if (seg->active == LED_ACTIVE_HIGH) 
			{
                DIO_PinWrite(seg->data[i], LEVEL_HIGH);
            } 
			else 
			{
                DIO_PinWrite(seg->data[i], LEVEL_LOW);
            }
        } 
		else 
		{
            /* Bit is 0 - write based on active type */
            if (seg->active == LED_ACTIVE_HIGH) 
			{
                DIO_PinWrite(seg->data[i], LEVEL_LOW);
            } 
			else 
			{
                DIO_PinWrite(seg->data[i], LEVEL_HIGH);
            }
        }
    }
}


/* ============================================
 * Public Function Implementations
 * ============================================ */
SevenSeg_BCD_t SEG_BCD_zInit(GPIO_Pin_t data_pins[4], LED_ActiveType_t active) 
{
    SevenSeg_BCD_t seg;
    
    /* Store pins and active type */
    for (u8_t i = 0; i < 4; i++) 
	{
        seg.data[i] = data_pins[i];
    }
    seg.active = active;
    
    /* Initialize all pins as outputs */
    for (u8_t i = 0; i < 4; i++) 
	{
        DIO_PinInit(seg.data[i], DIR_OUTPUT);
    }
    
    /* Start with display off */
    SEG_BCD_Clear(&seg);
    
    return seg;
}

void SEG_BCD_Init(const SevenSeg_BCD_t* seg)
{
	/* Initialize all pins as outputs */
	for (u8_t i = 0; i < 4; i++)
	{
		DIO_PinInit(seg->data[i], DIR_OUTPUT);
	}
	
	/* Start with display off */
	SEG_BCD_Clear(seg);
}

void SEG_BCD_DisplayDigit(const SevenSeg_BCD_t* seg, u8_t digit) 
{
    /* Valid range: 0-9 */
    if (digit > 9) 
	{
        digit = 0;  /* Fallback to 0 for safety */
    }
    
    /* Write BCD value to data pins */
    SEG_BCD_WriteData(seg, digit);
}

void SEG_BCD_DisplayHex(const SevenSeg_BCD_t* seg, u8_t hex) 
{
    /* Valid range: 0-15 */
    if (hex > 15) {
        hex = 0;  /* Fallback to 0 for safety */
    }
    
    /* Write BCD/Hex value to data pins */
    SEG_BCD_WriteData(seg, hex);
}

void SEG_BCD_Clear(const SevenSeg_BCD_t* seg) 
{
    /* 74LS47 blanks on 0x0F (invalid BCD) */
    /* For 74LS48, 0x0F shows "F" - adjust as needed */
    SEG_BCD_WriteData(seg, 0x0F);
}