/**
 * @file    hal_button.c
 * @brief   HAL Button Driver Implementation
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Implements button functionality using Button_t struct.
 *          Does NOT include software debouncing.
 */

#include "hal_button.h"
/* ============================================
 * Private Helper Functions
 * ============================================ */

/**
 * @brief   Get the raw physical state of a button.
 * @param   btn     Pointer to the Button_t object.
 * @return  bool_t  TRUE = physically pressed, FALSE = physically released.
 */
static inline bool_t BUTTON_IsPhysicallyPressed(const Button_t* btn) 
{
    GPIO_Level_t level = DIO_PinRead(btn->pin);
    
    if (btn->active == BUTTON_ACTIVE_HIGH) 
	{
        return (level == LEVEL_HIGH) ? TRUE : FALSE;
    } 
	else 
	{   /* BUTTON_ACTIVE_LOW */
        return (level == LEVEL_LOW) ? TRUE : FALSE;
    }
}

/* ============================================
 * Public Function Implementations
 * ============================================ */
Button_t BUTTON_zInit(GPIO_Pin_t pin, Button_ActiveType_t active_type, GPIO_PullUp_t pullup) 
{
    Button_t btn;
    
    /* Store configuration */
    btn.pin = pin;
    btn.active = active_type;
    btn.pullup = pullup;
    
    /* Initialize pin as input */
    DIO_PinInit(pin, DIR_INPUT);
    
    /* Configure pull-up resistor if requested */
    if (pullup == PULLUP_ENABLED)
	{	/* Write HIGH to enable internal pull-up */
        DIO_PinWrite(pin, LEVEL_HIGH);
    } 
	else
	{	/* Write LOW to disable internal pull-up */
        DIO_PinWrite(pin, LEVEL_LOW);
    }
    
    return btn;
}

void BUTTON_Init(const Button_t* btn)
{
	/* Initialize pin as input */
	DIO_PinInit(btn->pin, DIR_INPUT);
	
	/* Configure pull-up resistor if requested */
	if (btn->pullup == PULLUP_ENABLED)
	{	/* Write HIGH to enable internal pull-up */
		DIO_PinWrite(btn->pin, LEVEL_HIGH);
	}
	else
	{	/* Write LOW to disable internal pull-up */
		DIO_PinWrite(btn->pin, LEVEL_LOW);
	}
	
}

bool_t BUTTON_RawRead(const Button_t* btn) {
    return BUTTON_IsPhysicallyPressed(btn);
}


