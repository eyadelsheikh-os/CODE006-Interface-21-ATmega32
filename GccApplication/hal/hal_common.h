/**
 * @file    hal_common.h
 * @brief   Common HAL Types and Definitions
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    This header contains shared types used across multiple HAL modules.
 *          HAL modules should never include each other - they only include this.
 */

#ifndef HAL_COMMON_H_
#define HAL_COMMON_H_
/* ============================================
 * LED/Display Active Types
 * ============================================ */

/**
 * @brief   LED/Display active level configuration.
 * @note    Defines whether HIGH or LOW turns the LED/segment ON.
 *          Used by LED, 7-segment, and other display drivers.
 */
typedef enum {
	LED_ACTIVE_HIGH = 0,  /**< ON = HIGH (common cathode) */
	LED_ACTIVE_LOW  = 1   /**< ON = LOW (common anode) */
} LED_ActiveType_t;


/* ============================================
 * Button Configuration Types
 * ============================================ */

/**
 * @brief   Button active level configuration.
 * @note    Defines whether HIGH or LOW means "button pressed".
 */
typedef enum {
	BUTTON_ACTIVE_HIGH = 0,  /**< Pressed = HIGH (pull-down resistor) */
	BUTTON_ACTIVE_LOW  = 1   /**< Pressed = LOW (pull-up resistor) */
} Button_ActiveType_t;


#endif /* HAL_COMMON_H_ */