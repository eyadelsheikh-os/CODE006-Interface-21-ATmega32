/**
 * @file    hal_lcd4.c
 * @brief   HAL 4-Bit LCD Driver Implementation
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Implements 4-bit LCD control with busy flag polling.
 *          Data is sent as two nibbles: high nibble first, then low nibble.
 *          Busy flag read fetches two nibbles (high contains BF).
 *
 * @warning	This driver SHALLNOT be included with the 8-bit LCD driver.
 */

#include "hal_lcd4.h"
/* ============================================
 * Private Helper Prototypes
 * ============================================ */

/**
 * @brief   Initialize the LCD hardware pins (control + data pins).
 * @param   lcd     Pointer to const LCD_4Bit_t object.
 * 
 * @note    Configures all LCD pins as outputs, except during busy flag reads.
 */
static void LCD_PinInit(const LCD_4Bit_t* lcd);

/**
 * @brief   Write a 4-bit nibble to the data pins and pulse EN.
 * @param   lcd     Pointer to const LCD_4Bit_t object.
 * @param   nibble  4-bit value (bit0 = D4, bit3 = D7).
 */
static void LCD_WriteNibble(const LCD_4Bit_t* lcd, u8_t nibble);


/**
 * @brief   Read a 4-bit nibble from the data pins.
 * @param   lcd     Pointer to const LCD_4Bit_t object.
 * @return  4-bit value (bit0 = D4, bit3 = D7).
 */
static u8_t LCD_ReadNibble(const LCD_4Bit_t* lcd);

/**
 * @brief   Send a full byte as two nibbles (high first, then low).
 * @param   lcd     Pointer to const LCD_4Bit_t object.
 * @param   data    Byte to send.
 * @param   is_cmd  TRUE = command, FALSE = data (controls RS).
 */
static void LCD_SendByte(const LCD_4Bit_t* lcd, u8_t data, bool_t is_cmd);


/* Busy flag operations */

/**
 * @brief   Read the busy flag + address counter (returns full 8-bit status).
 * @return  Status byte: bit7 = BF, bits 6:0 = AC (partial).
 *
 * @note    In 4-bit mode, the status is read as two nibbles:
 *          high nibble carries BF + AC6:4, low nibble carries AC3:0.
 */
static u8_t LCD_ReadBusyFlag(const LCD_4Bit_t* lcd);

/**
 * @brief   Wait for the LCD to become ready (busy flag = 0).
 * @param   lcd     Pointer to const LCD_4Bit_t object.
 * @return  to_flag Timeout Flag (FALSE = Timeout, TRUE = Successful Operation).
 *
 * @note    Polls the busy flag until it clears.
 *          Replaces fixed delays for maximum efficiency.
 */
static bool_t LCD_WaitReady(const LCD_4Bit_t* lcd);


/* ============================================
 * Private Helpers Functions
 * ============================================ */
static void LCD_PinInit(const LCD_4Bit_t* lcd) {
    /* Control pins as outputs */
    DIO_PinInit(lcd->rs, DIR_OUTPUT);
    DIO_PinInit(lcd->rw, DIR_OUTPUT);
    DIO_PinInit(lcd->en, DIR_OUTPUT);

    /* Data pins as outputs (will be switched to input during reads) */
    DIO_PinInit(lcd->d4, DIR_OUTPUT);
    DIO_PinInit(lcd->d5, DIR_OUTPUT);
    DIO_PinInit(lcd->d6, DIR_OUTPUT);
    DIO_PinInit(lcd->d7, DIR_OUTPUT);

    /* Initial states */
    DIO_PinWrite(lcd->rs, LEVEL_LOW);
    DIO_PinWrite(lcd->rw, LEVEL_LOW);
    DIO_PinWrite(lcd->en, LEVEL_LOW);
    DIO_PinWrite(lcd->d4, LEVEL_LOW);
    DIO_PinWrite(lcd->d5, LEVEL_LOW);
    DIO_PinWrite(lcd->d6, LEVEL_LOW);
    DIO_PinWrite(lcd->d7, LEVEL_LOW);
}

static void LCD_WriteNibble(const LCD_4Bit_t* lcd, u8_t nibble) {
    /* Set data pins as outputs (they may be inputs after a read) */
    DIO_PinInit(lcd->d4, DIR_OUTPUT);
    DIO_PinInit(lcd->d5, DIR_OUTPUT);
    DIO_PinInit(lcd->d6, DIR_OUTPUT);
    DIO_PinInit(lcd->d7, DIR_OUTPUT);

    DIO_PinWrite(lcd->d4, (nibble & 0x01) ? LEVEL_HIGH : LEVEL_LOW);
    DIO_PinWrite(lcd->d5, (nibble & 0x02) ? LEVEL_HIGH : LEVEL_LOW);
    DIO_PinWrite(lcd->d6, (nibble & 0x04) ? LEVEL_HIGH : LEVEL_LOW);
    DIO_PinWrite(lcd->d7, (nibble & 0x08) ? LEVEL_HIGH : LEVEL_LOW);

    /* Pulse EN to latch */
    DIO_PinWrite(lcd->en, LEVEL_HIGH);
    _delay_us(1);
    DIO_PinWrite(lcd->en, LEVEL_LOW);
    _delay_us(1);
}

static u8_t LCD_ReadNibble(const LCD_4Bit_t* lcd) {
    u8_t nibble = 0;

    /* Data pins must be inputs */
    DIO_PinInit(lcd->d4, DIR_INPUT);
    DIO_PinInit(lcd->d5, DIR_INPUT);
    DIO_PinInit(lcd->d6, DIR_INPUT);
    DIO_PinInit(lcd->d7, DIR_INPUT);

	/* Pulse EN to read */
	DIO_PinWrite(lcd->en, LEVEL_HIGH);
    _delay_us(5);

    if (DIO_PinRead(lcd->d4) == LEVEL_HIGH) nibble |= 0x01;
    if (DIO_PinRead(lcd->d5) == LEVEL_HIGH) nibble |= 0x02;
    if (DIO_PinRead(lcd->d6) == LEVEL_HIGH) nibble |= 0x04;
    if (DIO_PinRead(lcd->d7) == LEVEL_HIGH) nibble |= 0x08;

    DIO_PinWrite(lcd->en, LEVEL_LOW);
    _delay_us(5);

    return nibble;
}

static void LCD_SendByte(const LCD_4Bit_t* lcd, u8_t data, bool_t is_cmd) {
    DIO_PinWrite(lcd->rs, is_cmd ? LEVEL_LOW : LEVEL_HIGH);
    DIO_PinWrite(lcd->rw, LEVEL_LOW);

    LCD_WriteNibble(lcd, HIGH_NIBBLE(data));	/* high nibble */
    LCD_WriteNibble(lcd, LOW_NIBBLE(data));		/* low nibble  */
}

/* Busy flag operations */
static u8_t LCD_ReadBusyFlag(const LCD_4Bit_t* lcd) {
    u8_t high_nibble, low_nibble;

    DIO_PinWrite(lcd->rs, LEVEL_LOW);   /* command mode */
    DIO_PinWrite(lcd->rw, LEVEL_HIGH);  /* read mode */

    _delay_us(10);

    high_nibble = LCD_ReadNibble(lcd);  /* contains BF */
    low_nibble  = LCD_ReadNibble(lcd);  /* contains AC low bits */

    DIO_PinWrite(lcd->rw, LEVEL_LOW);   /* back to write mode */

    return (u8_t)((high_nibble << 4) | (low_nibble & 0x0F));
}

static bool_t LCD_WaitReady(const LCD_4Bit_t* lcd) {
    u8_t status;
    u16_t guard = 0;
	
	/* Keep reading until BF = 0 */
	do {
        status = LCD_ReadBusyFlag(lcd);
		if (++guard >= LCD_WAIT_TIMEOUT) return FALSE;
    } while (status & 0x80);  /* Check bit 7 (BF) */
	return TRUE;
}

/* ============================================
 * Public Function Implementations
 * ============================================ */
void LCD_Init(const LCD_4Bit_t* lcd) {
	/* Hardware initialization */
	LCD_PinInit(lcd);
    _delay_ms(20);	/* power-up wait >15 ms */

    /* --- 4-bit wake-up sequence --- */
    /* Step 1: send 0x33 (nibble 0x3 twice) */
    DIO_PinWrite(lcd->rs, LEVEL_LOW);
    DIO_PinWrite(lcd->rw, LEVEL_LOW);
    LCD_WriteNibble(lcd, 0x03);
    _delay_ms(5);
    LCD_WriteNibble(lcd, 0x03);
    _delay_ms(1);

    /* Step 2: send 0x32 (switch to 4-bit) */
    LCD_WriteNibble(lcd, 0x02);
    _delay_ms(1);

	/* Software initialization */
    LCD_SendCommand(lcd, LCD_CMD_4BIT_MODE);	/* 4-bit, 2 lines, 5x8 font */
    LCD_SendCommand(lcd, LCD_CMD_DISPLAY_ON);	/* Display ON, cursor off */
    LCD_SendCommand(lcd, LCD_CMD_ENTRY_MODE);	/* Increment, no shift */
    LCD_Clear(lcd);
}

void LCD_SendCommand(const LCD_4Bit_t* lcd, u8_t cmd) {
    LCD_WaitReady(lcd);
    LCD_SendByte(lcd, cmd, TRUE);	/* is_cmd = TRUE */
}

void LCD_SendChar(const LCD_4Bit_t* lcd, u8_t data) {
    LCD_WaitReady(lcd);
    LCD_SendByte(lcd, data, FALSE);	/* is_cmd = FALSE */
}

void LCD_SendString(const LCD_4Bit_t* lcd, const char* str) {
    while (*str) {
        LCD_SendChar(lcd, (u8_t)*str++);
    }
}

/* ===== Cursor & Display Control ===== */
void LCD_GoTo(const LCD_4Bit_t* lcd, u8_t row, u8_t col) {
    u8_t address = (row == 0) ? LCD_LINE1_START : LCD_LINE2_START;
    LCD_SendCommand(lcd, 0x80 | (address + col));
}

void LCD_CursorLeft(const LCD_4Bit_t* lcd)  { LCD_SendCommand(lcd, LCD_CMD_CURSOR_LEFT);  }
void LCD_CursorRight(const LCD_4Bit_t* lcd) { LCD_SendCommand(lcd, LCD_CMD_CURSOR_RIGHT); }

void LCD_ShiftLeft(const LCD_4Bit_t* lcd)   { LCD_SendCommand(lcd, LCD_CMD_SHIFT_LEFT);   }
void LCD_ShiftRight(const LCD_4Bit_t* lcd)  { LCD_SendCommand(lcd, LCD_CMD_SHIFT_RIGHT);  }

/* ===== Display Control ===== */
void LCD_Clear(const LCD_4Bit_t* lcd) {
    LCD_SendCommand(lcd, LCD_CMD_CLEAR);
    LCD_WaitReady(lcd);	/* Home takes ~1.5ms */
}

void LCD_Home(const LCD_4Bit_t* lcd) {
    LCD_SendCommand(lcd, LCD_CMD_HOME);
    LCD_WaitReady(lcd);	/* Home takes ~1.5ms */
}

void LCD_Display(const LCD_4Bit_t* lcd, bool_t enable) {
    LCD_SendCommand(lcd, enable ? LCD_CMD_DISPLAY_ON : LCD_CMD_DISPLAY_OFF);
}

void LCD_Cursor(const LCD_4Bit_t* lcd, bool_t enable) {
    LCD_SendCommand(lcd, enable ? LCD_CMD_CURSOR_ON : LCD_CMD_DISPLAY_ON);
}

void LCD_Blink(const LCD_4Bit_t* lcd, bool_t enable) {
    LCD_SendCommand(lcd, enable ? LCD_CMD_CURSOR_BLINK : LCD_CMD_DISPLAY_ON);
}