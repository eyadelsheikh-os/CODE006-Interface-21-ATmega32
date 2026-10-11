/**
 * @file    mcal_dio.c
 * @brief   MCAL Digital I/O Driver Implementation
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    All register accesses are abstracted through mcal_registers.h
 *          for maximum portability and maintainability.
 */

#include "mcal_dio.h"
/* ============================================
 * Private Helper Functions
 * ============================================ */

/**
 * @brief   Get the Data Direction Register (DDR) for a given port.
 * @param   port    The port (PORT_A, PORT_B, etc.)
 * @return  Pointer to the DDR register (volatile).
 * @warning Internal function - not exposed in header.
 */
static inline reg8_t* DIO_GetDDR(GPIO_Port_t port) 
{
    switch(port) 
	{
        case PORT_A: return &DDRA;
        case PORT_B: return &DDRB;
        case PORT_C: return &DDRC;
        case PORT_D: return &DDRD;
        default: return NULL;
    }
}

/**
 * @brief   Get the Output Data Register (PORT) for a given port.
 * @param   port    The port (PORT_A, PORT_B, etc.)
 * @return  Pointer to the PORT register (volatile).
 * @warning Internal function - not exposed in header.
 */
static inline reg8_t* DIO_GetPort(GPIO_Port_t port) 
{
    switch(port) 
	{
        case PORT_A: return &PORTA;
        case PORT_B: return &PORTB;
        case PORT_C: return &PORTC;
        case PORT_D: return &PORTD;
        default: return NULL;
    }
}

/**
 * @brief   Get the Input Pins Register (PIN) for a given port.
 * @param   port    The port (PORT_A, PORT_B, etc.)
 * @return  Pointer to the PIN register (volatile, const).
 * @warning Internal function - not exposed in header.
 */
static inline const reg8_t* DIO_GetPin(GPIO_Port_t port) 
{
    switch(port) 
	{
        case PORT_A: return &PINA;
        case PORT_B: return &PINB;
        case PORT_C: return &PINC;
        case PORT_D: return &PIND;
        default: return NULL;
    }
}

/**
 * @brief   Validate a pin structure.
 * @param   pin     The pin to validate.
 * @return  true if valid, false otherwise.
 */
static inline bool_t DIO_IsPinValid(GPIO_Pin_t pin) 
{
    if (pin.port > PORT_D) return FALSE;
    if (pin.pin > PIN_7) return FALSE;
    return TRUE;
}


/* ============================================
 * Public Function Implementations
 * ============================================ */
void DIO_PinInit(GPIO_Pin_t pin, GPIO_Direction_t direction) 
{
    reg8_t* ddr = DIO_GetDDR(pin.port);
    
    /* Validate inputs */
    if (!DIO_IsPinValid(pin)) 
	{
        return;  /* Silently fail for safety */
    }
    
    if (direction == DIR_OUTPUT) 
	{
        SET_BIT(*ddr, pin.pin);
    } else {
        CLR_BIT(*ddr, pin.pin);
    }
}

void DIO_PinWrite(GPIO_Pin_t pin, GPIO_Level_t level) 
{
    reg8_t* port = DIO_GetPort(pin.port);
    
    /* Validate inputs */
    if (!DIO_IsPinValid(pin))
	{
        return;  /* Silently fail for safety */
    }
    
    if (level == LEVEL_HIGH) 
	{
        SET_BIT(*port, pin.pin);
    } 
	else 
	{
        CLR_BIT(*port, pin.pin);
    }
}

GPIO_Level_t DIO_PinRead(GPIO_Pin_t pin) 
{
    const reg8_t* pin_reg = DIO_GetPin(pin.port);
    
    /* Validate inputs */
    if (!DIO_IsPinValid(pin)) 
	{
        return LEVEL_LOW;  /* Safe default */
    }
    
    return (GPIO_Level_t)READ_BIT(*pin_reg, pin.pin);
}

void DIO_PinToggle(GPIO_Pin_t pin) 
{
    reg8_t* port = DIO_GetPort(pin.port);
    
    /* Validate inputs */
    if (!DIO_IsPinValid(pin)) 
	{
        return;  /* Silently fail for safety */
    }
    
    TOG_BIT(*port, pin.pin);
}

void DIO_PortInit(GPIO_Port_t port, GPIO_Direction_t direction) 
{
    reg8_t* ddr = DIO_GetDDR(port);
    
    /* Validate inputs */
    if (ddr == NULL)
	{
        return;  /* Silently fail for safety */
    }
    
    if (direction == DIR_OUTPUT) 
	{
        *ddr = 0xFF;  /* All pins output */
    } 
	else 
	{
        *ddr = 0x00;  /* All pins input */
    }
}

void DIO_PortWrite(GPIO_Port_t port, u8_t value) 
{
    reg8_t* port_reg = DIO_GetPort(port);
    
    /* Validate inputs */
    if (port_reg == NULL)
	{
        return;  /* Silently fail for safety */
    }
    
    *port_reg = value;
}

u8_t DIO_PortRead(GPIO_Port_t port) 
{
    const reg8_t* pin_reg = DIO_GetPin(port);
    
    /* Validate inputs */
    if (pin_reg == NULL)
	{
        return 0x00;  /* Safe default */
    }
    
    return *pin_reg;
}
