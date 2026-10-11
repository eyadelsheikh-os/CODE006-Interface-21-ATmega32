/**
 * @file    std_types.h
 * @brief   Standard
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    
 */

#ifndef STD_TYPES_H_
#define STD_TYPES_H_
/* ============================================
 * Standard Integer Types for ATmega32
 * ============================================ */

/* Unsigned integers */
typedef unsigned char       u8_t;
typedef unsigned short      u16_t;
typedef unsigned long       u32_t;
typedef unsigned long long  u64_t;

/* Signed integers */
typedef signed char         s8_t;
typedef signed short        s16_t;
typedef signed long         s32_t;
typedef signed long long    s64_t;

/* ============================================
 * Hardware Register Types 
 * ============================================ */
typedef volatile u8_t    reg8_t;
typedef volatile u16_t   reg16_t;

/* ============================================
 * Standard Bit Numbering
 * ============================================ */
#define BIT_0	0	/**< Bit 0 of a Byte */
#define BIT_1	1	/**< Bit 1 of a Byte */
#define BIT_2	2	/**< Bit 2 of a Byte */
#define BIT_3	3	/**< Bit 3 of a Byte */
#define BIT_4	4	/**< Bit 4 of a Byte */
#define BIT_5	5	/**< Bit 5 of a Byte */
#define BIT_6	6	/**< Bit 6 of a Byte */
#define BIT_7	7	/**< Bit 7 of a Byte */

/* ============================================
 * Boolean Types 
 * ============================================ */
typedef enum {
	TRUE  = 1,
	FALSE = 0
} bool_t;

/* ============================================
 * Null pointer definition
 * ============================================ */
#ifndef NULL
#define NULL                ((void*)0)
#endif /* NULL */

/* ============================================
 * Standard Status/Control Types
 * ============================================ */
typedef enum {
    STD_LOW  = 0,
    STD_HIGH = 1
} Std_LevelType_t;

typedef enum {
    STD_RESET = 0,
    STD_SET   = 1
} Std_StatusType_t;

typedef enum {
    STD_DISABLED = 0,
    STD_ENABLED  = 1
} Std_EnableType_t;


#endif /* STD_TYPES_H_ */