/**
 * @file    ud_library.h
 * @brief   
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    
 */

/* ============================================
 * User Defined Drivers Includes
 * ============================================ */
 #include "../common/std_types.h"			/* Provides u8, u16, etc. */
 #include "../common/gpio_types.h"			/* Provides GPIO_Pin_t, GPIO_Direction_t, etc. */
 #include "../common/ud_config.h"			/* Provides F_CPU, etc. */

 #include "../str/str_circular_queue.h"		/* Provides CIRQ_Init, CIRQ_Stack, CIRQ_Pop, etc. */

 #include "../hal/hal_led.h"					/* Provides LED_Init, LED_Off, LED_On, etc. */
 #include "../hal/hal_button.h"				/* Provides BUTTON_Init, BUTTON_RawRead, etc. */
 #include "../hal/hal_sevenseg.h"			/* Provides SEG_Init, SEG_RawWrite, etc. */
 #include "../hal/hal_keypad.h"				/* Provides KEYPAD_Init, KEYPAD_JustPressed, etc. */
 #include "../hal/hal_lcd4.h"				/* Provides LCD_Init, LCD_SendCommand, etc. */

 #include "../mcal/mcal_eeprom.h"			/* Provides EEPROM_Read, EEPROM_Write. */
 #include "../mcal/mcal_timers.h"			/* Provides TC0_vReset, TC0_Init, TC1_vReset, etc. */
 #include "../mcal/mcal_interrupt.h"			/* Provides INT_GlobalEnable, INT_GlobalDisable. */
 #include "../mcal/mcal_adc.h"				/* Provides ADC_vReset, ADC_Init, etc. */
 #include "../mcal/mcal_usart.h"				/* Provides USART_vReset, USART_Init, etc. */
 #include "../mcal/mcal_spi.h"				/* Provides SPI_vReset, SPI_Init, etc. */
 #include "../mcal/mcal_i2c.h"				/* Provides I2C_vReset, I2C_Init, etc. */

