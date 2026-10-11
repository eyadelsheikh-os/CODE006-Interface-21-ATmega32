/**
 * @file    hal_button.h
 * @brief   HAL Button Driver
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Provides high-level abstraction for reading buttons.
 *          Supports active-high and active-low configurations.
 *          Uses a Button_t struct to encapsulate button properties.
 *          Does NOT include software debouncing.
 */

#ifndef HAL_BUTTON_H_
#define HAL_BUTTON_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"        /* Provides u8, u16, etc. */
#include "../common/gpio_types.h"       /* Provides GPIO_Pin_t, GPIO_Direction_t, etc. */
#include "../hal/hal_common.h"			/* Provides LED_ActiveType_t, Button_ActiveType_t, etc. */
#include "../mcal/mcal_dio.h"			/* Provides DIO_PinInit, DIO_PinWrite, etc. */

/* ============================================
 * Button Configuration
 * ============================================ */

/**
 * @brief   Button object structure.
 * @note    Encapsulates all button properties in one place.
 *          Makes code cleaner and more maintainable.
 */
typedef struct {
	GPIO_Pin_t pin;              /**< GPIO pin connected to the button */
	GPIO_PullUp_t pullup;
	Button_ActiveType_t active;  /**< Active-high or active-low */
} Button_t;


/* ============================================
 * Public Function Prototypes
 * ============================================ */

/**
 * @brief   Initialize a button and return a Button_t object.
 * @param   pin         The GPIO pin connected to the button.
 * @param   active_type BUTTON_ACTIVE_HIGH or BUTTON_ACTIVE_LOW.
 * @param   pullup      PULLUP_ENABLED or PULLUP_DISABLED.
 * @return  Button_t    Fully configured button object.
 * 
 * @example
 *      Button_t my_button = BUTTON_Init(GPIO_PIN(PORT_D, PIN_2), 
 *                                        BUTTON_ACTIVE_LOW, 
 *                                        PULLUP_ENABLED);
 *      if (BUTTON_IsPressed(&my_button)) {
 *          // Button is pressed
 *      }
 */
Button_t BUTTON_zInit(GPIO_Pin_t pin, Button_ActiveType_t active_type, GPIO_PullUp_t pullup);

/**
 * @brief   Initialize a button object.
 * @param   btn     Pointer to the Button_t object.
 */
void BUTTON_Init(const Button_t* btn);

/**
 * @brief   Read the raw (un-debounced) state of a button.
 * @param   btn     Pointer to the Button_t object.
 * @return  bool_t  TRUE = pressed, FALSE = not pressed.
 * 
 * @note    Returns the actual pin level converted to pressed logic.
 *          For debounced reading, use BUTTON_IsPressed().
 */
bool_t BUTTON_RawRead(const Button_t* btn);


#endif /* HAL_BUTTON_H_ */

