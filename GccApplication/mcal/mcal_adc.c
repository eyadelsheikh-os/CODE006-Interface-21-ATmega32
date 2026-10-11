/**
 * @file    mcal_adc.c
 * @brief   MCAL Analog-to-Digital Converter Driver Implementation
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Implements 10-bit single-conversion and auto-trigger ADC control
 *          with the ATmega32's built-in Successive Approximation ADC.
 *          All register accesses go through mcal_registers.h.
 */

#include "mcal_adc.h"
/* ============================================================
 * ADC – Private State
 * ============================================================ */
struct ADC_State_t {
    ADC_Reference_t			reference;
    ADC_Clock_t				clock;
    ADC_Channel_t			channel;
    bool_t					left_adjust;
    bool_t					auto_trigger;
    ADC_TriggerSource_t		trigger_source;
    bool_t					enable_int;
};
ADC_State_t adc;


/* ============================================================
 * ADC – Configuration Setters
 * ============================================================ */
void ADC_vReset(void) {
    adc.reference		= ADC_REF_AREF;
    adc.clock			= ADC_CLK_DIV2;
    adc.channel			= ADC_CH0;
    adc.left_adjust		= FALSE;
    adc.auto_trigger	= FALSE;
    adc.trigger_source	= ADC_TRIG_FREE_RUNNING;
    adc.enable_int		= FALSE;
}

void ADC_vSetReference(ADC_Reference_t ref)			{ adc.reference = ref; }
void ADC_vSetClock(ADC_Clock_t clock)				{ adc.clock = clock; }
void ADC_vSetChannel(ADC_Channel_t channel)			{ adc.channel = channel; }
void ADC_vSetLeftAdjust(bool_t enable)				{ adc.left_adjust = enable; }
void ADC_vSetAutoTrigger(bool_t enable)				{ adc.auto_trigger = enable; }
void ADC_vSetTriggerSource(ADC_TriggerSource_t s)	{ adc.trigger_source = s; }
void ADC_vSetConversionInt(bool_t enable)			{ adc.enable_int = enable; }

ADC_Reference_t ADC_vGetReference(void)				{ return adc.reference; }
ADC_Clock_t ADC_vGetClock(void)						{ return adc.clock; }
ADC_Channel_t ADC_vGetChannel(void)					{ return adc.channel; }
ADC_TriggerSource_t ADC_vGetTriggerSource(void)		{ return adc.trigger_source; }


/* ============================================================
 * ADC – Lifecycle
 * ============================================================ */
void ADC_Init(void) {
    u8_t admux_value  = 0;
    u8_t adcsra_value = 0;

    /* --- Reference selection (REFS1:REFS0) --- */
    switch (adc.reference) {
        case ADC_REF_AREF:
            /* 00 — external AREF pin */
            break;

        case ADC_REF_AVCC:
            /* 01 — AVCC with external cap on AREF */
            SET_BIT(admux_value, ADMUX_REFS0);
            break;

        case ADC_REF_INTERNAL_2V56:
            /* 11 — internal 2.56 V reference */
            SET_BIT(admux_value, ADMUX_REFS0);
            SET_BIT(admux_value, ADMUX_REFS1);
            break;
    }

    /* --- Result alignment --- */
    if (adc.left_adjust) 
	{
        SET_BIT(admux_value, ADMUX_ADLAR);
    }

    /* --- Channel selection (MUX4:0) --- */
    admux_value |= (adc.channel & 0x07);	// (MUX2:0) 
    ADMUX = admux_value;
	CLR_BIT(DDRA, adc.channel);		// set pin to input

    /* --- Auto trigger source (SFIOR bits 7:5) --- */
    if (adc.auto_trigger) {
        /* Clear the three ADTS bits without touching the rest of SFIOR */
		CLR_BIT(SFIOR, SFIOR_ADTS0);
		CLR_BIT(SFIOR, SFIOR_ADTS1);
		CLR_BIT(SFIOR, SFIOR_ADTS2);
		
        SFIOR |= (u8_t)((adc.trigger_source & 0x07) << SFIOR_ADTS0);
        SET_BIT(adcsra_value, ADCSRA_ADATE);
    } 

    /* --- Interrupt enable --- */
    if (adc.enable_int) {
        SET_BIT(adcsra_value, ADCSRA_ADIE);
    }

    /* --- Prescaler/Clock (ADPS2:0) --- */
    adcsra_value |= (adc.clock & 0x07);

    /* --- Write ADCSRA (ADEN/ADSC left clear for now) --- */
    ADCSRA = adcsra_value;

    /* --- Enable ADC --- */
    ADC_Enable();

    /* --- Prime the ADC with one dummy conversion --- */
    ADC_WaitRead();
}

void ADC_Enable(void) {
    SET_BIT(ADCSRA, ADCSRA_ADEN);
}

void ADC_Disable(void) {
    CLR_BIT(ADCSRA, ADCSRA_ADEN);
}

/* ============================================================
 * ADC – Runtime
 * ============================================================ */
void ADC_StartConversion(void) {
    SET_BIT(ADCSRA, ADCSRA_ADSC);
}

bool_t ADC_IsConversionComplete(void) {
    return (READ_BIT(ADCSRA, ADCSRA_ADSC) == 0) ? TRUE : FALSE;
}

u16_t ADC_Read(void) {
	u8_t low  = ADCL;
	u8_t high = ADCH;
	if (adc.left_adjust) {
		return ((u16_t)high << 2) | ((u16_t)low >> 6);
	}
	return ((u16_t)high << 8) | low;
}

u16_t ADC_WaitRead(void) {
    SET_BIT(ADCSRA, ADCSRA_ADSC);
    while (READ_BIT(ADCSRA, ADCSRA_ADSC));
    return ADC_Read();
}

u16_t ADC_ChannelSwitch(ADC_Channel_t ch) {
    /* Update ADMUX channel bits only (keep REFS + ADLAR) */
    ADMUX = (ADMUX & 0xF8) | (ch & 0x07);
	CLR_BIT(DDRA, ch);	// set pin to input
	adc.channel = ch;

    /* First conversion after channel change is on the old channel — discard */
    (void)ADC_WaitRead();

    /* Second conversion is on the new channel */
    return ADC_WaitRead();
}


/* ============================================================
 * ADC – Interrupts
 * ============================================================ */
void ADC_EnableInterrupt(void) {
    SET_BIT(ADCSRA, ADCSRA_ADIE);
    adc.enable_int = TRUE;
}

void ADC_DisableInterrupt(void) {
    CLR_BIT(ADCSRA, ADCSRA_ADIE);
    adc.enable_int = FALSE;
}

void ADC_ClearInterruptFlag() {
	SET_BIT(ADCSRA, ADCSRA_ADIF);
}