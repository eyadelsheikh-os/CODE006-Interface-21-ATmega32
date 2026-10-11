/**
 * @file    hal_sevenseg_bcd.h
 * @brief   HAL 7-Segment Display Driver (BCD Decoder Version)
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Uses BCD decoder (e.g., 74LS47, 74HC47).
 *          Only 4 MCU pins needed for BCD data.
 *          Active type handles common cathode vs common anode.
 *          No common pin needed - it's hardwired to GND/VCC.
 */

#ifndef HAL_SEVENSEG_BCD_H_
#define HAL_SEVENSEG_BCD_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"		/* Provides u8, u16, etc. */
#include "../common/std_macros.h"		/* Provides SET_BIT, CLR_BIT, etc. */
#include "../common/gpio_types.h"		/* Provides GPIO_Pin_t, GPIO_Direction_t, etc. */
#include "../hal/hal_common.h"			/* Provides LED_ActiveType_t, Button_ActiveType_t, etc. */
#include "../mcal/mcal_dio.h"			/* Provides DIO_PinInit, DIO_PinWrite, etc. */

/* ============================================
 * 7-Segment BCD Display Object
 * ============================================ */

/**
 * @brief   7-segment display object with BCD decoder.
 * @note    Encapsulates 4 BCD data pins + active type.
 *          Common pin is NOT needed (hardwired in hardware).
 */
typedef struct {
	GPIO_Pin_t data[4];      /**< BCD data pins (D0=LSB, D3=MSB) */
	LED_ActiveType_t active; /**< ACTIVE_HIGH (common cathode) or ACTIVE_LOW (common anode) */
} SevenSeg_BCD_t;


/* ============================================
 * Public Function Prototypes
 * ============================================ */

/**
 * @brief   Initialize a 7-segment display with BCD decoder.
 * @param   data_pins   Array of 4 GPIO pins (D0=LSB, D3=MSB).
 * @param   active      LED_ACTIVE_HIGH (common cathode) or LED_ACTIVE_LOW (common anode).
 * @return  SevenSeg_BCD_t Configured display object.
 * 
 * @example
 *      GPIO_Pin_t pins[4] = {
 *          GPIO_PIN(PORT_A, PIN_0),  // D0 (LSB)
 *          GPIO_PIN(PORT_A, PIN_1),  // D1
 *          GPIO_PIN(PORT_A, PIN_2),  // D2
 *          GPIO_PIN(PORT_A, PIN_3)   // D3 (MSB)
 *      };
 *      SevenSeg_BCD_t display = SEG_BCD_Init(pins, LED_ACTIVE_HIGH);
 *      SEG_BCD_DisplayDigit(&display, 5);  // Shows "5"
 */
SevenSeg_BCD_t SEG_BCD_zInit(GPIO_Pin_t data_pins[4], LED_ActiveType_t active);

/**
 * @brief   Initialize a 7-segment display with BCD decoder.
 * @param   seg     Pointer to the SevenSeg_BCD_t object.
 */
void SEG_BCD_Init(const SevenSeg_BCD_t* seg);

/**
 * @brief   Display a digit (0-9) on the 7-segment display.
 * @param   seg     Pointer to the SevenSeg_BCD_t object.
 * @param   digit   Digit to display (0-9).
 * 
 * @note    Values 0-9 displayed correctly.
 *          Values 10-15 show blank (74LS47 invalid pattern).
 */
void SEG_BCD_DisplayDigit(const SevenSeg_BCD_t* seg, u8_t digit);

/**
 * @brief   Display a hexadecimal digit (0-F).
 * @param   seg     Pointer to the SevenSeg_BCD_t object.
 * @param   hex     Hex value (0-15).
 * 
 * @note    For 74LS47: 0-9 = digits, 10-15 = blank
 *          For 74LS48: 0-9 = digits, A-F = hex letters
 */
void SEG_BCD_DisplayHex(const SevenSeg_BCD_t* seg, u8_t hex);

/**
 * @brief   Clear the 7-segment display (turn all segments off).
 * @param   seg     Pointer to the SevenSeg_BCD_t object.
 * 
 * @note    For 74LS47: sends 0x0F (invalid BCD = blank)
 *          For active type: handles polarity automatically.
 */
void SEG_BCD_Clear(const SevenSeg_BCD_t* seg);


#endif /* HAL_SEVENSEG_BCD_H_ */
