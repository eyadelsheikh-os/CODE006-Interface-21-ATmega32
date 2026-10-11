/**
 * @file    mcal_timers.c
 * @brief   MCAL Timers/Counters Driver Implementation for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Provides low-level functions to configure and control
 *          the ATmega32 Timer/Counter peripherals (Timer0, Timer1, Timer2).
 */

#include "mcal_timers.h"
/* ============================================================
 * ============ TIMER 0 – 8-BIT COUNTER ================
 * ============================================================
 * Timer 0 – Private State
 * ============================================ */

/*
 * The struct definition is kept here (not in the header) to enforce
 * encapsulation – application code can only read/write through the
 * public API, never by touching the struct members directly.
 */
struct TC0_State_t {
	TC0_Mode_t   mode;
	TC0_Clock_t  clock;
	u8_t         compare;
	bool_t       enable_overflow_int;
	bool_t       enable_compare_int;
	OC0_Mode_t   output_mode;
};	

TC0_State_t timer0;


/* ============================================================
 * Timer 0 – Configuration Setters
 * ============================================================ */
void TC0_vReset(void) {
	timer0.mode					= TC0_MODE_NORMAL;
	timer0.clock				= TC0_CLK_STOP;
	timer0.compare				= 0;
	timer0.enable_overflow_int	= FALSE;
	timer0.enable_compare_int	= FALSE;
	timer0.output_mode			= OC0_MODE_DISCONNECTED;
}

void TC0_vSetMode(TC0_Mode_t mode)			{ timer0.mode = mode; }
void TC0_vSetClock(TC0_Clock_t clock)		{ timer0.clock = clock; }
void TC0_vSetCompareValue(u8_t compare)		{ timer0.compare = compare; }
void TC0_vSetOverflowInt(bool_t enable)		{ timer0.enable_overflow_int = enable; }
void TC0_vSetCompareInt(bool_t enable)		{ timer0.enable_compare_int = enable; }
void TC0_vSetOutputMode(OC0_Mode_t mode)	{ timer0.output_mode = mode; }

TC0_Mode_t TC0_vGetMode(void)			{ return timer0.mode; }
TC0_Clock_t TC0_vGetClock(void)			{ return timer0.clock; }
OC0_Mode_t TC0_vGetOutputMode(void)		{ return timer0.output_mode; }

/* ============================================================
 * Timer 0 – Lifecycle
 * ============================================================ */
void TC0_Init(void) {
    u8_t tccr0_value = 0;
	
    /* --- Waveform Generation Mode (WGM01:WGM00) --- */
	switch (timer0.mode) {
		case TC0_MODE_NORMAL:
		/* WGM01=0, WGM00=0 */
		break;

		case TC0_MODE_PHASE_PWM:
		/* WGM01=0, WGM00=1 */
		SET_BIT(tccr0_value, TCCR0_WGM00);
		break;

		case TC0_MODE_CTC:
		/* WGM01=1, WGM00=0 */
		SET_BIT(tccr0_value, TCCR0_WGM01);
		break;

		case TC0_MODE_FAST_PWM:
		/* WGM01=1, WGM00=1 */
		SET_BIT(tccr0_value, TCCR0_WGM00);
		SET_BIT(tccr0_value, TCCR0_WGM01);
		break;
	}

    /* --- Compare Output Mode (COM01:COM00) --- */
    tccr0_value |= ((timer0.output_mode & 0x03) << TCCR0_COM00);
	
	if (timer0.output_mode != OC0_MODE_DISCONNECTED)
	{
		if (timer0.mode == TC0_MODE_NORMAL || timer0.mode == TC0_MODE_CTC)
		{
			SET_BIT(DDRB, PIN_3);	// set pin to output
		}
		else if (timer0.output_mode != OC0_MODE_TOGGLE)
		{
			SET_BIT(DDRB, PIN_3);	// set pin to output
		}
	}
	
	/* --- Write final TCCR0 value */
    TCCR0 = tccr0_value;

	/* --- Output Compare value --- */
	OCR0 = timer0.compare;

	/* --- Interrupt enables --- */
	if (timer0.enable_overflow_int) SET_BIT(TIMSK, TIMSK_TOIE0);
	else							CLR_BIT(TIMSK, TIMSK_TOIE0);

	if (timer0.enable_compare_int)	SET_BIT(TIMSK, TIMSK_OCIE0);
	else							CLR_BIT(TIMSK, TIMSK_OCIE0);
	
	/* --- Clock select (prescaler) --- */
	TC0_Enable(timer0.clock);
}

void TC0_Enable(TC0_Clock_t clock) {
    /* Preserve WGM and COM bits, update only CS02:CS00 */
    u8_t tccr0_value = TCCR0;
	
	CLR_BIT(tccr0_value, TCCR0_CS00);
	CLR_BIT(tccr0_value, TCCR0_CS01);
	CLR_BIT(tccr0_value, TCCR0_CS02);
	
    tccr0_value |= (clock & 0x07);
    TCCR0 = tccr0_value;

    timer0.clock = clock;
}

void TC0_Disable(void) {
    u8_t tccr0_value = TCCR0;
	
    CLR_BIT(tccr0_value, TCCR0_CS02);
    CLR_BIT(tccr0_value, TCCR0_CS01);
    CLR_BIT(tccr0_value, TCCR0_CS00);
    
	TCCR0 = tccr0_value;

    timer0.clock = TC0_CLK_STOP;
}

void TC0_OutputEnable(OC0_Mode_t mode) {
    u8_t tccr0_value = TCCR0;
    
    CLR_BIT(tccr0_value, TCCR0_COM00);
    CLR_BIT(tccr0_value, TCCR0_COM01);
    
    tccr0_value |= ((mode & 0x03) << TCCR0_COM00);
    
    if (mode != OC0_MODE_DISCONNECTED)
    {
	    if (timer0.mode == TC0_MODE_NORMAL || timer0.mode == TC0_MODE_CTC)
	    {
		    SET_BIT(DDRB, PIN_3);	// set pin to output
	    }
	    else if (mode != OC0_MODE_TOGGLE)
	    {
			SET_BIT(DDRB, PIN_3);	// set pin to output
	    }
    }
	
    TCCR0 = tccr0_value;

    timer0.output_mode = mode;
}

void TC0_OutputDisable(void) {
	u8_t tccr0_value = TCCR0;
	
    CLR_BIT(tccr0_value, TCCR0_COM00);
    CLR_BIT(tccr0_value, TCCR0_COM01);
    
	TCCR0 = tccr0_value;

    timer0.output_mode = OC0_MODE_DISCONNECTED;
}


/* ============================================================
 * Timer 0 – Runtime
 * ============================================================ */
void TC0_SetCounterValue(u8_t value) {
    TCNT0 = value;
}

u8_t TC0_GetCounterValue(void) {
    return TCNT0;
}

void TC0_SetCompareValue(u8_t value) {
    OCR0 = value;
    timer0.compare = value;
}

u8_t TC0_GetCompareValue(void) {
    return OCR0;
}

void TC0_EnableInterrupt(TC0_Flag_t flag) {
	switch (flag) 
	{
		case TC0_FLAG_OVERFLOW:
		    SET_BIT(TIMSK, TIMSK_TOIE0);
			timer0.enable_overflow_int = TRUE;
			break;
			
		case TC0_FLAG_COMPARE:
		    SET_BIT(TIMSK, TIMSK_OCIE0);
			timer0.enable_compare_int = TRUE;
			break;
	}
}

void TC0_DisableInterrupt(TC0_Flag_t flag) {
	switch (flag) 
	{
		case TC0_FLAG_OVERFLOW:
			CLR_BIT(TIMSK, TIMSK_TOIE0);
			timer0.enable_overflow_int = FALSE;
			break;
			
		case TC0_FLAG_COMPARE:
			CLR_BIT(TIMSK, TIMSK_OCIE0);
			timer0.enable_compare_int = FALSE;
			break;
	}
}

void TC0_ClearInterruptFlag(TC0_Flag_t flag) {
    switch (flag) 
	{
        case TC0_FLAG_OVERFLOW:	SET_BIT(TIFR, TIFR_TOV0);	break;
        case TC0_FLAG_COMPARE:	SET_BIT(TIFR, TIFR_OCF0);	break;
    }
}


/* ============================================================
 * ================ TIMER 1 – 16-BIT COUNTER ==================
 * ============================================================
 * Timer 1 – Private State
 * ============================================================ */
struct TC1_State_t {
    TC1_Mode_t          mode;
    TC1_Clock_t         clock;
    u16_t               top;
    u16_t               compare_a;
    u16_t               compare_b;
	OC1_Mode_t			output_a_mode;
	OC1_Mode_t			output_b_mode;
	IC1_Mode_t			input_mode;
	bool_t              enable_overflow_int;
	bool_t              enable_compare_a_int;
	bool_t              enable_compare_b_int;
	bool_t              enable_capture_int;
	bool_t              noise_canceler;
};

TC1_State_t timer1;


/* ============================================================
 * Timer 1 – Configuration Setters
 * ============================================================ */
void TC1_vReset(void) {
    timer1.mode                 = TC1_MODE_NORMAL;
    timer1.clock                = TC1_CLK_STOP;
    timer1.top                  = 0xFFFF;
    timer1.compare_a            = 0;
    timer1.compare_b            = 0;
    timer1.enable_overflow_int  = FALSE;
    timer1.enable_compare_a_int = FALSE;
    timer1.enable_compare_b_int = FALSE;
    timer1.enable_capture_int   = FALSE;
    timer1.output_a_mode		= OC1_MODE_DISCONNECTED;
    timer1.output_b_mode		= OC1_MODE_DISCONNECTED;
    timer1.input_mode			= IC1_MODE_NORMAL;
    timer1.noise_canceler       = FALSE;
}

void TC1_vSetMode(TC1_Mode_t mode)					{ timer1.mode = mode; }
void TC1_vSetClock(TC1_Clock_t clock)				{ timer1.clock = clock; }
void TC1_vSetTopValue(u16_t top)					{ timer1.top = top; }
void TC1_vSetCompareAValue(u16_t value)				{ timer1.compare_a = value; }
void TC1_vSetCompareBValue(u16_t value)				{ timer1.compare_b = value; }
void TC1_vSetOverflowInt(bool_t enable)				{ timer1.enable_overflow_int = enable; }
void TC1_vSetCompareAInt(bool_t enable)				{ timer1.enable_compare_a_int = enable; }
void TC1_vSetCompareBInt(bool_t enable)				{ timer1.enable_compare_b_int = enable; }
void TC1_vSetCaptureInt(bool_t enable)				{ timer1.enable_capture_int = enable; }
void TC1_vSetOutputAMode(OC1_Mode_t mode)			{ timer1.output_a_mode = mode; }
void TC1_vSetOutputBMode(OC1_Mode_t mode)			{ timer1.output_b_mode = mode; }
void TC1_vSetInputMode(IC1_Mode_t mode)				{ timer1.input_mode = mode; }
void TC1_vSetNoiseCanceler(bool_t enable)			{ timer1.noise_canceler = enable; }

TC1_Mode_t TC1_vGetMode(void)					{ return timer1.mode; }
TC1_Clock_t TC1_vGetClock(void)					{ return timer1.clock; }
u16_t TC1_vGetTopValue(void)					{ return timer1.top; }
u16_t TC1_vGetCompareAValue(void)				{ return timer1.compare_a; }
u16_t TC1_vGetCompareBValue(void)				{ return timer1.compare_b; }
OC1_Mode_t TC1_vGetOutputAMode(void)			{ return timer1.output_a_mode; }
OC1_Mode_t TC1_vGetOutputBMode(void)			{ return timer1.output_b_mode; }
IC1_Mode_t TC1_vGetInputMode(void)				{ return timer1.input_mode; }


/* ============================================================
 * Timer 1 – Lifecycle
 * ============================================================ */
void TC1_Init(void) {
    u8_t tccr1a_value = 0;
    u8_t tccr1b_value = 0;

    /* --- Waveform Generation Mode (WGM13:WGM10) ---
     *
     *  Mode                          WGM13 WGM12 WGM11 WGM10
     *  ----------------------------------------------------------
     *  Normal                          0     0     0     0
     *  Phase-correct PWM  8-bit        0     0     0     1
     *  Phase-correct PWM  9-bit        0     0     1     0
     *  Phase-correct PWM 10-bit        0     0     1     1
     *  CTC (TOP = OCR1A)               0     1     0     0
     *  Fast PWM  8-bit                 0     1     0     1
     *  Fast PWM  9-bit                 0     1     1     0
     *  Fast PWM 10-bit                 0     1     1     1
     *  Phase & freq correct (ICR1)     1     0     0     0
     *  Phase & freq correct (OCR1A)    1     0     0     1
     *  Phase-correct (ICR1)            1     0     1     0
     *  Phase-correct (OCR1A)           1     0     1     1
     *  CTC (TOP = ICR1)                1     1     0     0
     *  Fast PWM (TOP = ICR1)           1     1     1     0
     *  Fast PWM (TOP = OCR1A)          1     1     1     1
     */
    switch (timer1.mode) {
        case TC1_MODE_NORMAL:
            break;

        case TC1_MODE_PHASE_PWM_8BIT:
            SET_BIT(tccr1a_value, TCCR1A_WGM10);
            break;

        case TC1_MODE_PHASE_PWM_9BIT:
            SET_BIT(tccr1a_value, TCCR1A_WGM11);
            break;

        case TC1_MODE_PHASE_PWM_10BIT:
            SET_BIT(tccr1a_value, TCCR1A_WGM10);
            SET_BIT(tccr1a_value, TCCR1A_WGM11);
            break;

        case TC1_MODE_CTC_OCR1A:
            SET_BIT(tccr1b_value, TCCR1B_WGM12);
            break;

        case TC1_MODE_FAST_PWM_8BIT:
            SET_BIT(tccr1a_value, TCCR1A_WGM10);
            SET_BIT(tccr1b_value, TCCR1B_WGM12);
            break;

        case TC1_MODE_FAST_PWM_9BIT:
            SET_BIT(tccr1a_value, TCCR1A_WGM11);
            SET_BIT(tccr1b_value, TCCR1B_WGM12);
            break;

        case TC1_MODE_FAST_PWM_10BIT:
            SET_BIT(tccr1a_value, TCCR1A_WGM10);
            SET_BIT(tccr1a_value, TCCR1A_WGM11);
            SET_BIT(tccr1b_value, TCCR1B_WGM12);
            break;

        case TC1_MODE_FREQ_PWM_ICR1:
            SET_BIT(tccr1b_value, TCCR1B_WGM13);
            break;

        case TC1_MODE_FREQ_PWM_OCR1A:
            SET_BIT(tccr1a_value, TCCR1A_WGM10);
            SET_BIT(tccr1b_value, TCCR1B_WGM13);
            break;

        case TC1_MODE_PHASE_PWM_ICR1:
            SET_BIT(tccr1a_value, TCCR1A_WGM11);
            SET_BIT(tccr1b_value, TCCR1B_WGM13);
            break;

        case TC1_MODE_PHASE_PWM_OCR1A:
            SET_BIT(tccr1a_value, TCCR1A_WGM10);
            SET_BIT(tccr1a_value, TCCR1A_WGM11);
            SET_BIT(tccr1b_value, TCCR1B_WGM13);
            break;

        case TC1_MODE_CTC_ICR1:
            SET_BIT(tccr1b_value, TCCR1B_WGM12);
            SET_BIT(tccr1b_value, TCCR1B_WGM13);
            break;

        case TC1_MODE_FAST_PWM_ICR1:
            SET_BIT(tccr1a_value, TCCR1A_WGM11);
            SET_BIT(tccr1b_value, TCCR1B_WGM12);
            SET_BIT(tccr1b_value, TCCR1B_WGM13);
            break;

        case TC1_MODE_FAST_PWM_OCR1A:
            SET_BIT(tccr1a_value, TCCR1A_WGM10);
            SET_BIT(tccr1a_value, TCCR1A_WGM11);
            SET_BIT(tccr1b_value, TCCR1B_WGM12);
            SET_BIT(tccr1b_value, TCCR1B_WGM13);
            break;
    }

    /* --- Input Capture configuration --- */
    if (timer1.input_mode == IC1_MODE_RISING || timer1.input_mode == IC1_MODE_FALLING)
    {
	    if (timer1.input_mode == IC1_MODE_RISING)	SET_BIT(tccr1b_value, TCCR1B_ICES1);
	   	
		CLR_BIT(DDRD, PIN_6);	// set pin to input
    }
	
    if (timer1.noise_canceler)	SET_BIT(tccr1b_value, TCCR1B_ICNC1);
	
    /* --- Compare Output Mode (COM01:COM00) --- */
    tccr1a_value |= ((timer1.output_a_mode & 0x0F) << TCCR1A_COM1A0);
    tccr1a_value |= ((timer1.output_b_mode & 0x0F) << TCCR1A_COM1B0);
    
    if (timer1.output_a_mode != OC1_MODE_DISCONNECTED)
	{
		if (timer1.mode == TC1_MODE_NORMAL || timer1.mode == TC1_MODE_CTC_OCR1A 
			|| timer1.mode == TC1_MODE_CTC_ICR1 || timer1.mode == TC1_MODE_FAST_PWM_OCR1A)
		{
			SET_BIT(DDRD, PIN_5);	// set pin to output
		}
		else if (timer1.output_a_mode != OC1_MODE_PWM_TOGGLE)
		{
			SET_BIT(DDRD, PIN_5);	// set pin to output
		}
	}
	
    if (timer1.output_b_mode != OC1_MODE_DISCONNECTED)
    {
	    if (timer1.mode == TC1_MODE_NORMAL || timer1.mode == TC1_MODE_CTC_OCR1A || timer1.mode == TC1_MODE_CTC_ICR1)
	    {
		    SET_BIT(DDRD, PIN_4);	// set pin to output
	    }
	    else if (timer1.output_b_mode != OC1_MODE_PWM_TOGGLE)
	    {
		    SET_BIT(DDRD, PIN_4);	// set pin to output
	    }
    }
	
    /* --- Write control registers --- */
    TCCR1A = tccr1a_value;
    TCCR1B = tccr1b_value;

    /* --- Write TOP / compare registers --- */
    OCR1A = timer1.compare_a;
    OCR1B = timer1.compare_b;
    ICR1  = timer1.top;

    /* --- Interrupt enables --- */
    if (timer1.enable_overflow_int)  SET_BIT(TIMSK, TIMSK_TOIE1);
    else                             CLR_BIT(TIMSK, TIMSK_TOIE1);

    if (timer1.enable_compare_a_int) SET_BIT(TIMSK, TIMSK_OCIE1A);
    else                             CLR_BIT(TIMSK, TIMSK_OCIE1A);

    if (timer1.enable_compare_b_int) SET_BIT(TIMSK, TIMSK_OCIE1B);
    else                             CLR_BIT(TIMSK, TIMSK_OCIE1B);

    if (timer1.enable_capture_int)   SET_BIT(TIMSK, TIMSK_TICIE1);
    else                             CLR_BIT(TIMSK, TIMSK_TICIE1);

    /* --- Clock select --- */
    TC1_Enable(timer1.clock);
}

void TC1_Enable(TC1_Clock_t clock) {
    /* Preserve WGM, ICES, ICNC bits; update only CS12:CS10 */
    u8_t tccr1b_value = TCCR1B;

    CLR_BIT(tccr1b_value, TCCR1B_CS10);
    CLR_BIT(tccr1b_value, TCCR1B_CS11);
    CLR_BIT(tccr1b_value, TCCR1B_CS12);

    tccr1b_value |= (clock & 0x07);
    TCCR1B = tccr1b_value;

    timer1.clock = clock;
}

void TC1_Disable(void) {
    u8_t tccr1b_value = TCCR1B;

    CLR_BIT(tccr1b_value, TCCR1B_CS12);
    CLR_BIT(tccr1b_value, TCCR1B_CS11);
    CLR_BIT(tccr1b_value, TCCR1B_CS10);

    TCCR1B = tccr1b_value;

    timer1.clock = TC1_CLK_STOP;
}
	
void TC1_OutputAEnable(OC1_Mode_t mode) {
	u8_t tccr1a_value = TCCR1A;
	
	CLR_BIT(tccr1a_value, TCCR1A_COM1A0);
	CLR_BIT(tccr1a_value, TCCR1A_COM1A1);
	
	tccr1a_value |= ((mode & 0x0F) << TCCR1A_COM1A0);
	
    if (mode != OC1_MODE_DISCONNECTED)
	{
		if (timer1.mode == TC1_MODE_NORMAL || timer1.mode == TC1_MODE_CTC_OCR1A 
			|| timer1.mode == TC1_MODE_CTC_ICR1 || timer1.mode == TC1_MODE_FAST_PWM_OCR1A)
		{
			SET_BIT(DDRD, PIN_5);	// set pin to output
		}
		else if (mode != OC1_MODE_PWM_TOGGLE)
		{
			SET_BIT(DDRD, PIN_5);	// set pin to output
		}
	}
    
	TCCR1A = tccr1a_value;

	timer1.output_a_mode = mode;
}

void TC1_OutputADisable(void) {
	u8_t tccr1a_value = TCCR1A;
	
	CLR_BIT(tccr1a_value, TCCR1A_COM1A0);
	CLR_BIT(tccr1a_value, TCCR1A_COM1A1);
	
	TCCR1A = tccr1a_value;

	CLR_BIT(PORTD, PIN_5);

	timer1.output_a_mode = OC1_MODE_DISCONNECTED;
}

void TC1_OutputBEnable(OC1_Mode_t mode) {
	u8_t tccr1a_value = TCCR1A;
	
	CLR_BIT(tccr1a_value, TCCR1A_COM1B0);
	CLR_BIT(tccr1a_value, TCCR1A_COM1B1);
	
	tccr1a_value |= ((mode & 0x0F) << TCCR1A_COM1B0);
	
	if (mode != OC1_MODE_DISCONNECTED)
	{
		if (timer1.mode == TC1_MODE_NORMAL || timer1.mode == TC1_MODE_CTC_OCR1A || timer1.mode == TC1_MODE_CTC_ICR1)
		{
			SET_BIT(DDRD, PIN_4);	// set pin to output
		}
		else if (mode != OC1_MODE_PWM_TOGGLE)
		{
			SET_BIT(DDRD, PIN_4);	// set pin to output
		}
	}
	
	TCCR1A = tccr1a_value;

	timer1.output_b_mode = mode;
}

void TC1_OutputBDisable(void) {
	u8_t tccr1a_value = TCCR1A;
	
	CLR_BIT(tccr1a_value, TCCR1A_COM1B0);
	CLR_BIT(tccr1a_value, TCCR1A_COM1B1);
	
	TCCR1A = tccr1a_value;
	CLR_BIT(PORTD, PIN_4);

	timer1.output_b_mode = OC1_MODE_DISCONNECTED;
}

void TC1_InputEnable(IC1_Mode_t mode) {
	u8_t tccr1b_value = TCCR1B;
	
    if (mode == IC1_MODE_RISING || mode == IC1_MODE_FALLING)
    {
	    if (mode == IC1_MODE_RISING)	SET_BIT(tccr1b_value, TCCR1B_ICES1);
	    else							CLR_BIT(tccr1b_value, TCCR1B_ICES1);
	    
	    CLR_BIT(DDRD, PIN_6);	// set pin to input
    }

	TCCR1B = tccr1b_value;

	timer1.input_mode = mode;
}

void TC1_InputDisable(void) {
	
	SET_BIT(DDRD, PIN_6);	// set pin to output
	
	timer1.input_mode = IC1_MODE_NORMAL;
}


/* ============================================================
 * Timer 1 – Runtime
 * ============================================================ */
void TC1_SetCounterValue(u16_t value) {
    TCNT1 = value;
}

u16_t TC1_GetCounterValue(void) {
    return TCNT1;
}

void TC1_SetCompareAValue(u16_t value) {
	OCR1A = value;
    timer1.compare_a = value;
}

u16_t TC1_GetCompareAValue(void) {
    return OCR1A;
}

void TC1_SetCompareBValue(u16_t value) {
	OCR1B = value;
    timer1.compare_b = value;
}

u16_t TC1_GetCompareBValue(void) {
    return OCR1B;

}

void TC1_SetTopValue(u16_t top) {
    ICR1 = top;
    timer1.top = top;
}

u16_t TC1_GetCaptureValue(void) {
    return ICR1;
}

void TC1_EnableInterrupt(TC1_Flag_t flag) {
    switch (flag) {
        case TC1_FLAG_OVERFLOW:
            SET_BIT(TIMSK, TIMSK_TOIE1);
            timer1.enable_overflow_int = TRUE;
            break;

        case TC1_FLAG_COMPARE_A:
            SET_BIT(TIMSK, TIMSK_OCIE1A);
            timer1.enable_compare_a_int = TRUE;
            break;

        case TC1_FLAG_COMPARE_B:
            SET_BIT(TIMSK, TIMSK_OCIE1B);
            timer1.enable_compare_b_int = TRUE;
            break;

        case TC1_FLAG_CAPTURE:
            SET_BIT(TIMSK, TIMSK_TICIE1);
            timer1.enable_capture_int = TRUE;
            break;
    }
}

void TC1_DisableInterrupt(TC1_Flag_t flag) {
    switch (flag) {
        case TC1_FLAG_OVERFLOW:
            CLR_BIT(TIMSK, TIMSK_TOIE1);
            timer1.enable_overflow_int = FALSE;
            break;

        case TC1_FLAG_COMPARE_A:
            CLR_BIT(TIMSK, TIMSK_OCIE1A);
            timer1.enable_compare_a_int = FALSE;
            break;

        case TC1_FLAG_COMPARE_B:
            CLR_BIT(TIMSK, TIMSK_OCIE1B);
            timer1.enable_compare_b_int = FALSE;
            break;

        case TC1_FLAG_CAPTURE:
            CLR_BIT(TIMSK, TIMSK_TICIE1);
            timer1.enable_capture_int = FALSE;
            break;
    }
}

void TC1_WaitInterruptFlag(TC1_Flag_t flag) {
	switch (flag) {
		case TC1_FLAG_CAPTURE:   while (!READ_BIT(TIFR, TIFR_ICF1));  break;
		case TC1_FLAG_OVERFLOW:	 while (!READ_BIT(TIFR, TIFR_TOV1));  break;
		case TC1_FLAG_COMPARE_A: while (!READ_BIT(TIFR, TIFR_OCF1A)); break;
		case TC1_FLAG_COMPARE_B: while (!READ_BIT(TIFR, TIFR_OCF1B)); break;
	}
}

void TC1_ClearInterruptFlag(TC1_Flag_t flag) {
    switch (flag) {
        case TC1_FLAG_CAPTURE:   SET_BIT(TIFR, TIFR_ICF1);  break;
        case TC1_FLAG_OVERFLOW:  SET_BIT(TIFR, TIFR_TOV1);  break;
        case TC1_FLAG_COMPARE_A: SET_BIT(TIFR, TIFR_OCF1A); break;
        case TC1_FLAG_COMPARE_B: SET_BIT(TIFR, TIFR_OCF1B); break;
    }
}

bool_t TC1_IsInputCapture(void) {
	return READ_BIT(TIFR, TIFR_ICF1) ? TRUE : FALSE;
}


/* ============================================================
 * ============ TIMER 2 – 8-BIT COUNTER ================
 * ============================================================
 * Timer 2 – Private State
 * ============================================ */
struct TC2_State_t {
    TC2_Mode_t   mode;
    TC2_Clock_t  clock;
    u8_t         compare;
    bool_t       enable_overflow_int;
    bool_t       enable_compare_int;
	OC2_Mode_t   output_mode;
};

TC2_State_t timer2;


/* ============================================================
 * Timer 2 – Configuration Setters
 * ============================================================ */
void TC2_vReset(void) {
    timer2.mode					= TC2_MODE_NORMAL;
    timer2.clock				= TC2_CLK_STOP;
    timer2.compare				= 0;
    timer2.enable_overflow_int	= FALSE;
    timer2.enable_compare_int	= FALSE;
	timer2.output_mode			= OC2_MODE_DISCONNECTED;
}

void TC2_vSetMode(TC2_Mode_t mode)			{ timer2.mode = mode; }
void TC2_vSetClock(TC2_Clock_t clock)		{ timer2.clock = clock; }
void TC2_vSetCompareValue(u8_t compare)		{ timer2.compare = compare; }
void TC2_vSetOverflowInt(bool_t enable)		{ timer2.enable_overflow_int = enable; }
void TC2_vSetCompareInt(bool_t enable)		{ timer2.enable_compare_int = enable; }
void TC2_vSetOutputMode(OC2_Mode_t mode)	{ timer2.output_mode = mode; }

TC2_Mode_t TC2_vGetMode(void)			{ return timer2.mode; }
TC2_Clock_t TC2_vGetClock(void)			{ return timer2.clock; }
OC2_Mode_t TC2_vGetOutputMode(void)		{ return timer2.output_mode; }
	
	
/* ============================================================
 * Timer 2 – Lifecycle
 * ============================================================ */
void TC2_Init(void) {
    u8_t tccr2_value = 0;

    /* --- Waveform Generation Mode (WGM21:WGM20) --- */
    switch (timer2.mode) {
        case TC2_MODE_NORMAL:
            /* WGM21=0, WGM20=0 */
            break;

        case TC2_MODE_PHASE_PWM:
            /* WGM21=0, WGM20=1 */
            SET_BIT(tccr2_value, TCCR2_WGM20);
            break;

        case TC2_MODE_CTC:
            /* WGM21=1, WGM20=0 */
            SET_BIT(tccr2_value, TCCR2_WGM21);
            break;

        case TC2_MODE_FAST_PWM:
            /* WGM21=1, WGM20=1 */
            SET_BIT(tccr2_value, TCCR2_WGM20);
            SET_BIT(tccr2_value, TCCR2_WGM21);
            break;
    }
	
    /* --- Compare Output Mode (COM01:COM00) --- */
    tccr2_value |= ((timer2.output_mode & 0x03) << TCCR2_COM20);
    
	if (timer2.output_mode != OC2_MODE_DISCONNECTED)
	{
		if (timer2.mode == TC2_MODE_NORMAL || timer2.mode == TC2_MODE_CTC)
		{
			SET_BIT(DDRD, PIN_7);	// set pin to output
		}
		else if (timer2.output_mode != OC2_MODE_TOGGLE)
		{
			SET_BIT(DDRD, PIN_7);	// set pin to output
		}
	}
	
    /* --- Write final TCCR2 value --- */
    TCCR2 = tccr2_value;

    /* --- Output Compare value --- */
    OCR2 = timer2.compare;

    /* --- Interrupt enables --- */
    if (timer2.enable_overflow_int) SET_BIT(TIMSK, TIMSK_TOIE2);
    else                            CLR_BIT(TIMSK, TIMSK_TOIE2);

    if (timer2.enable_compare_int)  SET_BIT(TIMSK, TIMSK_OCIE2);
    else                            CLR_BIT(TIMSK, TIMSK_OCIE2);

    /* --- Clock select (prescaler) --- */
    TC2_Enable(timer2.clock);
}

void TC2_Enable(TC2_Clock_t clock) {
    /* Preserve WGM and COM bits, update only CS22:CS20 */
    u8_t tccr2_value = TCCR2;

    CLR_BIT(tccr2_value, TCCR2_CS20);
    CLR_BIT(tccr2_value, TCCR2_CS21);
    CLR_BIT(tccr2_value, TCCR2_CS22);

    tccr2_value |= (clock & 0x07);
    TCCR2 = tccr2_value;

    timer2.clock = clock;
}

void TC2_Disable(void) {
    u8_t tccr2_value = TCCR2;

    CLR_BIT(tccr2_value, TCCR2_CS22);
    CLR_BIT(tccr2_value, TCCR2_CS21);
    CLR_BIT(tccr2_value, TCCR2_CS20);

    TCCR2 = tccr2_value;

    timer2.clock = TC2_CLK_STOP;
}

void TC2_OutputEnable(OC2_Mode_t mode) {
	u8_t tccr2_value = TCCR2;
	
	CLR_BIT(tccr2_value, TCCR2_COM20);
	CLR_BIT(tccr2_value, TCCR2_COM21);
	
	tccr2_value |= ((mode & 0x03) << TCCR2_COM20);
	
	if (mode != OC2_MODE_DISCONNECTED)
    {
		if (timer2.mode == TC2_MODE_NORMAL || timer2.mode == TC2_MODE_CTC)
		{
			SET_BIT(DDRD, PIN_7);	// set pin to output
		}
	    else if (mode != OC2_MODE_TOGGLE)
		{
			SET_BIT(DDRD, PIN_7);	// set pin to output
		}
    }
    
	TCCR2 = tccr2_value;

	timer2.output_mode = mode;
}

void TC2_OutputDisable(void) {
	u8_t tccr2_value = TCCR2;
	
	CLR_BIT(tccr2_value, TCCR2_COM20);
	CLR_BIT(tccr2_value, TCCR2_COM21);
	
	TCCR2 = tccr2_value;

	timer2.output_mode = OC2_MODE_DISCONNECTED;
}


/* ============================================================
 * Timer 2 – Runtime
 * ============================================================ */
void TC2_SetCounterValue(u8_t value) {
    TCNT2 = value;
}

u8_t TC2_GetCounterValue(void) {
    return TCNT2;
}

void TC2_SetCompareValue(u8_t value) {
    OCR2 = value;
    timer2.compare = value;
}

u8_t TC2_GetCompareValue(void) {
    return OCR2;
}

void TC2_EnableInterrupt(TC2_Flag_t flag) {
    switch (flag) {
        case TC2_FLAG_OVERFLOW:
            SET_BIT(TIMSK, TIMSK_TOIE2);
            timer2.enable_overflow_int = TRUE;
            break;

        case TC2_FLAG_COMPARE:
            SET_BIT(TIMSK, TIMSK_OCIE2);
            timer2.enable_compare_int = TRUE;
            break;
    }
}

void TC2_DisableInterrupt(TC2_Flag_t flag) {
    switch (flag) {
        case TC2_FLAG_OVERFLOW:
            CLR_BIT(TIMSK, TIMSK_TOIE2);
            timer2.enable_overflow_int = FALSE;
            break;

        case TC2_FLAG_COMPARE:
            CLR_BIT(TIMSK, TIMSK_OCIE2);
            timer2.enable_compare_int = FALSE;
            break;
    }
}

void TC2_ClearInterruptFlag(TC2_Flag_t flag) {
    switch (flag) {
        case TC2_FLAG_OVERFLOW:	SET_BIT(TIFR, TIFR_TOV2);	break;
        case TC2_FLAG_COMPARE:	SET_BIT(TIFR, TIFR_OCF2);	break;
    }
}

