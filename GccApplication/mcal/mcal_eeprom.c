/**
 * @file    mcal_eeprom.c
 * @brief   MCAL EEPROM Driver Implementation
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Implements read and write using standard AVR EEPROM procedure.
 *          Polls EEWE bit for write completion.
 */

#include "mcal_eeprom.h"
/* ============================================
 * Private Macros
 * ============================================ */
#define DISABLE_INTERRUPTS()    __asm__ volatile ("cli" ::)

/* ============================================
 * Private Helper Functions
 * ============================================ */

/**
 * @brief   Wait for any ongoing EEPROM write to complete.
 * @note    Polls the EEWE bit in EECR.
 *          Blocks until write finishes.
 */
static inline void EEPROM_WaitReady(void) {
    while (READ_BIT(EECR, EECR_EEWE)) {
        /* Wait for write to finish */
    }
}


/* ============================================
 * Public Function Implementations
 * ============================================ */
u8_t EEPROM_Read(u16_t address) {
    u8_t data;
    
    /* Wait for any ongoing write to finish */
    EEPROM_WaitReady();
    
    /* Set up address */
    EEAR = address;   /* 16-bit address register */
    
    /* Start read by setting EERE bit */
    SET_BIT(EECR, EECR_EERE);
    
    /* Read data from EEDR */
    data = EEDR;
    
    return data;
}

void EEPROM_Write(u16_t address, u8_t data) {
    /* Wait for any ongoing write to finish */
    EEPROM_WaitReady();
    
    /* Set up address and data */
    EEAR = address;
    EEDR = data;   
    
    /* Write sequence (as per AVR datasheet) */
    /* Disable interrupts to avoid interruption of the 4-cycle sequence */
    u8_t sreg_value = SREG;
	DISABLE_INTERRUPTS();
    
    /* Set Master Write Enable */
    SET_BIT(EECR, EECR_EEMWE);
    
    /* Within 4 cycles, set EEWE to start write */
    SET_BIT(EECR, EECR_EEWE);
    
    /* Restore interrupts */
    SREG = sreg_value;
}