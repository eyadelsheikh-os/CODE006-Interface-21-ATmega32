/**
 * @file    hal_led.c
 * @brief   HAL LED Driver Implementation
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Uses MCAL DIO driver for all hardware operations.
 *          This layer is hardware-agnostic.
 */

#include "hal_led.h"
/* ============================================
 * Public Function Implementations
 * ============================================ */
LED_t LED_zInit(GPIO_Pin_t pin, LED_ActiveType_t active_type) 
{
	LED_t led;
	
	/* Store configuration */
    led.pin = pin;
    led.active = active_type;
	
    /* Initialize pin as output */
    DIO_PinInit(pin, DIR_OUTPUT);
	
	return led;
}

void LED_Init(const LED_t* led)
{
	/* Initialize pin as output */
	DIO_PinInit(led->pin, DIR_OUTPUT);
}

void LED_On(const LED_t* led) 
{
	if (led->active == LED_ACTIVE_HIGH) 
	{
		DIO_PinWrite(led->pin, LEVEL_HIGH);
    } 
	else 
	{   /* LED_ACTIVE_LOW */
		DIO_PinWrite(led->pin, LEVEL_LOW);
    }
}

void LED_Off(const LED_t* led) 
{
	if (led->active == LED_ACTIVE_HIGH) 
	{
		DIO_PinWrite(led->pin, LEVEL_LOW);
    } 
	else 
	{   /* LED_ACTIVE_LOW */
		DIO_PinWrite(led->pin, LEVEL_HIGH);
    }
}

void LED_Toggle(const LED_t* led) 
{
    DIO_PinToggle(led->pin);
}

GPIO_Level_t LED_GetState(const LED_t* led) 
{
    return DIO_PinRead(led->pin);
}
