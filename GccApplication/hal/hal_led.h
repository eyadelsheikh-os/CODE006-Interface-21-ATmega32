/**
 * @file    hal_led.h
 * @brief   HAL LED Driver
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Provides high-level abstraction for controlling LEDs.
 *          All functions are hardware-agnostic and use GPIO_Pin_t.
 */

#ifndef HAL_LED_H_
#define HAL_LED_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"		/* Provides u8, u16, etc. */
#include "../common/gpio_types.h"		/* Provides GPIO_Pin_t, GPIO_Direction_t, etc. */
#include "../hal/hal_common.h"			/* Provides LED_ActiveType_t, Button_ActiveType_t, etc. */
#include "../mcal/mcal_dio.h"			/* Provides DIO_PinInit, DIO_PinWrite, etc. */

/* ============================================
 * LED Configuration
 * ============================================ */

/**
 * @brief   LED object structure.
 * @note    Encapsulates all LED properties in one place.
 *          Makes code cleaner and more maintainable.
 */
typedef struct {
    GPIO_Pin_t pin;             /**< GPIO pin connected to the button */
    LED_ActiveType_t active;	/**< Active-high or active-low */
} LED_t;


/* ============================================
 * Public Function Prototypes
 * ============================================ */
/**
 * @brief   Initialize an LED pin as output.
 * @param   pin			The GPIO pin connected to the LED.
 * @param   active_type LED_ACTIVE_HIGH or LED_ACTIVE_LOW.
 * @return  LED_t.
 * 
 * @example
 *      LED_t led = LED_Init(GPIO_PIN(PORT_B, PIN_5));
 */
LED_t LED_zInit(GPIO_Pin_t pin, LED_ActiveType_t active_type);

/**
 * @brief   Initialize an LED pin as output.
 * @param   pin     Pointer to the LED_t object.
 */
void LED_Init(const LED_t* led);

/**
 * @brief   Turn on the LED.
 * @param   pin     Pointer to the LED_t object.
 */
void LED_On(const LED_t* led);

/**
 * @brief   Turn off the LED.
 * @param   pin     Pointer to the LED_t object.
 */
void LED_Off(const LED_t* led);

/**
 * @brief   Toggle the LED state.
 * @param   pin     Pointer to the LED_t object.
 * 
 * @note    If LED is on, turns it off. If off, turns it on.
 */
void LED_Toggle(const LED_t* led);

/**
 * @brief   Get the current LED state.
 * @param   pin     Pointer to the LED_t object.
 * @return  GPIO_Level_t (LEVEL_HIGH = ON, LEVEL_LOW = OFF).
 * 
 * @note    Returns LEVEL_HIGH if LED is on, LEVEL_LOW if off.
 */
GPIO_Level_t LED_GetState(const LED_t* led);


#endif /* HAL_LED_H_ */
