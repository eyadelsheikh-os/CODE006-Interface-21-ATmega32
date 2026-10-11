/**
 * @file    hal_keypad.h
 * @brief   HAL 4x4 Matrix Keypad Driver
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Supports multi-key detection with bitmask.
 *          Rows: pins 0-3 (outputs)
 *          Columns: pins 4-7 (inputs with external pull-downs)
 */


#ifndef HAL_KEYPAD_H_
#define HAL_KEYPAD_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"		/* Provides u8, u16, etc. */
#include "../common/std_macros.h"		/* Provides SET_BIT, CLR_BIT, etc. */
#include "../common/gpio_types.h"		/* Provides GPIO_Pin_t, GPIO_Direction_t, etc. */
#include "../common/ud_config.h"		/* Provides F_CPU, etc. */
#include "../mcal/mcal_dio.h"			/* Provides DIO_PinInit, DIO_PinWrite, etc. */
#include <util/delay.h>

/* ============================================
 * Configuration Constants
 * ============================================ */
#define KEYPAD_NO_KEY        0x0000		/**< Return value when no key pressed */

/* ============================================
 * Keypad Object Structure
 * ============================================ */

/**
 * @brief   Keypad object structure.
 * @note    Encapsulates the port and current state.
 */
typedef struct {
	GPIO_Port_t port;			/**< Port for all keypad pins (rows 0-3, cols 4-7) */
	u16_t last_mask;			/**< Previous key mask (for edge detection) */
} Keypad_t;


/* ============================================
 * Public Function Prototypes
 * ============================================ */

/**
 * @brief   Initialize a 4x4 matrix keypad.
 * @param   port    GPIO port (rows: pins 0-3, columns: pins 4-7)
 * @return  Keypad_t  Configured keypad object.
 * 
 * @example
 *      Keypad_t keypad = KEYPAD_Init(PORT_C);
 */
Keypad_t KEYPAD_zInit(GPIO_Port_t port);


/**
 * @brief   Initialize a 4x4 matrix keypad.
 * @param   keypad  Pointer to Keypad_t object.
 * @return  Keypad_t  Configured keypad object.
 */
void KEYPAD_Init(const Keypad_t* keypad);

/**
 * @brief   Read the current keypad state as a bitmask.
 * @param   keypad  Pointer to Keypad_t object.
 * @return  u16_t   Bitmask: bit N = 1 if key N is pressed (N = row*4 + col).
 *                  Returns KEYPAD_NO_KEY (0x0000) if no keys pressed.
 * 
 * @example
 *      u16_t mask = KEYPAD_Read(&keypad);
 *      if (mask & (1 << 0)) { // Key 0 pressed  }
 */
u16_t KEYPAD_ReadMask(const Keypad_t* keypad);

/**
 * @brief   Read the first pressed key as a character.
 * @param   keypad  Pointer to Keypad_t object.
 * @return  char    Character of the first pressed key, or '\0' if none.
 * 
 * @example
 *      char key = KEYPAD_ReadChar(&keypad);
 *      if (key != '\0') { // Key pressed  }
 */
char KEYPAD_ReadChar(const Keypad_t* keypad);

/**
 * @brief   Check if a key was just pressed (edge detection).
 * @param   keypad  Pointer to Keypad_t object.
 * @return  char    A character corresponding to a key that's
 *					just pressed, or '\0' if none.
 */
char KEYPAD_JustPressed(const Keypad_t* keypad);

/**
 * @brief   Check if a key was just released (edge detection).
 * @param   keypad  Pointer to Keypad_t object.
 * @return  char    A character corresponding to a key that's 
 *					just released, or '\0' if none.
 */
char KEYPAD_JustReleased(const Keypad_t* keypad);

/**
 * @brief   Get the character for a key mask.
 * @param   key_mask    Bitmask of pressed keys.
 * @return  char        First character in the mask, or '\0' if none.
 */
char KEYPAD_MaskToChar(u16_t key_mask);


#endif /* HAL_KEYPAD_H_ */





