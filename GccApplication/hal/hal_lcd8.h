/**
 * @file    hal_lcd8.h
 * @brief   HAL 8-Bit LCD Driver with Busy Flag Support
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Supports 8-bit mode LCDs (HD44780 compatible).
 *          Control lines: RS, RW, EN
 *          Data lines: Port
 *          Uses busy flag polling instead of fixed delays for maximum efficiency.
 *          RW pin is REQUIRED for busy flag reading.
 */

#ifndef HAL_LCD8_H_
#define HAL_LCD8_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"			/* Provides u8, u16, etc. */
#include "../common/std_macros.h"			/* Provides SET_BIT, CLR_BIT, etc. */
#include "../common/gpio_types.h"			/* Provides GPIO_Pin_t, GPIO_Direction_t, etc. */
#include "../common/ud_config.h"			/* Provides F_CPU, etc. */
#include "../mcal/mcal_dio.h"				/* Provides DIO_PinInit, DIO_PinWrite, etc. */
#include <util/delay.h>

/* ============================================
 * Special Macros
 * ============================================ */
#define LCD_WAIT_TIMEOUT  50000U   /* ? 100 ms @ 8 MHz, -Os */


/* ============================================
 * LCD Commands (Instruction Set)
 * ============================================ */

/* ===== Display & Cursor Control ===== */
#define LCD_CMD_CLEAR           0x01  /**< Clear display */
#define LCD_CMD_HOME            0x02  /**< Return home */
#define LCD_CMD_DISPLAY_OFF     0x08  /**< Display off */
#define LCD_CMD_DISPLAY_ON      0x0C  /**< Display on, cursor off */
#define LCD_CMD_CURSOR_ON       0x0E  /**< Display on, cursor on */
#define LCD_CMD_CURSOR_BLINK    0x0F  /**< Display on, cursor blinking */

/* ===== Entry Mode ===== */
#define LCD_CMD_ENTRY_MODE      0x06  /**< Increment, no shift */
#define LCD_CMD_ENTRY_ARABIC    0x04  /**< Decrement, no shift (right-to-left) */
#define LCD_CMD_ENTRY_SHIFT     0x07  /**< Increment, shift display */

/* ===== Function Set ===== */
#define LCD_CMD_8BIT_MODE       0x38  /**< 8-bit mode, 2 lines, 5x8 font */
#define LCD_CMD_4BIT_MODE       0x28  /**< 4-bit mode, 2 lines, 5x8 font */

/* ===== DDRAM Address ===== */
#define LCD_CMD_LINE1           0x80  /**< Set DDRAM address to line 1 */
#define LCD_CMD_LINE2           0xC0  /**< Set DDRAM address to line 2 */

/* ===== Cursor/Display Shift ===== */
#define LCD_CMD_SHIFT_LEFT      0x18  /**< Shift display left */
#define LCD_CMD_SHIFT_RIGHT     0x1C  /**< Shift display right */
#define LCD_CMD_CURSOR_LEFT     0x10  /**< Move cursor left */
#define LCD_CMD_CURSOR_RIGHT    0x14  /**< Move cursor right */

/* ===== DDRAM Address Ranges (16x2 LCD) ===== */
#define LCD_LINE1_START         0x00  /**< Line 1 DDRAM start address */
#define LCD_LINE1_END           0x0F  /**< Line 1 DDRAM end address (15 chars) */
#define LCD_LINE2_START         0x40  /**< Line 2 DDRAM start address */
#define LCD_LINE2_END           0x4F  /**< Line 2 DDRAM end address (15 chars) */

/* ============================================
 * LCD Object Structure
 * ============================================ */

/**
 * @brief   8-bit LCD object structure.
 * @note    Encapsulates all control and data pins for a 8-bit LCD.
 */
typedef struct {
	GPIO_Port_t  port;			/**< Port for D0:D7 (all 8 bits) */
	GPIO_Pin_t   rs;            /**< Register Select pin (0 = command, 1 = data) */
	GPIO_Pin_t   rw;            /**< Read/Write pin (0 = write, 1 = read) — required for BF */
	GPIO_Pin_t   en;            /**< Enable pin (pulse to latch data) */
} LCD_8Bit_t;


/* ============================================
 * Public Function Prototypes
 * ============================================ */

/**
 * @brief   Initialize the LCD in 8-bit mode.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * 
 * @note    Performs the required 8-bit initialization sequence.
 *          Uses busy flag polling instead of delays.
 * 
 * @example
 *      LCD_8Bit_t lcd = {
 *          .port = PORT_A,
 *          .rs = GPIO_PIN(PORT_B, PIN_0),
 *          .rw = GPIO_PIN(PORT_B, PIN_1),
 *          .en = GPIO_PIN(PORT_B, PIN_2)
 *      };
 *      LCD_Init(&lcd);
 */
void LCD_Init(const LCD_8Bit_t* lcd);

/**
 * @brief   Send a command to the LCD.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @param   cmd     Command byte (8-bit).
 * 
 * @note    Waits for busy flag before sending.
 * 
 * @example
 *      LCD_SendCommand(&lcd, LCD_CMD_CLEAR);  // Clear display
 */
void LCD_SendCommand(const LCD_8Bit_t* lcd, u8_t cmd);

/**
 * @brief   Send a character to the LCD.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @param   data    Character to display.
 * 
 * @note    Waits for busy flag before sending.
 * 
 * @example
 *      LCD_SendChar(&lcd, 'A');  // Display 'A'
 */
void LCD_SendChar(const LCD_8Bit_t* lcd, u8_t data);

/**
 * @brief   Send a string to the LCD.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @param   str     Null-terminated string to display.
 * 
 * @example
 *      LCD_SendString(&lcd, "Hello World!");
 */
void LCD_SendString(const LCD_8Bit_t* lcd, const char* str);


/* ===== Cursor & Display Control ===== */

/**
 * @brief   Move cursor to a specific position.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @param   row     Row (0 = first line, 1 = second line).
 * @param   col     Column (0-15).
 * 
 * @example
 *      LCD_GoTo(&lcd, 0, 5);  // Move cursor to line 1, column 5
 */
void LCD_GoTo(const LCD_8Bit_t* lcd, u8_t row, u8_t col);

/**
 * @brief   Move cursor left by one position.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 */
void LCD_CursorLeft(const LCD_8Bit_t* lcd);

/**
 * @brief   Move cursor right by one position.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 */
void LCD_CursorRight(const LCD_8Bit_t* lcd);

/**
 * @brief   Move cursor down to the next line (same column).
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 */
void LCD_CursorDown(const LCD_8Bit_t* lcd);

/**
 * @brief   Move cursor up to the previous line (same column).
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 */
void LCD_CursorUp(const LCD_8Bit_t* lcd);

/**
 * @brief   Shift display left by one position.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 */
void LCD_ShiftLeft(const LCD_8Bit_t* lcd);

/**
 * @brief   Shift display right by one position.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 */
void LCD_ShiftRight(const LCD_8Bit_t* lcd);


/* ===== Display Control ===== */

/**
 * @brief   Clear the LCD screen.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 */
void LCD_Clear(const LCD_8Bit_t* lcd);

/**
 * @brief   Return the cursor to home position (0,0).
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 */
void LCD_Home(const LCD_8Bit_t* lcd);

/**
 * @brief   Turn the LCD display on/off.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @param   enable  TRUE = display on, FALSE = display off.
 */
void LCD_Display(const LCD_8Bit_t* lcd, bool_t enable);

/**
 * @brief   Enable/disable the cursor.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @param   enable  TRUE = cursor on, FALSE = cursor off.
 */
void LCD_Cursor(const LCD_8Bit_t* lcd, bool_t enable);

/**
 * @brief   Enable/disable cursor blinking.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @param   enable  TRUE = blinking on, FALSE = blinking off.
 */
void LCD_Blink(const LCD_8Bit_t* lcd, bool_t enable);

#endif /* HAL_LCD_H_ */