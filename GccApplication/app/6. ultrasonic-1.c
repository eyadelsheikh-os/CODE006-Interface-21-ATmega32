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
#define SOUND	34892	/**< Speed of Sound */

/* 7-bit I2C addresses — all outside reserved ranges
 * (0x00-0x07 general call, 0x50-0x57 EEPROM, 0x78-0x7F reserved) */
#define ID_MMC0		0x0A		/* central's own address */
#define ID_SMC0		0x10

#define MIN_TEMPERATURE 15.0
#define MAX_TEMPERATURE 35.0
#define DIF_TEMPERATURE 20.0
#define DATA_SIZE 256UL

static const u16_t sound_speed_lut[256] = {
	34017,     34022,     34027,     34031,     34036,     34041,     34045,     34050,
	34054,     34059,     34064,     34068,     34073,     34078,     34082,     34087,
	34091,     34096,     34101,     34105,     34110,     34115,     34119,     34124,
	34128,     34133,     34138,     34142,     34147,     34152,     34156,     34161,
	34165,     34170,     34175,     34179,     34184,     34188,     34193,     34198,
	34202,     34207,     34211,     34216,     34221,     34225,     34230,     34234,
	34239,     34244,     34248,     34253,     34257,     34262,     34267,     34271,
	34276,     34280,     34285,     34290,     34294,     34299,     34303,     34308,
	34313,     34317,     34322,     34326,     34331,     34336,     34340,     34345,
	34349,     34354,     34359,     34363,     34366,     34372,     34377,     34381,
	34386,     34391,     34395,     34400,     34404,     34409,     34414,     34418,
	34423,     34427,     34432,     34436,     34441,     34446,     34450,     34455,
	34459,     34464,     34468,     34473,     34476,     34482,     34487,     34491,
	34496,     34500,     34505,     34510,     34514,     34519,     34523,     34528,
	34532,     34537,     34541,     34546,     34551,     34555,     34560,     34564,
	34569,     34573,     34578,     34583,     34587,     34592,     34596,     34601,
	34605,     34610,     34614,     34619,     34623,     34628,     34633,     34637,
	34642,     34646,     34651,     34655,     34660,     34664,     34669,     34674,
	34678,     34683,     34687,     34692,     34696,     34701,     34705,     34710,
	34714,     34719,     34723,     34728,     34733,     34737,     34742,     34746,
	34751,     34755,     34760,     34764,     34769,     34773,     34778,     34782,
	34787,     34791,     34796,     34801,     34805,     34810,     34814,     34819,
	34823,     34828,     34832,     34837,     34841,     34846,     34850,     34855,
	34859,     34864,     34868,     34873,     34877,     34882,     34886,     34891,
	34895,     34900,     34904,     34909,     34914,     34918,     34923,     34927,
	34932,     34936,     34941,     34945,     34950,     34954,     34959,     34963,
	34968,     34972,     34977,     34981,     34986,     34990,     34995,     34999,
	35004,     35008,     35013,     35017,     35022,     35026,     35031,     35035,
	35040,     35044,     35049,     35053,     35058,     35062,     35067,     35071,
	35076,     35080,     35085,     35089,     35094,     35098,     35103,     35107,
	35112,     35116,     35121,     35125,     35129,     35134,     35138,     35143,
	35147,     35152,     35156,     35161,     35165,     35170,     35174,     35179

};

/* ============================================
 * Application Variables
 * ============================================ */
volatile u16_t distance = 450;
volatile u16_t temperature = 30;
volatile u16_t sound_speed = SOUND;

/* ============================================
 * Application Objects
 * ============================================ */

/* ===== Define LCD Pins ===== */
LCD_4Bit_t lcd = {
	.rs = GPIO_PIN(PORT_C, PIN_4),
	.rw = GPIO_PIN(PORT_C, PIN_5),
	.en = GPIO_PIN(PORT_C, PIN_6),
	
	.d4 = GPIO_PIN(PORT_A, PIN_7),
	.d5 = GPIO_PIN(PORT_A, PIN_6),
	.d6 = GPIO_PIN(PORT_A, PIN_5),
	.d7 = GPIO_PIN(PORT_A, PIN_4)
};

/* ===== HW External interrupt Pin ===== */
Button_t hardware_int = { .pin = GPIO_PIN(PORT_D, PIN_3) };

/* ============================================
 * Application Functions
 * ============================================ */
static void Temp2Speed(void)
{
	s16_t index = (s16_t)( (float)(temperature -MIN_TEMPERATURE) * (DATA_SIZE-1.0f) / (float)DIF_TEMPERATURE +0.5f );
	if		(index > 255)	sound_speed = sound_speed_lut[255];
	else if (index < 0)		sound_speed = sound_speed_lut[0];
	else					sound_speed = sound_speed_lut[index];
}

static inline void UpdateLCD(void)
{
	u16_t temp = distance;
	LCD_GoTo(&lcd, 0, 12);
	for (u8_t i = 0; i < 3; i++)
	{
		u8_t ch;
		if (temp == 0) ch = ' '; 
		else ch = temp % 10 + '0';
		LCD_SendChar(&lcd, ch);
		temp /= 10;
	}
	
	temp = temperature;
	LCD_GoTo(&lcd, 1, 12);
	for (u8_t i = 0; i < 2; i++)
	{
		u8_t ch;
		if (temp == 0) ch = ' ';
		else ch = temp % 10 + '0';
		LCD_SendChar(&lcd, ch);
		temp /= 10;
	}
}
/* ============================================
 * Master transfer
 * ============================================ */
volatile u8_t tx_buf[2] = {0};

void Xfer_SoundSpeed(void)
{
	if (!I2C_IsBusy())
	{
		tx_buf[0] = (u8_t)(sound_speed >> 8);
		tx_buf[1] = (u8_t)(sound_speed);
		I2C_SendAsync(ID_SMC0, (const u8_t *)tx_buf, 2, NULL);
	}
}


/* ============================================
 * Slave transfer
 * ============================================ */
volatile u8_t rx_buf[2] = {0};
volatile u8_t rx_ind = 0;

void SlaveRxCallback(u8_t byte)
{
	if (rx_ind < 2) rx_buf[rx_ind++] = byte;
}

void SlaveStopCallback(void)
{
    if (rx_ind >= 2) distance = (((u16_t)rx_buf[0]) << 8) + ((u16_t)rx_buf[1]);
    rx_ind = 0;
}

/* ============================================
 * Main Application
 * ============================================ */
int main(void) {

	/* ===== Initialize HW External interrupt Pin ===== */
	BUTTON_Init(&hardware_int);
	
	/* ===== Initialize Display ===== */
    LCD_Init(&lcd);
	
	LCD_GoTo(&lcd, 0, 0);
	LCD_SendString(&lcd, "Dist.");
	LCD_GoTo(&lcd, 1, 0);
	LCD_SendString(&lcd, "Temp.");
	LCD_GoTo(&lcd, 0, 13);
	LCD_SendString(&lcd, " cm");
	LCD_GoTo(&lcd, 1, 13);
	LCD_SendChar(&lcd, ' ');
	LCD_SendChar(&lcd, 0xDF);
	LCD_SendChar(&lcd, 'C');
    
	LCD_Home(&lcd);
    LCD_SendCommand(&lcd, LCD_CMD_ENTRY_ARABIC);
    
	/* ===== ADC Configuration (for heat sensor) ===== */
	ADC_vReset();
	ADC_vSetChannel(ADC_CH0);
	ADC_vSetReference(ADC_REF_AREF);
	ADC_vSetClock(ADC_CLK_DIV128);
	ADC_vSetConversionInt(TRUE);
	
    /* Initialize ADC */
	ADC_Init();
	
	/* ===== I2C Configuration ===== */
	I2C_vReset();
	I2C_vSetAddress(ID_MMC0);
	I2C_vSetInterrupt(TRUE);
	
	/* Initialize I2C */
	I2C_SetSlaveCallbacks(SlaveRxCallback, NULL, SlaveStopCallback);
	I2C_Init();
	
	/* ===== Timers Configuration ===== */
	TC1_vReset();
	TC1_vSetMode(TC1_MODE_CTC_OCR1A);
	TC1_vSetClock(TC1_CLK_DIV256);
	TC1_vSetCompareAValue(31249);	// 1 interrupt per second
	TC1_vSetCompareAInt(TRUE);
	
	/* Initialize timers */
	TC1_Init();

	/* ===== Interrupt Configuration ===== */
    INT1_Enable(INT_TRIGGER_FALLING_EDGE);
    INT1_ClearFlag();
	INT_GlobalEnable();
	
    /* ===== Main Loop ===== */
	UpdateLCD();
    while(TRUE);
}

/* ============================================
 * Interrupts
 * ============================================ */

ISR(INT1_vect) // External interrupt 1
{
	Xfer_SoundSpeed();
}

/* Timer Interrupts*/
ISR(TIMER1_COMPA_vect)	// Timer1 compare A match interrupt
{
	ADC_StartConversion();
}

/* ADC Interrupts */
ISR(ADC_vect)		// ADC conversion complete
{	
	temperature = (u8_t)(100.0f * ADC_Read() * 5.0f / 1024.0f);
	Temp2Speed();
	UpdateLCD();
}

/* I2C Interrupts */
ISR(TWI_vect) // TWI event action
{
	I2C_OnEvent();
}