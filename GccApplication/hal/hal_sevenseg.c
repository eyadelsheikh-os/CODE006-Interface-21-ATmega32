/**
 * @file    hal_sevenseg.c
 * @brief   HAL 7-Segment Display Driver Implementation
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Implements 7-segment display control with lookup table.
 *          Supports both common cathode and common anode.
 */

#include "hal_sevenseg.h"
/* ============================================
 * Lookup Table (Segment Patterns)
 * ============================================ */

/**
 * @brief   7-segment patterns for digits 0-9 and hex A-F.
 * @note    Bit mapping: bit0=A, bit1=B, bit2=C, bit3=D, bit4=E, bit5=F, bit6=G
 * 
 *          Common cathode: 1 = segment ON
 *          Common anode:   0 = segment ON (handled by LED_Write)
 */
static const u8_t SEG_Patterns[16] = {
    0b00111111,  // 0: A B C D E F
    0b00000110,  // 1: B C
    0b01011011,  // 2: A B D E G
    0b01001111,  // 3: A B C D G
    0b01100110,  // 4: B C F G
    0b01101101,  // 5: A C D F G
    0b01111101,  // 6: A C D E F G
    0b00000111,  // 7: A B C
    0b01111111,  // 8: A B C D E F G
    0b01101111,  // 9: A B C D F G
    0b01110111,  // A: A B C E F G
    0b01111100,  // b: C D E F G
    0b00111001,  // C: A D E F
    0b01011110,  // d: B C D E G
    0b01111001,  // E: A D E F G
    0b01110001   // F: A E F G
};


/* ============================================
 * Private Helper Functions
 * ============================================ */

/**
 * @brief   Write a logical state to a segment pin.
 * @param   pin     The GPIO pin.
 * @param   state   TRUE = ON, FALSE = OFF.
 * @param   active  LED_ACTIVE_HIGH or LED_ACTIVE_LOW.
 */
static inline void SEG_WriteSegment(GPIO_Pin_t pin, bool_t state, LED_ActiveType_t active) 
{
    if (active == LED_ACTIVE_HIGH) 
	{
        /* Common cathode: HIGH = ON */
        DIO_PinWrite(pin, (state == TRUE) ? LEVEL_HIGH : LEVEL_LOW);
    } 
	else 
	{
        /* Common anode: LOW = ON */
        DIO_PinWrite(pin, (state == TRUE) ? LEVEL_LOW : LEVEL_HIGH);
    }
}


/* ============================================
 * Public Function Implementations
 * ============================================ */
SevenSeg_t SEG_zInit(GPIO_Pin_t segments[7], LED_ActiveType_t active) 
{
    SevenSeg_t seg;
    
    /* Store pins and active type */
    for(u8_t i = 0; i < 7; i++) 
	{
        seg.segments[i] = segments[i];
    }
    seg.active = active;
    
    /* Initialize all 7 pins as outputs */
    for(u8_t i = 0; i < 7; i++) 
	{
        DIO_PinInit(segments[i], DIR_OUTPUT);
    }
    
    /* Start with display off */
    SEG_Clear(&seg);
    
    return seg;
}

void SEG_Init(const SevenSeg_t* seg)
{
	/* Initialize all 7 pins as outputs */
	for(u8_t i = 0; i < 7; i++)
	{
		DIO_PinInit(seg->segments[i], DIR_OUTPUT);
	}
	
	/* Start with display off */
	SEG_Clear(seg);
	
}

void SEG_RawWrite(const SevenSeg_t* seg, u8_t pattern) 
{
    /* Write each bit to its corresponding segment pin */
    for(u8_t i = 0; i < 7; i++) 
	{
        bool_t state = READ_BIT(pattern, i);
        SEG_WriteSegment(seg->segments[i], state, seg->active);
    }
}

void SEG_DisplayDigit(const SevenSeg_t* seg, u8_t digit) 
{
    if (digit > 9) 
	{
        return;  /* Invalid digit */
    }
    SEG_RawWrite(seg, SEG_Patterns[digit]);
}

void SEG_DisplayHex(const SevenSeg_t* seg, u8_t hex) 
{
    if (hex > 15) 
	{
        return;  /* Invalid hex value */
    }
    SEG_RawWrite(seg, SEG_Patterns[hex]);
}

void SEG_Clear(const SevenSeg_t* seg) 
{
    SEG_RawWrite(seg, 0x00);
}