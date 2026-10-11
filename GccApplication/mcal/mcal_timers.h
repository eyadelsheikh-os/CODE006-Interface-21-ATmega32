/**
 * @file    mcal_timers.h
 * @brief   MCAL Timers/Counters Driver for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Provides low-level functions to configure and control
 *          the ATmega32 Timer/Counter peripherals (Timer0, Timer1, Timer2).
 */

#ifndef MCAL_TIMERS_H_
#define MCAL_TIMERS_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"			/* Provides u8_t, u16_t, bool_t */
#include "../common/std_macros.h"			/* Provides SET_BIT, CLR_BIT, etc. */
#include "../common/gpio_types.h"         /* Provides GPIO_Pin_t, GPIO_Direction_t, etc. */
#include "../mcal/mcal_registers.h"			/* Provides TCCR0, TCNT0, OCR0, TIMSK, TIFR */

/* ============================================
 * Bit Positions – Timer Interrupt Mask (TIMSK)
 * ============================================ */
#define TIMSK_TOIE0   0   /**< Timer0 Overflow Interrupt Enable */
#define TIMSK_OCIE0   1   /**< Timer0 Output Compare Interrupt Enable */
#define TIMSK_TOIE1   2   /**< Timer1 Overflow Interrupt Enable */
#define TIMSK_OCIE1B  3   /**< Timer1 Output Compare B Interrupt Enable */
#define TIMSK_OCIE1A  4   /**< Timer1 Output Compare A Interrupt Enable */
#define TIMSK_TICIE1  5   /**< Timer1 Input Capture Interrupt Enable */
#define TIMSK_TOIE2   6   /**< Timer2 Overflow Interrupt Enable */
#define TIMSK_OCIE2   7   /**< Timer2 Output Compare Interrupt Enable */

/* ============================================
 * Bit Positions – Timer Interrupt Flag (TIFR)
 * ============================================ */
#define TIFR_TOV0     0   /**< Timer0 Overflow Flag */
#define TIFR_OCF0     1   /**< Timer0 Output Compare Flag */
#define TIFR_TOV1     2   /**< Timer1 Overflow Flag */
#define TIFR_OCF1B    3   /**< Timer1 Output Compare B Flag */
#define TIFR_OCF1A    4   /**< Timer1 Output Compare A Flag */
#define TIFR_ICF1     5   /**< Timer1 Input Capture Flag */
#define TIFR_TOV2     6   /**< Timer2 Overflow Flag */
#define TIFR_OCF2     7   /**< Timer2 Output Compare Flag */

/* ============================================================
 * ============ TIMER 0 – 8-BIT COUNTER ================
 * ============================================================
 * Bit Positions – Timer/Counter 0 (TCCR0)
 * ============================================ */
#define TCCR0_CS00    0   /**< Clock Select bit 0 */
#define TCCR0_CS01    1   /**< Clock Select bit 1 */
#define TCCR0_CS02    2   /**< Clock Select bit 2 */
#define TCCR0_WGM01   3   /**< Waveform Generation Mode bit 1 */
#define TCCR0_COM00   4   /**< Compare Match Output Mode bit 0 */
#define TCCR0_COM01   5   /**< Compare Match Output Mode bit 1 */
#define TCCR0_WGM00   6   /**< Waveform Generation Mode bit 0 */
#define TCCR0_FOC0    7   /**< Force Output Compare */

/* ============================================
 * Public Enums – Timer 0
 * ============================================ */
typedef enum {
    TC0_MODE_NORMAL,      /**< Count up, overflow at max (0xFF) */
    TC0_MODE_PHASE_PWM,   /**< Phase-correct PWM */
    TC0_MODE_CTC,         /**< Clear Timer on Compare match (period = OCR0) */
    TC0_MODE_FAST_PWM     /**< Fast PWM, top = 0xFF or OCR0 */
} TC0_Mode_t;

typedef enum {
    TC0_CLK_STOP,         /**< No clock source (timer stopped) */
    TC0_CLK_DIV1,         /**< clk / 1 */
    TC0_CLK_DIV8,         /**< clk / 8 */
    TC0_CLK_DIV64,        /**< clk / 64 */
    TC0_CLK_DIV256,       /**< clk / 256 */
    TC0_CLK_DIV1024,      /**< clk / 1024 */
    TC0_CLK_EXT_FALLING,  /**< External clock, falling edge (T0 pin) */
    TC0_CLK_EXT_RISING    /**< External clock, rising edge (T0 pin) */
} TC0_Clock_t;

typedef enum {
    TC0_FLAG_OVERFLOW,    /**< Overflow flag (TOV0) */
    TC0_FLAG_COMPARE      /**< Output Compare flag (OCF0) */
} TC0_Flag_t;

typedef enum {
	OC0_MODE_DISCONNECTED,			/**< Normal port operation */
	OC0_MODE_TOGGLE,				/**< Toggle OC0 on compare match */
	OC0_MODE_CLEAR,					/**< Clear OC0 on compare match */
	OC0_MODE_SET,					/**< Set OC0 on compare match */
	OC0_MODE_PWM_CLEAR	= 2,		/**< Clear OC0 on compare match. Set OC0 at BOTTOM */
	OC0_MODE_PWM_SET	= 3,		/**< Set OC0 on compare match. Clear OC0 at BOTTOM */
} OC0_Mode_t;

/* ============================================
 * Timer 0 – State (Opaque)
 * ============================================ */
typedef struct TC0_State_t TC0_State_t;
extern TC0_State_t timer0;

/* ============================================
 * Timer 0 – Public API
 * ============================================ */

/* ---- Configuration setters ---- */

/**
 * @brief   Resets the status variable "Timer0" to the default initial state.
 */
void TC0_vReset(void);

/**
 * @brief   Set the waveform generation mode for Timer0 status variable.
 * @param   mode    TC0_MODE_NORMAL, _PHASE_PWM, _CTC, or _FAST_PWM
 */
void TC0_vSetMode(TC0_Mode_t mode);

/**
 * @brief   Set the clock source (prescaler) for Timer0 status variable.
 * @param   clock   TC0_CLK_* value
 */
void TC0_vSetClock(TC0_Clock_t clock);

/**
 * @brief   Set the Output Compare value for Timer0 status variable.
 * @param   compare Compare value (0-255)
 */
void TC0_vSetCompareValue(u8_t compare);

/**
 * @brief   Enable or disable the Timer0 Overflow interrupt status variable.
 * @param   enable  TRUE = enable, FALSE = disable
 */
void TC0_vSetOverflowInt(bool_t enable);

/**
 * @brief   Enable or disable the Timer0 Compare Match interrupt status variable.
 * @param   enable  TRUE = enable, FALSE = disable
 */
void TC0_vSetCompareInt(bool_t enable);

/**
 * @brief   Set the output mode configuration of the pin OC0 status variable.
 * @param   mode    OC0_MODE_NORMAL, _TOGGLE, _CLEAR, or _SET
 */
void TC0_vSetOutputMode(OC0_Mode_t mode);

/**
 * @brief   Get the waveform generation mode for Timer0.
 * @return  mode    TC0_MODE_NORMAL, _PHASE_PWM, _CTC, or _FAST_PWM
 */
TC0_Mode_t TC0_vGetMode(void);

/**
 * @brief   Get the clock source (prescaler) for Timer0.
 * @return  clock   TC0_CLK_* value
 */
TC0_Clock_t TC0_vGetClock(void);

/**
 * @brief   Get the output mode configuration of the pin OC0.
 * @return	mode    OC0_MODE_NORMAL, _TOGGLE, _CLEAR, or _SET
 */
OC0_Mode_t TC0_vGetOutputMode(void);


/* ---- Lifecycle ---- */

/**
 * @brief   Apply the stored configuration to the Timer0 hardware.
 */
void TC0_Init(void);

/**
 * @brief   Start Timer0 with the given clock source.
 * @param   clock   TC0_CLK_* value
 * 
 * @note    Can be called at any time to change the prescaler.
 */
void TC0_Enable(TC0_Clock_t clock);  /**< Adjust timer clock */

/**
 * @brief   Stop Timer0 (no clock source).
 */
void TC0_Disable(void);

/**
 * @brief   Start OC0 "output pin" with the given mode.
 * @param   mode   OC0_MODE_* value
 * 
 * @note    Can be called at any time to change the mode.
 */
void TC0_OutputEnable(OC0_Mode_t mode);

/**
 * @brief   Stops OC0 (Set to normal operation).
 */
void TC0_OutputDisable(void);


/* ---- Runtime ---- */

/**
 * @brief   Set the Timer0 counter value.
 * @param   value   Value to load into TCNT0.
 */
void TC0_SetCounterValue(u8_t value);

/**
 * @brief   Get the current Timer0 counter value.
 * @return  TCNT0 register value.
 */
u8_t TC0_GetCounterValue(void);

/**
 * @brief   Set the Output Compare value at runtime.
 * @param   value   New OCR0 value.
 */
void TC0_SetCompareValue(u8_t value);

/**
 * @brief   Get the current Output Compare value.
 * @return  OCR0 register value.
 */
u8_t TC0_GetCompareValue(void);

/**
 * @brief   Enable the Timer0 interrupt.
 * @param   flag    TC0_FLAG_OVERFLOW or TC0_FLAG_COMPARE
 */
void TC0_EnableInterrupt(TC0_Flag_t flag);

/**
 * @brief   Disable the Timer0 interrupt.
 * @param   flag    TC0_FLAG_OVERFLOW or TC0_FLAG_COMPARE
 */
void TC0_DisableInterrupt(TC0_Flag_t flag);

/**
 * @brief   Clear a Timer0 interrupt flag.
 * @param   flag    TC0_FLAG_OVERFLOW or TC0_FLAG_COMPARE
 * 
 * @note    Flags in TIFR are cleared by writing '1'.
 */
void TC0_ClearInterruptFlag(TC0_Flag_t flag);


/* ============================================================
 * ================ TIMER 1 – 16-BIT COUNTER ==================
 * ============================================================
 * Bit Positions – Timer/Counter 1 (TCCR1A)
 * ============================================ */
#define TCCR1A_WGM10   0   /**< Waveform Generation Mode bit 0 */
#define TCCR1A_WGM11   1   /**< Waveform Generation Mode bit 1 */
#define TCCR1A_FOC1B   2   /**< Force Output Compare B */
#define TCCR1A_FOC1A   3   /**< Force Output Compare A */
#define TCCR1A_COM1B0  4   /**< Compare Match Output B Mode bit 0 */
#define TCCR1A_COM1B1  5   /**< Compare Match Output B Mode bit 1 */
#define TCCR1A_COM1A0  6   /**< Compare Match Output A Mode bit 0 */
#define TCCR1A_COM1A1  7   /**< Compare Match Output A Mode bit 1 */

/* ============================================
 * Bit Positions – Timer/Counter 1 (TCCR1B)
 * ============================================ */
#define TCCR1B_CS10    0   /**< Clock Select bit 0 */
#define TCCR1B_CS11    1   /**< Clock Select bit 1 */
#define TCCR1B_CS12    2   /**< Clock Select bit 2 */
#define TCCR1B_WGM12   3   /**< Waveform Generation Mode bit 2 */
#define TCCR1B_WGM13   4   /**< Waveform Generation Mode bit 3 */
#define TCCR1B_ICES1   6   /**< Input Capture Edge Select */
#define TCCR1B_ICNC1   7   /**< Input Capture Noise Canceler */

/* ============================================
 * Public Enums – Timer 1
 * ============================================ */
typedef enum {
    TC1_MODE_NORMAL,                    /**< Normal, TOP = 0xFFFF */
    TC1_MODE_PHASE_PWM_8BIT,			/**< Phase-correct PWM, TOP = 0x00FF */
    TC1_MODE_PHASE_PWM_9BIT,			/**< Phase-correct PWM, TOP = 0x01FF */
    TC1_MODE_PHASE_PWM_10BIT,			/**< Phase-correct PWM, TOP = 0x03FF */
    TC1_MODE_CTC_OCR1A,                 /**< CTC, TOP = OCR1A */
    TC1_MODE_FAST_PWM_8BIT,             /**< Fast PWM, TOP = 0x00FF */
    TC1_MODE_FAST_PWM_9BIT,             /**< Fast PWM, TOP = 0x01FF */
    TC1_MODE_FAST_PWM_10BIT,            /**< Fast PWM, TOP = 0x03FF */
    TC1_MODE_FREQ_PWM_ICR1,				/**< Phase & freq correct, TOP = ICR1 */
    TC1_MODE_FREQ_PWM_OCR1A,			/**< Phase & freq correct, TOP = OCR1A */
    TC1_MODE_PHASE_PWM_ICR1,			/**< Phase-correct, TOP = ICR1 */
    TC1_MODE_PHASE_PWM_OCR1A,			/**< Phase-correct, TOP = OCR1A */
    TC1_MODE_CTC_ICR1,                  /**< CTC, TOP = ICR1 */
    TC1_MODE_FAST_PWM_ICR1,             /**< Fast PWM, TOP = ICR1 */
    TC1_MODE_FAST_PWM_OCR1A             /**< Fast PWM, TOP = OCR1A */
} TC1_Mode_t;

typedef enum {
    TC1_CLK_STOP,         /**< No clock source */
    TC1_CLK_DIV1,         /**< clk / 1 */
    TC1_CLK_DIV8,         /**< clk / 8 */
    TC1_CLK_DIV64,        /**< clk / 64 */
    TC1_CLK_DIV256,       /**< clk / 256 */
    TC1_CLK_DIV1024,      /**< clk / 1024 */
    TC1_CLK_EXT_FALLING,  /**< External clock, falling edge (T1 pin) */
    TC1_CLK_EXT_RISING    /**< External clock, rising edge (T1 pin) */
} TC1_Clock_t;

typedef enum {
    TC1_FLAG_OVERFLOW,    /**< Overflow flag (TOV1) */
    TC1_FLAG_COMPARE_A,   /**< Output Compare A flag (OCF1A) */
    TC1_FLAG_COMPARE_B,   /**< Output Compare B flag (OCF1B) */
    TC1_FLAG_CAPTURE      /**< Input Capture flag (ICF1) */
} TC1_Flag_t;

typedef enum {
    IC1_MODE_NORMAL,	/**< Normal port operation */
    IC1_MODE_FALLING,	/**< Capture on falling edge */
    IC1_MODE_RISING		/**< Capture on rising edge */
} IC1_Mode_t;

typedef enum {
	OC1_MODE_DISCONNECTED,		/**< Normal port operation */
	OC1_MODE_TOGGLE,			/**< Toggle OC1A/OC1B on compare match */
	OC1_MODE_CLEAR,				/**< Clear OC1A/OC1B on compare match */
	OC1_MODE_SET,				/**< Set OC1A/OC1B on compare match */
	OC1_MODE_PWM_TOGGLE	= 1,	/**< TC1_MODE_FAST_PWM_OCR1A->Toggle OC1A on compare match. OC1B in normal port operation*/
	OC1_MODE_PWM_CLEAR	= 2,	/**< Clear OC1A/OC1B on compare match. Set OC1A/OC1B at BOTTOM */
	OC1_MODE_PWM_SET	= 3,	/**< Set OC1A/OC1B on compare match. Clear OC1A/OC1B at BOTTOM */
} OC1_Mode_t;


/* ============================================
 * Timer 1 – State (Opaque)
 * ============================================ */
typedef struct TC1_State_t TC1_State_t;
extern TC1_State_t timer1;


/* ============================================
 * Timer 1 – Public API
 * ============================================ */

/* ---- Configuration setters ---- */

/**
 * @brief   Resets the status variable "Timer1" to the default initial state.
 */
void TC1_vReset(void);

/**
 * @brief   Set the waveform generation mode for Timer1 status variable.
 * @param   mode    TC1_MODE_NORMAL, _PHASE_PWM, _CTC, or _FAST_PWM
 */
void TC1_vSetMode(TC1_Mode_t mode);

/**
 * @brief   Set the clock source (prescaler) for Timer1 status variable.
 * @param   clock   TC1_CLK_* value
 */
void TC1_vSetClock(TC1_Clock_t clock);

/**
 * @brief   Set the TOP value for Timer1 status variable.
 * @param   compare Compare value (0-0xFFFF)
 */
void TC1_vSetTopValue(u16_t top);

/**
 * @brief   Set the Output Compare A value for Timer1 status variable.
 * @param   compare Compare value (0-0xFFFF)
 */
void TC1_vSetCompareAValue(u16_t value);

/**
 * @brief   Set the Output Compare A value for Timer1 status variable.
 * @param   compare Compare value (0-0xFFFF)
 */
void TC1_vSetCompareBValue(u16_t value);

/**
 * @brief   Enable or disable the Timer1 Overflow interrupt status variable.
 * @param   enable  TRUE = enable, FALSE = disable
 */
void TC1_vSetOverflowInt(bool_t enable);

/**
 * @brief   Enable or disable the Timer1 Compare A Match interrupt status variable.
 * @param   enable  TRUE = enable, FALSE = disable
 */
void TC1_vSetCompareAInt(bool_t enable);

/**
 * @brief   Enable or disable the Timer1 Compare B Match interrupt status variable.
 * @param   enable  TRUE = enable, FALSE = disable
 */
void TC1_vSetCompareBInt(bool_t enable);

/**
 * @brief   Enable or disable the Timer1 Compare Match interrupt status variable.
 * @param   enable  TRUE = enable, FALSE = disable
 */
void TC1_vSetCaptureInt(bool_t enable);

/**
 * @brief   Set which edge of the ICP1 signal triggers an input capture status variable.
 * @param   edge    TC1_CAPTURE_FALLING or TC1_CAPTURE_RISING
 */
void TC1_vSetInputMode(IC1_Mode_t mode);

/**
 * @brief   Enable or disable the Input Capture Noise Canceler status variable.
 * @param   enable  TRUE = enable, FALSE = disable
 */
void TC1_vSetNoiseCanceler(bool_t enable);

/**
 * @brief   Set output A mode configuration of the pin OC1A status variable.
 * @param   mode    OC1_MODE_NORMAL, _TOGGLE, _CLEAR, or _SET
 */
void TC1_vSetOutputAMode(OC1_Mode_t mode);

/**
 * @brief   Set output B mode configuration of the pin OC1B status variable.
 * @param   mode    OC1_MODE_NORMAL, _TOGGLE, _CLEAR, or _SET
 */
void TC1_vSetOutputBMode(OC1_Mode_t mode);

/**
 * @brief   Get the waveform generation mode for Timer1 status variable.
 * @return  mode    TC1_MODE_NORMAL, _PHASE_PWM, _CTC, or _FAST_PWM
 */
TC1_Mode_t TC1_vGetMode(void);

/**
 * @brief   Get the clock source (prescaler) for Timer1 status variable.
 * @return  clock   TC1_CLK_* value
 */
TC1_Clock_t TC1_vGetClock(void);

/**
 * @brief   Get the TOP value for Timer1 status variable.
 * @return  compare Compare value (0-0xFFFF)
 */
u16_t TC1_vGetTopValue(void);

/**
 * @brief   Get the Output Compare A value for Timer1 status variable.
 * @return  compare Compare value (0-0xFFFF)
 */
u16_t TC1_vGetCompareAValue(void);

/**
 * @brief   Get the Output Compare A value for Timer1 status variable.
 * @return  compare Compare value (0-0xFFFF)
 */
u16_t TC1_vGetCompareBValue(void);

/**
 * @brief    Get output A mode configuration of the pin OC1A status variable.
 * @return   mode    OC1_MODE_NORMAL, _TOGGLE, _CLEAR, or _SET
 */
OC1_Mode_t TC1_vGetOutputAMode(void);

/**
 * @brief   Get output B mode configuration of the pin OC1B status variable.
 * @return  mode    OC1_MODE_NORMAL, _TOGGLE, _CLEAR, or _SET
 */
OC1_Mode_t TC1_vGetOutputBMode(void);

/**
 * @brief   Get which edge of the ICP1 signal triggers an input capture status variable.
 * @return  edge    TC1_CAPTURE_FALLING or TC1_CAPTURE_RISING
 */
IC1_Mode_t TC1_vGetInputMode(void);



/* ---- Lifecycle ---- */

/**
 * @brief   Apply the stored configuration to the Timer1 hardware.
 */
void TC1_Init(void);

/**
 * @brief   Start Timer0 with the given clock source.
 * @param   clock   TC1_CLK_* value
 * 
 * @note    Can be called at any time to change the prescaler.
 */
void TC1_Enable(TC1_Clock_t clock);

/**
 * @brief   Stop Timer1 (no clock source).
 */
void TC1_Disable(void);

/**
 * @brief   Start OC1A "output pin" with the given mode.
 * @param   mode   OC1_MODE_* value
 * 
 * @note    Can be called at any time to change the mode.
 */
void TC1_OutputAEnable(OC1_Mode_t mode);

/**
 * @brief   Stops OC1A (Set to normal operation).
 */
void TC1_OutputADisable(void);

/**
 * @brief   Start OC1B "output pin" with the given mode.
 * @param   mode   OC1_MODE_* value
 * 
 * @note    Can be called at any time to change the mode.
 */
void TC1_OutputBEnable(OC1_Mode_t mode);

/**
 * @brief   Stops OC1B (Set to normal operation).
 */
void TC1_OutputBDisable(void);

/**
 * @brief   Start ICP1 "input pin" with the given mode.
 * @param   mode   IC1_MODE_* value
 * 
 * @note    Can be called at any time to change the mode.
 */
void TC1_InputEnable(IC1_Mode_t mode);

/**
 * @brief   Stops ICP1 (Set to normal operation).
 */
void TC1_InputDisable(void);


/* ---- Runtime ---- */

/**
 * @brief   Set the Timer1 counter value (16-bit).
 * @param   value   Value to load into TCNT1.
 */
void TC1_SetCounterValue(u16_t value);

/**
 * @brief   Get the current Timer1 counter value (16-bit).
 * @return  TCNT1 register value (0–65535).
 */
u16_t TC1_GetCounterValue(void);

/**
 * @brief   Set the Timer1 Output Compare A value at runtime.
 * @param   value   New OCR1A value (0–65535).
 *
 * @note    In CTC / PWM modes where OCR1A is TOP, changing this
 *          also changes the timer period.
 */
void TC1_SetCompareAValue(u16_t value);

/**
 * @brief   Get the current Timer1 Output Compare A value.
 * @return  OCR1A register value (0–65535).
 */
u16_t TC1_GetCompareAValue(void);

/**
 * @brief   Set the Timer1 Output Compare B value at runtime.
 * @param   value   New OCR1B value (0–65535).
 *
 * @note    OCR1B is used as a second PWM duty cycle in modes that
 *          support two PWM outputs on OC1A and OC1B.
 */
void TC1_SetCompareBValue(u16_t value);

/**
 * @brief   Get the current Timer1 Output Compare B value.
 * @return  OCR1B register value (0–65535).
 */
u16_t TC1_GetCompareBValue(void);

/**
 * @brief   Set the Timer1 TOP value at runtime.
 * @param   top     New TOP value (0–65535), written to ICR1.
 *
 * @note    Only affects modes where TOP = ICR1:
 *          - TC1_MODE_PWM_PHASE_FREQ_ICR1
 *          - TC1_MODE_PWM_PHASE_CORRECT_ICR1
 *          - TC1_MODE_CTC_ICR1
 *          - TC1_MODE_FAST_PWM_ICR1
 *          In other modes ICR1 is used for Input Capture, and
 *          writing it here would corrupt the capture result.
 */
void TC1_SetTopValue(u16_t top);

/**
 * @brief   Read the last captured Timer1 value (16-bit).
 * @return  ICR1 register value (0–65535).
 */
u16_t TC1_GetCaptureValue(void);

/**
 * @brief   Enable a Timer1 interrupt source.
 * @param   flag    TC1_FLAG_OVERFLOW,  TC1_FLAG_COMPARE_A,
 *                  TC1_FLAG_COMPARE_B, or TC1_FLAG_CAPTURE
 */
void TC1_EnableInterrupt(TC1_Flag_t flag);

/**
 * @brief   Disable a Timer1 interrupt source.
 * @param   flag    TC1_FLAG_OVERFLOW,  TC1_FLAG_COMPARE_A,
 *                  TC1_FLAG_COMPARE_B, or TC1_FLAG_CAPTURE
 */
void TC1_DisableInterrupt(TC1_Flag_t flag);

/**
 * @brief   Wait for a Timer1 interrupt flag to become set.
 * @param   flag    Which flag to wait on.
 */
void TC1_WaitInterruptFlag(TC1_Flag_t flag);

/**
 * @brief   Clear a Timer1 interrupt flag.
 * @param   flag    TC1_FLAG_OVERFLOW,  TC1_FLAG_COMPARE_A,
 *                  TC1_FLAG_COMPARE_B, or TC1_FLAG_CAPTURE
 *
 * @note    Flags in TIFR are cleared by writing '1' to the bit
 */
void TC1_ClearInterruptFlag(TC1_Flag_t flag);

/**
 * @brief   Read Timer1 input captured flag.
 */
bool_t TC1_IsInputCapture(void);


/* ============================================================
 * ============ TIMER 2 – 8-BIT COUNTER ================
 * ============================================================
  * Bit Positions – Timer/Counter 2 (TCCR2)
 * ============================================ */
#define TCCR2_CS20    0   /**< Clock Select bit 0 */
#define TCCR2_CS21    1   /**< Clock Select bit 1 */
#define TCCR2_CS22    2   /**< Clock Select bit 2 */
#define TCCR2_WGM21   3   /**< Waveform Generation Mode bit 1 */
#define TCCR2_COM20   4   /**< Compare Match Output Mode bit 0 */
#define TCCR2_COM21   5   /**< Compare Match Output Mode bit 1 */
#define TCCR2_WGM20   6   /**< Waveform Generation Mode bit 0 */
#define TCCR2_FOC2    7   /**< Force Output Compare */


/* ============================================
 * Public Enums – Timer 2
 * ============================================ */
typedef enum {
    TC2_MODE_NORMAL,      /**< Count up, overflow at max (0xFF) */
    TC2_MODE_PHASE_PWM,   /**< Phase-correct PWM */
    TC2_MODE_CTC,         /**< Clear Timer on Compare match (period = OCR2) */
    TC2_MODE_FAST_PWM     /**< Fast PWM, top = 0xFF or OCR2 */
} TC2_Mode_t;

typedef enum {
    TC2_CLK_STOP,         /**< No clock source (timer stopped) */
    TC2_CLK_DIV1,         /**< clk / 1 */
    TC2_CLK_DIV8,         /**< clk / 8 */
    TC2_CLK_DIV32,        /**< clk / 32 */
    TC2_CLK_DIV64,        /**< clk / 64 */
    TC2_CLK_DIV128,       /**< clk / 128 */
    TC2_CLK_DIV256,       /**< clk / 256 */
    TC2_CLK_DIV1024       /**< clk / 1024 */
} TC2_Clock_t;

typedef enum {
    TC2_FLAG_OVERFLOW,    /**< Overflow flag (TOV2) */
    TC2_FLAG_COMPARE      /**< Output Compare flag (OCF2) */
} TC2_Flag_t;

typedef enum {
	OC2_MODE_DISCONNECTED, 		/**< Normal port operation */
	OC2_MODE_TOGGLE,			/**< Toggle OC2 on compare match */
	OC2_MODE_CLEAR,				/**< Clear OC2 on compare match */
	OC2_MODE_SET,				/**< Set OC2 on compare match */
	OC2_MODE_PWM_CLEAR	= 2,	/**< Clear OC2 on compare match. Set OC2 at BOTTOM */
	OC2_MODE_PWM_SET	= 3,	/**< Set OC2 on compare match. Clear OC2 at BOTTOM */
} OC2_Mode_t;


/* ============================================
 * Timer 2 – State (Opaque)
 * ============================================ */
typedef struct TC2_State_t TC2_State_t;
extern TC2_State_t timer2;


/* ============================================
 * Timer 2 – Public API
 * ============================================ */

/* ---- Configuration setters ---- */

/**
 * @brief   Resets the status variable "Timer2" to the default initial state.
 */
void TC2_vReset(void);

/**
 * @brief   Set the waveform generation mode for Timer2.
 * @param   mode    TC2_MODE_NORMAL, _PHASE_PWM, _CTC, or _FAST_PWM
 */
void TC2_vSetMode(TC2_Mode_t mode);

/**
 * @brief   Set the clock source (prescaler) for Timer2.
 * @param   clock   TC2_CLK_* value
 */
void TC2_vSetClock(TC2_Clock_t clock);

/**
 * @brief   Set the Output Compare value for Timer2.
 * @param   compare Compare value (0-255)
 */
void TC2_vSetCompareValue(u8_t compare);

/**
 * @brief   Enable or disable the Timer2 Overflow interrupt.
 * @param   enable  TRUE = enable, FALSE = disable
 */
void TC2_vSetOverflowInt(bool_t enable);

/**
 * @brief   Enable or disable the Timer2 Compare Match interrupt.
 * @param   enable  TRUE = enable, FALSE = disable
 */
void TC2_vSetCompareInt(bool_t enable);

/**
 * @brief   Set the output mode configuration of the pin OC2.
 * @param   mode    OC2_MODE_NORMAL, _TOGGLE, _CLEAR, or _SET
 */
void TC2_vSetOutputMode(OC2_Mode_t mode);

/**
 * @brief   Get the waveform generation mode for Timer0.
 * @return  mode    TC2_MODE_NORMAL, _PHASE_PWM, _CTC, or _FAST_PWM
 */
TC2_Mode_t TC2_vGetMode(void);

/**
 * @brief   Get the clock source (prescaler) for Timer0.
 * @return  clock   TC2_CLK_* value
 */
TC2_Clock_t TC2_vGetClock(void);

/**
 * @brief   Get the output mode configuration of the pin OC2.
 * @return	mode    OC2_MODE_NORMAL, _TOGGLE, _CLEAR, or _SET
 */
OC2_Mode_t TC2_vGetOutputMode(void);


/* ---- Lifecycle ---- */

/**
 * @brief   Apply the stored configuration to the Timer2 hardware.
 */
void TC2_Init(void);

/**
 * @brief   Start Timer0 with the given clock source.
 * @param   clock   TC2_CLK_* value
 * 
 * @note    Can be called at any time to change the prescaler.
 */
void TC2_Enable(TC2_Clock_t clock);

/**
 * @brief   Stop Timer2 (no clock source).
 */
void TC2_Disable(void);

/**
 * @brief   Start OC2 "output pin" with the given mode.
 * @param   mode   OC2_MODE_* value
 * 
 * @note    Can be called at any time to change the mode.
 */
void TC2_OutputEnable(OC2_Mode_t mode);

/**
 * @brief   Stops OC2 (Set to normal operation).
 */
void TC2_OutputDisable(void);


/* ---- Runtime ---- */

/**
 * @brief   Set the Timer2 counter value.
 * @param   value   Value to load into TCNT2.
 */
void TC2_SetCounterValue(u8_t value);

/**
 * @brief   Get the current Timer2 counter value.
 * @return  TCNT2 register value.
 */
u8_t TC2_GetCounterValue(void);

/**
 * @brief   Set the Output Compare value at runtime.
 * @param   value   New OCR2 value.
 */
void TC2_SetCompareValue(u8_t value);

/**
 * @brief   Get the current Output Compare value.
 * @return  OCR2 register value.
 */
u8_t TC2_GetCompareValue(void);

/**
 * @brief   Enable the Timer2 interrupt.
 * @param   flag    TC2_FLAG_OVERFLOW or TC2_FLAG_COMPARE
 */
void TC2_EnableInterrupt(TC2_Flag_t flag);

/**
 * @brief   Disable the Timer2 interrupt.
 * @param   flag    TC2_FLAG_OVERFLOW or TC2_FLAG_COMPARE
 */
void TC2_DisableInterrupt(TC2_Flag_t flag);

/**
 * @brief   Clear a Timer2 interrupt flag.
 * @param   flag    TC2_FLAG_OVERFLOW or TC2_FLAG_COMPARE
 */
void TC2_ClearInterruptFlag(TC2_Flag_t flag);


#endif /* MCAL_TIMERS_H_ */





