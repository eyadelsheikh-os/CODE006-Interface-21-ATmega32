/**
 * @file    main.c
 * @brief   LED Chaser with Toggle-on-Button-Release Application
 * @author  eyads
 * @date    2026-05-07
 *
 * @note    This application implements a bouncing LED chaser.
 *          Pressing and releasing a button toggles the LED pattern
 *          between two banks of LEDs (left and right).
 */

/* ============================================
 * User Defined Library Include
 * ============================================ */
#include "_ud_library.h"

/* ============================================
 *  AVR Library Includes
 * ============================================ */
#include <util/delay.h>
#include <avr/interrupt.h>

/* ============================================
 * Configuration Constants
 * ============================================ */
#define DELAY_MS	200		/**< Simple fps delay */
#define TIMEOUT	 	60000	/**< Timeout Guard */
#define SOUND		34892	/**< Speed of Sound */

/* 7-bit I2C addresses — all outside reserved ranges
 * (0x00-0x07 general call, 0x50-0x57 EEPROM, 0x78-0x7F reserved) */
#define ID_MMC0		0x0A		/* central's own address */
#define ID_SMC0		0x10

/* ============================================
 * Application Variables
 * ============================================ */
volatile u8_t distance_discrate[3] = {4, 5, 0};
volatile u16_t distance = 450;
volatile u16_t sound_speed = SOUND;

/* ============================================
 * Application Objects
 * ============================================ */

/* ===== Define 7seg Pins ===== */
SevenSeg_t seg = {
	.segments = {
		GPIO_PIN(PORT_A, PIN_0),  // A
		GPIO_PIN(PORT_A, PIN_1),  // B
		GPIO_PIN(PORT_A, PIN_2),  // C
		GPIO_PIN(PORT_A, PIN_3),  // D
		GPIO_PIN(PORT_A, PIN_4),  // E
		GPIO_PIN(PORT_A, PIN_5),  // F
		GPIO_PIN(PORT_A, PIN_6)   // G
	}
};

/* ===== Display Units Controller Pins ===== */
LED_t ctrl[3] = {
	{ .pin = GPIO_PIN(PORT_C, PIN_5), .active = LED_ACTIVE_LOW },
	{ .pin = GPIO_PIN(PORT_C, PIN_6), .active = LED_ACTIVE_LOW },
	{ .pin = GPIO_PIN(PORT_C, PIN_7), .active = LED_ACTIVE_LOW }
};

/* ===== Ultrasonic Trigger Pin ===== */
LED_t trig = { .pin = GPIO_PIN(PORT_D, PIN_5) };

/* ===== HW External interrupt Pin ===== */
Button_t hardware_int = { .pin = GPIO_PIN(PORT_D, PIN_2) };

/* ============================================
 * Application Functions
 * ============================================ */
static inline void UpdateSegs(void)
{
	static u8_t index = 0;
	static u8_t index_sub = 0;
	
	index_sub = index++;
	if (index > 2) index = 0;
	
	LED_Off(&ctrl[index_sub]);	// disable segment
	SEG_DisplayDigit(&seg, distance_discrate[index]);
	LED_On(&ctrl[index]);	// enable segment
}

static inline void UpdateDistanceDiscrate(void)
{
	distance_discrate[0] = distance/100;
	distance_discrate[1] = (distance/10) % 10;
	distance_discrate[2] = distance % 10;
}

static inline bool_t WaitCapture(void)
{
	u16_t guard = 0;
	while (!TC1_IsInputCapture())
	{
		 if (++guard >= TIMEOUT) return FALSE;
	}
	return TRUE;
}

static inline void TriggerSensor(void)
{
	/* Capturing the Rising Edge */
	TC1_InputEnable(IC1_MODE_RISING);
	TC1_ClearInterruptFlag(TC1_FLAG_CAPTURE);
		
	/* Trigger Start */
	LED_On(&trig);
	_delay_us(20);
	LED_Off(&trig);
}

static inline bool_t ReadSensor(void)
{
	static u16_t a, b;
	static u8_t flip_flop = 0;
	
	if (flip_flop) {
		u16_t high;
		b = TC1_GetCaptureValue();
	
		if (a < b) high = b - a;
		else high = 65536 - a + b;
		
        if (high < 50 || high > 26000) distance = 450;      /* out of range or invalid */
	    else distance = (u16_t)((float)sound_speed * high / 2.0f / ((float)F_CPU / 8.0f)  + 0.5f);
		
		flip_flop = 0;
	} else {
		a = TC1_GetCaptureValue();
		/* Capturing the FALLING Edge */
		TC1_InputEnable(IC1_MODE_FALLING);
		TC1_ClearInterruptFlag(TC1_FLAG_CAPTURE);
		flip_flop = 1;
	}
	return !flip_flop;
}

/*============================================
 * Master transfer
 * ============================================ */
static volatile bool_t req_distance = FALSE;
volatile u8_t tx_buf[2] = {0};

static inline void Xfer_Distance(void)
{
	u8_t buf[2] = {
		(u8_t)(distance >> 8),
		(u8_t)(distance)
	};
	I2C_SendAsync(ID_MMC0, (const u8_t *)buf, 2, NULL);
}

/* ============================================
 * Slave transfer — callbacks for receiving sound_speed
 * ============================================ */
volatile u8_t rx_buf[2] = {0};
volatile u8_t rx_ind    = 0;

void SlaveRxCallback(u8_t byte)
{
	if (rx_ind < 2) rx_buf[rx_ind++] = byte;
}

void SlaveStopCallback(void)
{
	if (rx_ind >= 2)
		sound_speed = (((u16_t)rx_buf[0]) << 8) + ((u16_t)rx_buf[1]);
	rx_ind = 0;
}

/* ============================================
 * Main Application
 * ============================================ */
int main(void) {
    /* ===== Initialize Display ===== */
    SEG_Init(&seg);
    
    /* ===== Initialize Controller Pins ===== */
    for (u8_t i = 0; i < 3; i++)
	{
		LED_Init(&ctrl[i]);
		LED_Off(&ctrl[i]);
	}
	
    /* ===== Initialize Ultrasonic Trigger Pin ===== */
	LED_Init(&trig);
	
    /* ===== Initialize HW External interrupt Pin ===== */
	BUTTON_Init(&hardware_int);
	
	/* ===== I2C Configuration ===== */
	I2C_vReset();
	I2C_vSetAddress(ID_SMC0);
	I2C_vSetInterrupt(TRUE);
	
	/* Initialize I2C */
	I2C_SetSlaveCallbacks(SlaveRxCallback, NULL, SlaveStopCallback);
	I2C_Init();
	
	/* ===== Timers Configuration ===== */
	
    /* fps timer */
    TC0_vReset();
    TC0_vSetMode(TC0_MODE_CTC);
    TC0_vSetClock(TC0_CLK_DIV1024);
    TC0_vSetCompareValue(37);	// 70 fps
    TC0_vSetCompareInt(TRUE);
	
    /* Event capture timer */
	TC1_vReset();
	TC1_vSetClock(TC1_CLK_DIV8);
	TC1_vSetNoiseCanceler(TRUE);
	TC1_vSetInputMode(IC1_MODE_RISING);
	TC1_vSetCaptureInt(TRUE);
	
	/* Initialize timers */
    TC0_Init();
    TC1_Init();

	/* ===== Interrupt Configuration ===== */
    INT0_Enable(INT_TRIGGER_FALLING_EDGE);
    INT0_ClearFlag();
	INT_GlobalEnable();
	
    /* ===== Main Loop ===== */
	TriggerSensor();
    while (TRUE)
	{
		if (req_distance && !I2C_IsBusy())
		{
			req_distance = FALSE;
			Xfer_Distance();
		}
	}
}

/* ============================================
 * Interrupts
 * ============================================ */

/* External Interrupts */
ISR(INT0_vect) // External interrupt 0
{
	req_distance = TRUE;
}

/* Timer Interrupts*/
ISR(TIMER1_CAPT_vect) // Timer1 event capture interrupt
{
	if (ReadSensor()) 
	{
		UpdateDistanceDiscrate();
		TriggerSensor();
	}
}

/* Timer Interrupts*/
ISR(TIMER0_COMP_vect) // Timer0 overflow interrupt
{
	UpdateSegs();
}

/* I2C Interrupts */
ISR(TWI_vect) // TWI event action
{
	I2C_OnEvent();
}
