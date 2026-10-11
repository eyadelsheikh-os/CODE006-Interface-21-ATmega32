/**
 * @file    mcal_dio.h
 * @brief   MCAL Digital I/O Driver for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    This driver provides hardware abstraction for all GPIO
 *          operations on the ATmega32 microcontroller. All functions
 *          are designed to be lightweight and efficient.
 * 
 * @warning Direct register manipulation is encapsulated here.
 *          Application code should never access registers directly.
 */

#ifndef MCAL_DIO_H_
#define MCAL_DIO_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"          /* Provides u8, u16, etc. */
#include "../common/std_macros.h"		  /* Provides SET_BIT, CLR_BIT, etc. */
#include "../common/gpio_types.h"         /* Provides GPIO_Pin_t, GPIO_Direction_t, etc. */
#include "../mcal/mcal_registers.h"       /* Provides PORTA, DDRA, etc. */


/* ============================================
 * Public Function Prototypes
 * ============================================ */

/**
 * @brief   Initialize a single GPIO pin direction.
 * @param   pin         The pin to configure (GPIO_Pin_t).
 * @param   direction   DIR_INPUT or DIR_OUTPUT.
 * 
 * @example
 *      GPIO_Pin_t led = GPIO_PIN(PORT_B, PIN_5);
 *      DIO_PinInit(led, DIR_OUTPUT);
 */
void DIO_PinInit(GPIO_Pin_t pin, GPIO_Direction_t direction);

/**
 * @brief   Write a logic level to a GPIO pin.
 * @param   pin     The pin to write to (GPIO_Pin_t).
 * @param   level   LEVEL_LOW or LEVEL_HIGH.
 * 
 * @example
 *      GPIO_Pin_t led = GPIO_PIN(PORT_B, PIN_5);
 *      DIO_PinWrite(led, LEVEL_HIGH);  // Turn LED on
 */
void DIO_PinWrite(GPIO_Pin_t pin, GPIO_Level_t level);

/**
 * @brief   Read the current logic level of a GPIO pin.
 * @param   pin     The pin to read from (GPIO_Pin_t).
 * @return  GPIO_Level_t (LEVEL_LOW or LEVEL_HIGH).
 * 
 * @example
 *      GPIO_Pin_t button = GPIO_PIN(PORT_D, PIN_2);
 *      if (DIO_PinRead(button) == LEVEL_HIGH) {
 *          // Button is pressed
 *      }
 */
GPIO_Level_t DIO_PinRead(GPIO_Pin_t pin);

/**
 * @brief   Toggle the current logic level of a GPIO pin.
 * @param   pin     The pin to toggle (GPIO_Pin_t).
 * 
 * @example
 *      GPIO_Pin_t led = GPIO_PIN(PORT_B, PIN_5);
 *      DIO_PinToggle(led);  // Flip LED state
 */
void DIO_PinToggle(GPIO_Pin_t pin);

/**
 * @brief   Initialize all pins of a GPIO port.
 * @param   port        The port to configure (GPIO_Port_t).
 * @param   direction   DIR_INPUT or DIR_OUTPUT for ALL pins.
 * 
 * @note    This sets the entire 8-bit port to the same direction.
 */
void DIO_PortInit(GPIO_Port_t port, GPIO_Direction_t direction);

/**
 * @brief   Write a value to an entire GPIO port.
 * @param   port    The port to write to (GPIO_Port_t).
 * @param   value   8-bit value to write to the port.
 * 
 * @note    Only the lower 8 bits are used.
 */
void DIO_PortWrite(GPIO_Port_t port, u8_t value);

/**
 * @brief   Read the current value of an entire GPIO port.
 * @param   port    The port to read from (GPIO_Port_t).
 * @return  8-bit value of the port.
 */
u8_t DIO_PortRead(GPIO_Port_t port);


#endif /* MCAL_DIO_H_ */