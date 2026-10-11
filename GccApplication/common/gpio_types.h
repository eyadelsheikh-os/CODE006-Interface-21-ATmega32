/**
 * @file    gpio_types.h
 * @brief   Standard GPIO type definitions for AVR ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    This header defines the fundamental types used to represent
 *          GPIO pins across all layers (MCAL, HAL, APP). All drivers
 *          should use these types for hardware abstraction.
 */

#ifndef GPIO_TYPES_H_
#define GPIO_TYPES_H_
/* ============================================
 * GPIO Port Definitions
 * ============================================ */

/**
 * @brief   Available GPIO ports on ATmega32.
 * @note    These map directly to hardware PORT registers.
 */
typedef enum {
    PORT_A = 0,     /**< Port A (PA0-PA7) */
    PORT_B = 1,     /**< Port B (PB0-PB7) */
    PORT_C = 2,     /**< Port C (PC0-PC7) */
    PORT_D = 3      /**< Port D (PD0-PD7) */
} GPIO_Port_t;

/**
 * @brief   GPIO pin number (0-7) as defined by hardware.
 * @note    Corresponds to the bit position in the PORT register.
 */
typedef enum {
	PIN_0 = 0,      /**< Bit 0 of the port */
	PIN_1 = 1,      /**< Bit 1 of the port */
	PIN_2 = 2,      /**< Bit 2 of the port */
	PIN_3 = 3,      /**< Bit 3 of the port */
	PIN_4 = 4,      /**< Bit 4 of the port */
	PIN_5 = 5,      /**< Bit 5 of the port */
	PIN_6 = 6,      /**< Bit 6 of the port */
	PIN_7 = 7       /**< Bit 7 of the port */
} GPIO_PinNumber_t;


/* ============================================
 * GPIO Pin Structure
 * ============================================ */

/**
 * @brief   Complete GPIO pin descriptor.
 * @note    Used to identify any physical pin on the MCU.
 *          Example usage:
 *          @code
 *          GPIO_Pin_t led_pin = { .port = PORT_B, .pin = PIN_5 };
 *          DIO_WritePin(led_pin, STD_HIGH);
 *          @endcode
 */
 typedef struct {
 GPIO_Port_t port;           /**< Which port (A, B, C, or D) */
 GPIO_PinNumber_t pin;       /**< Which pin number (0-7) */
 } GPIO_Pin_t;


/* ============================================
 * GPIO Pin Direction
 * ============================================ */

/**
 * @brief   GPIO pin direction states.
 * @note    Used for configuring pin mode in MCAL drivers.
 */
typedef enum {
	DIR_INPUT  = 0,     /**< Pin configured as input (high impedance) */
	DIR_OUTPUT = 1      /**< Pin configured as output (push-pull) */
} GPIO_Direction_t;

/**
 * @brief   GPIO internal pull-up resistor states.
 * @note    Only valid when pin is configured as INPUT.
 */
typedef enum {
    PULLUP_DISABLED = 0,    /**< No pull-up resistor */
    PULLUP_ENABLED  = 1     /**< Internal pull-up resistor enabled */
} GPIO_PullUp_t;


/* ============================================
 * GPIO Pin Level
 * ============================================ */
/**
 * @brief   GPIO logic level states.
 * @note    Maps directly to hardware HIGH/LOW voltage levels.
 */
typedef enum {
    LEVEL_LOW  = 0,     /**< Logic LOW (0V or GND) */
    LEVEL_HIGH = 1      /**< Logic HIGH (VCC or VDD) */
} GPIO_Level_t;


/* ============================================
 * Convenience Macros for Common Pins
 * ============================================ */

/**
 * @brief   Macro to create a GPIO pin descriptor with less typing.
 * @param   p     Port (PORT_A, PORT_B, etc.)
 * @param   n     Pin number (PIN_0 to PIN_7)
 * @return  GPIO_Pin_t structure.
 * 
 * @example
 *      GPIO_Pin_t led = GPIO_PIN(PORT_B, PIN_5);
 */
#define GPIO_PIN(p, n)  ((GPIO_Pin_t){ .port = (p), .pin = (n) })


#endif /* GPIO_TYPES_H_ */