/**
 * @file    mcal_eeprom.h
 * @brief   MCAL EEPROM Driver for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Provides low-level functions to read and write the internal EEPROM.
 *          Uses polling to wait for completion.
 *          All operations are blocking (non-interrupt driven).
 */

#ifndef MCAL_EEPROM_H_
#define MCAL_EEPROM_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"          /* Provides u8, u16, etc. */
#include "../common/std_macros.h"		  /* Provides SET_BIT, CLR_BIT, etc. */
#include "../mcal/mcal_registers.h"       /* Provides EEAR, EECR, EEDR, SREG, etc.*/

/* ============================================
 * Bit Positions – EEPROM (EECR)
 * ============================================ */
#define EECR_EERE	0		/**< EEPROM Read Enable */
#define EECR_EEWE	1		/**< EEPROM Write Enable */
#define EECR_EEMWE	2		/**< EEPROM Master Write Enable */

/* ============================================
 * Public Function Prototypes
 * ============================================ */

/**
 * @brief   Read a byte from EEPROM.
 * @param   address     EEPROM address (0 to 1023 for ATmega32).
 * @return  u16_t		Byte read from EEPROM.
 * 
 * @note    Blocks until EEPROM is ready (previous write completed).
 * @warning Does not validate address range. Ensure address < 1024.
 * 
 * @example
 *      uint8_t value = EEPROM_Read(10);  // Read byte at address 10
 */
u8_t EEPROM_Read(u16_t address);

/**
 * @brief   Write a byte to EEPROM.
 * @param   address     EEPROM address (0 to 1023 for ATmega32).
 * @param   data        Byte to write.
 * 
 * @note    Blocks until write is complete (polling).
 *          Disables interrupts during the write sequence (optional).
 * @warning Does not validate address range. Ensure address < 1024.
 * 
 * @example
 *      EEPROM_Write(10, 0x55);  // Write 0x55 to address 10
 */
void EEPROM_Write(u16_t address, u8_t data);


#endif /* MCAL_EEPROM_H_ */