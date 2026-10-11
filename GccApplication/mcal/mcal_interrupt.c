/**
 * @file    mcal_interrupt.h
 * @brief   MCAL External Interrupt Driver for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Provides low-level functions to configure external interrupts.
 *          Supports INT0, INT1, and INT2 with all trigger modes.
 */

#include "mcal_interrupt.h"
/* ============================================
 * Private Macros
 * ============================================ */
#define DISABLE_INTERRUPTS()    __asm__ volatile ("cli" ::)
#define ENABLE_INTERRUPTS()     __asm__ volatile ("sei" ::)

/* ============================================
 * Private Variables
 * ============================================ */

/*
 * Stores the software state of global interrupts.
 *
 * STD_SET   -> Global interrupts are considered enabled.
 * STD_RESET -> Global interrupts are considered disabled.
 */
static volatile Std_StatusType_t global_interrupts_status = STD_RESET;


/* ============================================
 * Global Interrupt Control
 * ============================================ */
void INT_GlobalEnable(void)
{
    global_interrupts_status = STD_SET;
    ENABLE_INTERRUPTS();
}

void INT_GlobalDisable(void)
{
    global_interrupts_status = STD_RESET;
    DISABLE_INTERRUPTS();
}

void INT_GlobalSaveAndDisable(void)
{
    DISABLE_INTERRUPTS();
}

void INT_GlobalRestore()
{
    if (global_interrupts_status == STD_SET)	ENABLE_INTERRUPTS();
    else										DISABLE_INTERRUPTS();
}


/* ============================================
 * INT0 Control
 * ============================================ */
void INT0_Enable(INT_TriggerMode_t mode)
{
    /* Configure INT0 trigger mode */
	u8_t mcucr_value = MCUCR;
	 
	CLR_BIT(mcucr_value, MCUCR_ISC00);
	CLR_BIT(mcucr_value, MCUCR_ISC01); 
    mcucr_value |= ((mode & 0x03) << MCUCR_ISC00);

	MCUCR = mcucr_value;
    SET_BIT(GIFR, GIFR_INTF0);	// Clear any pending INT0 request.
	SET_BIT(GICR, GICR_INT0);	// Enable INT0 after configuring its trigger mode.
}

void INT0_Disable(void)
{
	CLR_BIT(GICR, GICR_INT0);
}
void INT0_ClearFlag(void)
{
	SET_BIT(GIFR, GIFR_INTF0);
}


/* ============================================
 * INT1 Control
 * ============================================ */
void INT1_Enable(INT_TriggerMode_t mode)
{
    /* Configure INT1 trigger mode */
	u8_t mcucr_value = MCUCR;

	CLR_BIT(MCUCR, MCUCR_ISC10);
	CLR_BIT(MCUCR, MCUCR_ISC11); 
    mcucr_value |= ((mode & 0x03) << MCUCR_ISC10);

	MCUCR = mcucr_value;
	SET_BIT(GIFR, GIFR_INTF1);	// Clear any pending INT1 request.
    SET_BIT(GICR, GICR_INT1);	// Enable INT1 after configuring its trigger mode.
}

void INT1_Disable(void)
{
	CLR_BIT(GICR, GICR_INT1);
}

void INT1_ClearFlag(void)
{
	SET_BIT(GICR, GICR_INT1);
}


/* ============================================
 * INT2 Control
 * ============================================ */
void INT2_Enable(INT2_TriggerMode_t mode)
{
    /* Configure INT2 trigger mode */
    if (mode == INT2_TRIGGER_RISING_EDGE)
    {
        SET_BIT(MCUCSR, MCUCSR_ISC2);
    }
    else
    {
        CLR_BIT(MCUCSR, MCUCSR_ISC2);
    }

    SET_BIT(GIFR, GIFR_INTF2);	// Clear any pending INT2 request.
    SET_BIT(GICR, GICR_INT2);	// Enable INT2 after configuring its trigger mode.
}

void INT2_Disable(void)
{
	CLR_BIT(GICR, GICR_INT2);
}

void INT2_ClearFlag(void)
{
	SET_BIT(GIFR, GIFR_INTF2);
}
