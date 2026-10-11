/**
 * @file    hal_keypad.c
 * @brief   HAL 4x4 Matrix Keypad Driver Implementation
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Implements scanning for a 4x4 matrix keypad.
 *          Rows: pins 0-3 (outputs)
 *          Columns: pins 4-7 (inputs with external pull-downs)
 */

#include "hal_keypad.h"
/* ============================================
 * Private Constants
 * ============================================ */

/**
 * @brief   Keypad character map for key codes 0-15.
 * @note    Layout matches typical 4x4 keypad:
 *          1  2  3  A
 *          4  5  6  B
 *          7  8  9  C
 *          *  0  #  D
 */
static const char KEYPAD_CharMap[16] = {
	'1', '2', '3', 'A',
	'4', '5', '6', 'B',
	'7', '8', '9', 'C',
	'*', '0', '#', 'D'
};


/* ============================================
 * Private Helper Functions
 * ============================================ */

/**
 * @brief   Scan the keypad and return a bitmask of all pressed keys.
 * @param   port    GPIO port with keypad pins.
 * @return  u16_t   Bitmask: bit N = 1 if key N is pressed.
 *                  Returns KEYPAD_NO_KEY (0x0000) if no keys pressed.
 * 
 * @note    Active-high logic (columns with external pull-downs).
 *          Rows are driven High one at a time, columns are read.
 */
static u16_t KEYPAD_Scan(GPIO_Port_t port) {
	u16_t key_mask = KEYPAD_NO_KEY;
	
	/* ===== Drive ALL rows LOW ===== */
	for (u8_t row = 0; row < 4; row++) {
		DIO_PinWrite(GPIO_PIN(port, row), LEVEL_LOW);
	}
	/* Allow signals to settle */
	_delay_us(50);
	
	/* ===== Scan each row ===== */
	for (u8_t row = 0; row < 4; row++) {
		/* Drive current row HIGH */
		DIO_PinWrite(GPIO_PIN(port, row), LEVEL_HIGH);
		/* Allow signal to settle */
		_delay_us(10);
		
		/* Read columns */
		u8_t col_mask = DIO_PortRead(port) & 0xF0;
		key_mask |= ((u16_t)(col_mask >> 4)) << (row * 4);
		
		/* Drive current row LOW */
		DIO_PinWrite(GPIO_PIN(port, row), LEVEL_LOW);
		/* Allow signal to settle */
		_delay_us(10);
	}
	
	/* ===== Restore all rows HIGH ===== */
	for (u8_t row = 0; row < 4; row++) {
		DIO_PinWrite(GPIO_PIN(port, row), LEVEL_HIGH);
	}
	
	/* Allow signals to settle */
	_delay_ms(1);
	
	return key_mask;
}


/* ============================================
 * Public Function Implementations
 * ============================================ */
Keypad_t KEYPAD_zInit(GPIO_Port_t port) {
	Keypad_t keypad;
	
	/* Store port and initialize state */
	keypad.port = port;
	keypad.last_mask = KEYPAD_NO_KEY;
	
	/* ===== Configure Rows (pins 0-3) as Outputs ===== */
	for (u8_t i = 0; i < 4; i++) {
		DIO_PinInit(GPIO_PIN(port, i), DIR_OUTPUT);
		DIO_PinWrite(GPIO_PIN(port, i), LEVEL_HIGH);  /* Idle HIGH */
	}
	
	/* ===== Configure Columns (pins 4-7) as Inputs without Pull-ups ===== */
	for (u8_t i = 4; i < 8; i++) {
		DIO_PinInit(GPIO_PIN(port, i), DIR_INPUT);
		DIO_PinWrite(GPIO_PIN(port, i), LEVEL_LOW);  /* Disable internal pull-up! */
	}
	
	return keypad;
}


void KEYPAD_Init(const Keypad_t* keypad) {
	/* ===== Configure Rows (pins 0-3) as Outputs ===== */
	for (u8_t i = 0; i < 4; i++) {
		DIO_PinInit(GPIO_PIN(keypad->port, i), DIR_OUTPUT);
		DIO_PinWrite(GPIO_PIN(keypad->port, i), LEVEL_HIGH);  /* Idle HIGH */
	}
	
	/* ===== Configure Columns (pins 4-7) as Inputs without Pull-ups ===== */
	for (u8_t i = 4; i < 8; i++) {
		DIO_PinInit(GPIO_PIN(keypad->port, i), DIR_INPUT);
		DIO_PinWrite(GPIO_PIN(keypad->port, i), LEVEL_LOW);  /* Disable internal pull-up! */
	}
	
}

u16_t KEYPAD_ReadMask(const Keypad_t* keypad) {
	u16_t key_mask = KEYPAD_Scan(keypad->port);
	
	/* Update last_mask */
	((Keypad_t*)keypad)->last_mask = key_mask;
	
	return key_mask;
}

char KEYPAD_ReadChar(const Keypad_t* keypad) {
	u16_t key_mask = KEYPAD_ReadMask(keypad);
	return KEYPAD_MaskToChar(key_mask);
}

char KEYPAD_JustPressed(const Keypad_t* keypad) {
	u16_t current_mask = KEYPAD_Scan(keypad->port);
	u16_t relative_mask = current_mask & ~(keypad->last_mask);
	
	/* Update last_mask */
	((Keypad_t*)keypad)->last_mask = current_mask;
	
	return KEYPAD_MaskToChar(relative_mask);
}

char KEYPAD_JustReleased(const Keypad_t* keypad) {
	u16_t current_mask = KEYPAD_Scan(keypad->port);
	u16_t relative_mask = keypad->last_mask & ~current_mask;
	
	/* Update last_mask */
	((Keypad_t*)keypad)->last_mask = current_mask;
	
	return KEYPAD_MaskToChar(relative_mask);
}

char KEYPAD_MaskToChar(u16_t key_mask) {
	if (key_mask == KEYPAD_NO_KEY) {
		return '\0';
	}
	
	/* Return the first key found (lowest bit) */
	for (u8_t i = 0; i < 16; i++) {
		if (READ_BIT(key_mask, i)) {
			return KEYPAD_CharMap[i];
		}
	}
	
	return '\0';
}