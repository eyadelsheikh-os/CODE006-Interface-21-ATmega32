/**
 * @file    mcal_adc.h
 * @brief   MCAL Analog-to-Digital Converter Driver for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Provides low-level functions to configure and control the
 *          on-chip 10-bit Successive Approximation ADC.
 *          Channels: ADC0–ADC7 (PORTA pins 0–7).
 */

#ifndef MCAL_ADC_H_
#define MCAL_ADC_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"		/* Provides u8_t, u16_t, bool_t */
#include "../common/std_macros.h"		/* Provides SET_BIT, CLR_BIT, etc. */
#include "../mcal/mcal_registers.h"		/* Provides ADMUX, ADCSRA, ADCL, ADCH, SFIOR */

/* ============================================
 * Bit Positions – ADC Multiplexer (ADMUX)
 * ============================================ */
#define ADMUX_MUX0    0		/**< Analog channel select bit 0 */
#define ADMUX_MUX1    1		/**< Analog channel select bit 1 */
#define ADMUX_MUX2    2		/**< Analog channel select bit 2 */
#define ADMUX_MUX3    3		/**< Analog channel select bit 3 */
#define ADMUX_MUX4    4		/**< Analog channel select bit 4 */
#define ADMUX_ADLAR   5		/**< ADC Left Adjust Result */
#define ADMUX_REFS0   6		/**< Reference Selection bit 0 */
#define ADMUX_REFS1   7		/**< Reference Selection bit 1 */

/* ============================================
 * Bit Positions – ADC Control & Status (ADCSRA)
 * ============================================ */
#define ADCSRA_ADPS0  0		/**< Prescaler bit 0 */
#define ADCSRA_ADPS1  1		/**< Prescaler bit 1 */
#define ADCSRA_ADPS2  2		/**< Prescaler bit 2 */
#define ADCSRA_ADIE   3		/**< ADC Interrupt Enable */
#define ADCSRA_ADIF   4		/**< ADC Interrupt Flag */
#define ADCSRA_ADATE  5		/**< ADC Auto Trigger Enable */
#define ADCSRA_ADSC   6		/**< ADC Start Conversion */
#define ADCSRA_ADEN   7		/**< ADC Enable */

/* ============================================
 * Bit Positions – Special Function IO (SFIOR)
 * ============================================ */
#define SFIOR_ADTS0   5		/**< Auto Trigger Source bit 0 */
#define SFIOR_ADTS1   6		/**< Auto Trigger Source bit 1 */
#define SFIOR_ADTS2   7		/**< Auto Trigger Source bit 2 */

/* ============================================
 * Public Enums – ADC
 * ============================================ */
typedef enum {
    ADC_REF_AREF,			/**< External AREF pin */
    ADC_REF_AVCC,			/**< AVCC with external cap on AREF */
    ADC_REF_INTERNAL_2V56	/**< Internal 2.56 V reference */
} ADC_Reference_t;

typedef enum {
    ADC_CH0,				/**< ADC0 – PA0 */
    ADC_CH1,				/**< ADC1 – PA1 */
    ADC_CH2,				/**< ADC2 – PA2 */
    ADC_CH3,				/**< ADC3 – PA3 */
    ADC_CH4,				/**< ADC4 – PA4 */
    ADC_CH5,				/**< ADC5 – PA5 */
    ADC_CH6,				/**< ADC6 – PA6 */
    ADC_CH7,				/**< ADC7 – PA7 */
} ADC_Channel_t;

typedef enum {
    ADC_CLK_DIV2	= 0,	/**< clk / 2   */
    ADC_CLK_DIV4	= 2,	/**< clk / 4   */
    ADC_CLK_DIV8	= 3,	/**< clk / 8   */
    ADC_CLK_DIV16	= 4,	/**< clk / 16  */
    ADC_CLK_DIV32	= 5,	/**< clk / 32  */
    ADC_CLK_DIV64	= 6,	/**< clk / 64  */
    ADC_CLK_DIV128	= 7		/**< clk / 128 */
} ADC_Clock_t;

typedef enum {
    ADC_TRIG_FREE_RUNNING,		/**< Free running mode */
    ADC_TRIG_ANALOG_COMP,		/**< Analog comparator */
    ADC_TRIG_EXT_INT0,			/**< External interrupt 0 */
    ADC_TRIG_TIMER0_COMPARE,	/**< Timer0 compare match */
    ADC_TRIG_TIMER0_OVERFLOW,	/**< Timer0 overflow */
    ADC_TRIG_TIMER1_COMPARE_B,	/**< Timer1 compare match B */
    ADC_TRIG_TIMER1_OVERFLOW,	/**< Timer1 overflow */
    ADC_TRIG_TIMER1_CAPTURE		/**< Timer1 capture event */
} ADC_TriggerSource_t;

/* ============================================
 * ADC – State (Opaque)
 * ============================================ */
typedef struct ADC_State_t ADC_State_t;
extern ADC_State_t adc;

/* ============================================
 * ADC – Public API
 * ============================================ */

/* ---- Configuration setters ---- */
/**
 * @brief   Reset the ADC configuration to safe defaults.
 * @note    Does NOT touch hardware — call ADC_Init() afterwards.
 *          Defaults: AREF reference, /2 prescaler, channel ADC0,
 *                    right-adjusted, no auto-trigger, no interrupt.
 */
void ADC_vReset(void);

void ADC_vSetReference(ADC_Reference_t ref);
void ADC_vSetClock(ADC_Clock_t clock);
void ADC_vSetChannel(ADC_Channel_t channel);
void ADC_vSetLeftAdjust(bool_t enable);
void ADC_vSetAutoTrigger(bool_t enable);
void ADC_vSetTriggerSource(ADC_TriggerSource_t source);
void ADC_vSetConversionInt(bool_t enable);

ADC_Reference_t ADC_vGetReference(void);
ADC_Clock_t ADC_vGetClock(void);
ADC_Channel_t ADC_vGetChannel(void);
ADC_TriggerSource_t ADC_vGetTriggerSource(void);

/* ---- Lifecycle ---- */

/**
 * @brief   Apply the stored configuration to the ADC hardware.
 *
 * @note    The ADC is left ENABLED (ADEN = 1) after this function.
 *          A dummy conversion is performed to prime the ADC, since
 *          the first conversion after enabling takes 25 ADC clocks
 *          instead of the usual 13.
 */
void ADC_Init(void);

/**
 * @brief   Enable the ADC (ADEN = 1).
 */
void ADC_Enable(void);

/**
 * @brief   Disable the ADC (ADEN = 0).
 */
void ADC_Disable(void);


/* ---- Runtime ---- */

/**
 * @brief   Start a single conversion (manual mode).
 *
 * @note    Only meaningful when auto-trigger is disabled. In auto-trigger
 *          mode, the hardware starts conversions on each trigger event.
 */
void  ADC_StartConversion(void);

/**
 * @brief   Check whether a conversion has finished.
 * @return  TRUE if ADSC has cleared (conversion done), FALSE otherwise.
 */
bool_t ADC_IsConversionComplete(void);

/**
 * @brief   return the 10-bit result.
 * @return  ADC value in the range 0–1023 (right-adjusted) or 0–1023 with
 *          a different bit layout depending on the ADLAR setting.
 *
 * @note    Always reads ADCL before ADCH — this is required by the hardware:
 *          reading ADCL locks the high byte, and reading ADCH releases it,
 *          guaranteeing a consistent 10-bit value.
 *
 *          If ADLAR = 0 (right-adjusted): result = (ADCH << 8) | ADCL
 *          If ADLAR = 1 (left-adjusted):  result = (ADCH << 2) | (ADCL >> 6)
 */
u16_t ADC_Read(void);

/**
 * @brief   Perform a blocking single conversion and return the 10-bit result.
 * @return  ADC value in the range 0–1023 (right-adjusted) or 0–1023 with
 *          a different bit layout depending on the ADLAR setting.
 *
 * @note    Always reads ADCL before ADCH — this is required by the hardware:
 *          reading ADCL locks the high byte, and reading ADCH releases it,
 *          guaranteeing a consistent 10-bit value.
 *
 *          If ADLAR = 0 (right-adjusted): result = (ADCH << 8) | ADCL
 *          If ADLAR = 1 (left-adjusted):  result = (ADCH << 2) | (ADCL >> 6)
 */
u16_t ADC_WaitRead(void);

/**
 * @brief   Change the channel and perform one blocking conversion.
 * @param   ch  Channel to select.
 * @return  ADC value (0–1023).
 *
 * @note    Changing the channel requires updating ADMUX, which takes
 *          effect on the next conversion. This function writes the new
 *          channel, starts one conversion, discards the result, then
 *          performs a second conversion and returns that result —
 *          guaranteeing the sample comes from the requested channel.
 */
u16_t ADC_ChannelSwitch(ADC_Channel_t ch);

/**
 * @brief   Enable the ADC conversion-complete interrupt.
 */
void ADC_EnableInterrupt(void);

/**
 * @brief   Disable the ADC conversion-complete interrupt.
 */
void ADC_DisableInterrupt(void);

/**
 * @brief   Clear an ADC interrupt flag.
 * @param   flag    ADC_FLAG_CONVERSION (ADIF).
 *
 * @note    ADIF is cleared by writing '1'. It is also cleared
 *          automatically when the ADC ISR executes.
 */
void ADC_ClearInterruptFlag(void);


#endif /* MCAL_ADC_H_ */