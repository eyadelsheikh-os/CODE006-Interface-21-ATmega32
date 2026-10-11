/**
 * @file    hal_lcd8.c
 * @brief   HAL 8-Bit LCD Driver Implementation
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Implements 8-bit LCD control with busy flag polling.
 *          Uses RW pin for reading the busy flag (no delays!).
 *          Supports HD44780 compatible LCDs.
 *
 * @warning	This driver SHALLNOT be included with the 4-bit LCD driver.
 */

#include "hal_lcd8.h"
/* ============================================
 * Private Helper Prototypes
 * ============================================ */

/**
 * @brief   Initialize the LCD hardware pins (control + data port).
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * 
 * @note    Configures all LCD pins as outputs, except during busy flag reads.
 */
static void LCD_PinInit(const LCD_8Bit_t* lcd);

/**
 * @brief   Write a full byte to the data port.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @param   data    8?bit value to write.
 */
static void LCD_WriteByte(const LCD_8Bit_t* lcd, u8_t data);

/**
 * @brief   Read a full byte from the data port.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @return  u8_t    Value read from data port.
 */
static u8_t LCD_ReadByte(const LCD_8Bit_t* lcd);

/**
 * @brief   Send a byte (command or data) to the LCD.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @param   data    Byte to send.
 * @param   is_cmd  TRUE = command, FALSE = data (controls RS).
 */
static void LCD_SendByte(const LCD_8Bit_t* lcd, u8_t data, bool_t is_cmd);


/* Busy flag operations */

/**
 * @brief   Read the busy flag and cursor address.
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @return  u8_t    BF (bit 7) + AC (bits 0-6)
 * 
 * @note    BF = 1 means LCD is busy (not ready for new commands).
 *          AC = Current DDRAM/CGRAM address.
 */
static u8_t LCD_ReadBusyFlag(const LCD_8Bit_t* lcd);

/**
 * @brief   Wait for the LCD to become ready (busy flag = 0).
 * @param   lcd     Pointer to const LCD_8Bit_t object.
 * @return  to_flag Timeout Flag (FALSE = Timeout, TRUE = Successful Operation).
 *
 * @note    Polls the busy flag until it clears.
 *          Replaces fixed delays for maximum efficiency.
 */
static bool_t LCD_WaitReady(const LCD_8Bit_t* lcd);


/* ============================================
 * Private Helper Functions
 * ============================================ */
static void LCD_PinInit(const LCD_8Bit_t* lcd) {
    /* Control pins as outputs */
    DIO_PinInit(lcd->rs, DIR_OUTPUT);
    DIO_PinInit(lcd->rw, DIR_OUTPUT);
    DIO_PinInit(lcd->en, DIR_OUTPUT);
    
    /* Data port as output (will be switched to input during read) */
    DIO_PortInit(lcd->port, DIR_OUTPUT);
    
    /* Initial states */
    DIO_PinWrite(lcd->rs, LEVEL_LOW);
    DIO_PinWrite(lcd->rw, LEVEL_LOW);
    DIO_PinWrite(lcd->en, LEVEL_LOW);
    DIO_PortWrite(lcd->port, 0x00);
}

static void LCD_WriteByte(const LCD_8Bit_t* lcd, u8_t data) {
	/* Set data port to output (they may be inputs after a read) */
	DIO_PortInit(lcd->port, DIR_OUTPUT);
	DIO_PortWrite(lcd->port, data);

	/* Pulse EN to latch data */
	DIO_PinWrite(lcd->en, LEVEL_HIGH);
	_delay_us(1);	
	DIO_PinWrite(lcd->en, LEVEL_LOW);
	_delay_us(1);
}

static u8_t LCD_ReadByte(const LCD_8Bit_t* lcd) {
	u8_t data;

	/* Set data port as input */
	DIO_PortInit(lcd->port, DIR_INPUT);

	/* Pulse EN to read */
	DIO_PinWrite(lcd->en, LEVEL_HIGH);
	_delay_us(5);
	
	data = DIO_PortRead(lcd->port);
	
	DIO_PinWrite(lcd->en, LEVEL_LOW);
	_delay_us(5);

	return data;
}

static void LCD_SendByte(const LCD_8Bit_t* lcd, u8_t data, bool_t is_cmd) {
    /* Set RS = LOW/HIGH (command/data), RW = LOW (write) */
    DIO_PinWrite(lcd->rs, is_cmd ? LEVEL_LOW : LEVEL_HIGH);
	DIO_PinWrite(lcd->rw, LEVEL_LOW);      /* Write mode */
	LCD_WriteByte(lcd, data);
}

static u8_t LCD_ReadBusyFlag(const LCD_8Bit_t* lcd) {
    u8_t status;

    /* Set RS = LOW (command), RW = HIGH (read) */
    DIO_PinWrite(lcd->rs, LEVEL_LOW);
    DIO_PinWrite(lcd->rw, LEVEL_HIGH);

    _delay_us(10);	/* Simple delay for pins to settle */
    status = LCD_ReadByte(lcd);	/* Received data */

    DIO_PinWrite(lcd->rw, LEVEL_LOW);	/* Back to write mode */

    return status;
}

static bool_t LCD_WaitReady(const LCD_8Bit_t* lcd) {
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
void LCD_Init(const LCD_8Bit_t* lcd) {
	/* Hardware initialization */
	LCD_PinInit(lcd);
	_delay_ms(20);	/* Power up wait >15ms */

	/* Software initialization */
	LCD_SendCommand(lcd, LCD_CMD_8BIT_MODE);   /* 8-bit, 2 lines, 5x8 font */
	LCD_SendCommand(lcd, LCD_CMD_DISPLAY_ON);  /* Display ON, cursor off */
	LCD_SendCommand(lcd, LCD_CMD_ENTRY_MODE);  /* Increment, no shift */
	LCD_Clear(lcd);
}

void LCD_SendCommand(const LCD_8Bit_t* lcd, u8_t cmd) {
	LCD_WaitReady(lcd);
	LCD_SendByte(lcd, cmd, TRUE);	/* is_cmd = TRUE */
}

void LCD_SendChar(const LCD_8Bit_t* lcd, u8_t data) {
	LCD_WaitReady(lcd);
	LCD_SendByte(lcd, data, FALSE);	/* is_cmd = FALSE */
}

void LCD_SendString(const LCD_8Bit_t* lcd, const char* str) {
	while (*str) {
		LCD_SendChar(lcd, (u8_t)*str++);
	}
}

/* ===== Cursor & Display Control ===== */
void LCD_GoTo(const LCD_8Bit_t* lcd, u8_t row, u8_t col) {
	u8_t address = (row == 0) ? LCD_LINE1_START : LCD_LINE2_START;
	LCD_SendCommand(lcd, 0x80 | (address + col));
}

void LCD_CursorLeft(const LCD_8Bit_t* lcd)	{ LCD_SendCommand(lcd, LCD_CMD_CURSOR_LEFT); }
void LCD_CursorRight(const LCD_8Bit_t* lcd)	{ LCD_SendCommand(lcd, LCD_CMD_CURSOR_RIGHT); }

void LCD_ShiftLeft(const LCD_8Bit_t* lcd)	{ LCD_SendCommand(lcd, LCD_CMD_SHIFT_LEFT); }
void LCD_ShiftRight(const LCD_8Bit_t* lcd)	{ LCD_SendCommand(lcd, LCD_CMD_SHIFT_RIGHT); }

/* ===== Display Control ===== */
void LCD_Clear(const LCD_8Bit_t* lcd) {
	LCD_SendCommand(lcd, LCD_CMD_CLEAR);
	LCD_WaitReady(lcd);	/* Clear takes ~1.5ms */
}

void LCD_Home(const LCD_8Bit_t* lcd) {
    LCD_SendCommand(lcd, LCD_CMD_HOME);
    LCD_WaitReady(lcd);	/* Home takes ~1.5ms */
}

void LCD_Display(const LCD_8Bit_t* lcd, bool_t enable) {
	LCD_SendCommand(lcd, enable ? LCD_CMD_DISPLAY_ON : LCD_CMD_DISPLAY_OFF);
}

void LCD_Cursor(const LCD_8Bit_t* lcd, bool_t enable) {
	LCD_SendCommand(lcd, enable ? LCD_CMD_CURSOR_ON : LCD_CMD_DISPLAY_ON);
}

void LCD_Blink(const LCD_8Bit_t* lcd, bool_t enable) {
	LCD_SendCommand(lcd, enable ? LCD_CMD_CURSOR_BLINK : LCD_CMD_DISPLAY_ON);
}
