/**
 * @file    mcal_interrupt.h
 * @brief   MCAL External Interrupt Driver for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Provides low-level functions to configure external interrupts.
 *          Supports INT0, INT1, and INT2 with all trigger modes.
 */

#ifndef MCAL_INTERRUPT_H_
#define MCAL_INTERRUPT_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"          /* Provides u8, u16, etc. */
#include "../common/std_macros.h"         /* Provides SET_BIT, CLR_BIT, etc. */
#include "../mcal/mcal_registers.h"       /* Provides PORTA, DDRA, etc. */

/* ============================================
 * Bit Positions – Interrupt Control Registers
 * ============================================ */
/* MCUCR Control Register */
#define MCUCR_ISC00		0   /**< INT0 Sense Control Bit 0 */
#define MCUCR_ISC01		1   /**< INT0 Sense Control Bit 1 */
#define MCUCR_ISC10		2   /**< INT1 Sense Control Bit 0 */
#define MCUCR_ISC11		3   /**< INT1 Sense Control Bit 1 */

/* MCUCSR Control Register */
#define MCUCSR_ISC2		6   /**< INT2 Sense Control Bit */

/* GICR Control Register */
#define GICR_INT2		5   /**< INT2 Enable Bit */
#define GICR_INT0		6   /**< INT0 Enable Bit */
#define GICR_INT1		7   /**< INT1 Enable Bit */

/* GIFR Control Register */
#define GIFR_INTF2		5   /**< INT2 Interrupt Flag Bit */
#define GIFR_INTF0		6   /**< INT0 Interrupt Flag Bit */
#define GIFR_INTF1		7   /**< INT1 Interrupt Flag Bit */


/* ============================================
 * INT0 and INT1 Trigger Modes
 * ============================================ */
typedef enum
{
    INT_TRIGGER_LOW_LEVEL    = 0,	/**< Interrupt on low level */
    INT_TRIGGER_ANY_CHANGE   = 1,	/**< Interrupt on any logical change */
    INT_TRIGGER_FALLING_EDGE = 2,	/**< Interrupt on falling edge */
    INT_TRIGGER_RISING_EDGE  = 3	/**< Interrupt on rising edge */
	
} INT_TriggerMode_t;

/* ============================================
 * INT2 Trigger Modes
 * ============================================ */
typedef enum
{
    INT2_TRIGGER_FALLING_EDGE = 0,		/**< Interrupt on falling edge */
    INT2_TRIGGER_RISING_EDGE  = 1		/**< Interrupt on rising edge */
	
} INT2_TriggerMode_t;

/* ============================================
 * Global Interrupt Control
 * ============================================ */

/**
 * @brief Enable global interrupts.
 */
void INT_GlobalEnable(void);

/**
 * @brief Disable global interrupts.
 */
void INT_GlobalDisable(void);

/**
 * @brief Save the current software interrupt state and disable interrupts.
 */
void INT_GlobalSaveAndDisable(void);

/**
 * @brief Restore a previously saved global interrupt state.
 */
void INT_GlobalRestore(void);


/* ============================================
 * INT0 Control
 * ============================================ */

/**
 * @brief Configure and enable INT0.
 * @param mode INT0 trigger mode.
 */
void INT0_Enable(INT_TriggerMode_t mode);

/**
 * @brief Disable INT0.
 */
void INT0_Disable(void);

/**
 * @brief Clear the INT0 interrupt flag.
 */
void INT0_ClearFlag(void);


/* ============================================
 * INT1 Control
 * ============================================ */

/**
 * @brief Configure and enable INT1.
 * @param mode INT1 trigger mode.
 */
void INT1_Enable(INT_TriggerMode_t mode);

/**
 * @brief Disable INT1.
 */
void INT1_Disable(void);

/**
 * @brief Clear the INT1 interrupt flag.
 */
void INT1_ClearFlag(void);


/* ============================================
 * INT2 Control
 * ============================================ */

/**
 * @brief Configure and enable INT2.
 * @param mode INT2 trigger mode.
 */
void INT2_Enable(INT2_TriggerMode_t mode);

/**
 * @brief Disable INT2.
 */
void INT2_Disable(void);

/**
 * @brief Clear the INT2 interrupt flag.
 */
void INT2_ClearFlag(void);


#endif /* MCAL_INTERRUPT_H_ */