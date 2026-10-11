/**
 * @file    hal_sevenseg.h
 * @brief   HAL 7-Segment Display Driver
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Supports common cathode and common anode displays.
 *          Uses 7 individual GPIO pins for segments A-G.
 */

#ifndef HAL_SEVENSEG_H_
#define HAL_SEVENSEG_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"		/* Provides u8, u16, etc. */
#include "../common/std_macros.h"		/* Provides SET_BIT, CLR_BIT, etc. */
#include "../common/gpio_types.h"		/* Provides GPIO_Pin_t, GPIO_Direction_t, etc. */
#include "../hal/hal_common.h"			/* Provides LED_ActiveType_t, Button_ActiveType_t, etc. */
#include "../mcal/mcal_dio.h"			/* Provides DIO_PinInit, DIO_PinWrite, etc. */

/* ============================================
 * 7-Segment Display Object
 * ============================================ */

/**
 * @brief   7-segment display object structure.
 * @note    Encapsulates all 7 segment pins and active type.
 */
typedef struct {
	GPIO_Pin_t segments[7];     /**< Pins for segments A-G (index 0-6) */
	LED_ActiveType_t active;    /**< ACTIVE_HIGH (common cathode) or ACTIVE_LOW (common anode) */
} SevenSeg_t;


/* ============================================
 * Public Function Prototypes
 * ============================================ */

/**
 * @brief   Initialize a 7-segment display with 7 individual pins.
 * @param   segments   Array of 7 GPIO_Pin_t (A, B, C, D, E, F, G)
 * @param   active     LED_ACTIVE_HIGH (common cathode) or LED_ACTIVE_LOW (common anode)
 * @return  SevenSeg_t Configured display object.
 * 
 * @example
 *      GPIO_Pin_t pins[7] = {
 *          GPIO_PIN(PORT_A, PIN_0),  // A
 *          GPIO_PIN(PORT_A, PIN_1),  // B
 *          GPIO_PIN(PORT_A, PIN_2),  // C
 *          GPIO_PIN(PORT_A, PIN_3),  // D
 *          GPIO_PIN(PORT_A, PIN_4),  // E
 *          GPIO_PIN(PORT_A, PIN_5),  // F
 *          GPIO_PIN(PORT_A, PIN_6)   // G
 *      };
 *      SevenSeg_t display = SEG_Init(pins, LED_ACTIVE_HIGH);
 */
SevenSeg_t SEG_zInit(GPIO_Pin_t segments[7], LED_ActiveType_t active);

/**
 * @brief   Initialize a 7-segment display with 7 individual pins.
 * @param   seg     Pointer to the SevenSeg_t object.
 */
void SEG_Init(const SevenSeg_t* seg);

/**
 * @brief   Display a digit (0-9) on the 7-segment display.
 * @param   seg     Pointer to the SevenSeg_t object.
 * @param   digit   Digit to display (0-9).
 * 
 * @example
 *      SEG_DisplayDigit(&display, 5);  // Shows "5"
 */
void SEG_DisplayDigit(const SevenSeg_t* seg, u8_t digit);

/**
 * @brief   Display a hexadecimal digit (0-F) on the 7-segment display.
 * @param   seg     Pointer to the SevenSeg_t object.
 * @param   hex     Hexadecimal value (0-15).
 * 
 * @example
 *      SEG_DisplayHex(&display, 0xA);  // Shows "A"
 */
void SEG_DisplayHex(const SevenSeg_t* seg, u8_t hex);

/**
 * @brief   Clear the 7-segment display (turn all segments off).
 * @param   seg     Pointer to the SevenSeg_t object.
 */
void SEG_Clear(const SevenSeg_t* seg);

/**
 * @brief   Write a raw segment pattern to the display.
 * @param   seg         Pointer to the SevenSeg_t object.
 * @param   pattern     Bitmask: bit0=A, bit1=B, ..., bit6=G
 * 
 * @example
 *      SEG_RawWrite(&display, 0b01111111);  // Shows "8"
 */
void SEG_RawWrite(const SevenSeg_t* seg, u8_t pattern);


#endif /* HAL_SEVENSEG_H_ */






