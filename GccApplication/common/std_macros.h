/**
 * @file    std_macros.h
 * @brief   Standard bit manipulation and rotation macros for AVR
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    This header provides safe, portable macros for common
 *          embedded operations. All macros are designed to be
 *          side-effect aware and use unsigned integer types.
 */

#ifndef STD_MACROS_H_
#define STD_MACROS_H_
/* ============================================
 * Basic Bit Manipulation Macros
 * ============================================ */

/**
 * @brief   Set a specific bit in a register/variable.
 * @param   reg     The register/variable to modify (lvalue).
 * @param   bit     The bit position to set (0-indexed).
 */
#define SET_BIT(reg, bit)   ((reg) |= (1u << (bit)))

/**
 * @brief   Clear a specific bit in a register/variable.
 * @param   reg     The register/variable to modify (lvalue).
 * @param   bit     The bit position to clear (0-indexed).
 */
#define CLR_BIT(reg, bit)   ((reg) &= (~(1u << (bit))))

/**
 * @brief   Set/Clear a specific bit in a register/variable.
 * @param   reg     The register/variable to modify (lvalue).
 * @param   bit     The bit position to set/clear (0-indexed).
 * @param   val     The desired state (x-set : 0-clear).
 */
#define WRITE_BIT(reg, bit, val)	((val) ? SET_BIT(reg, bit) : CLR_BIT(reg, bit))

/**
 * @brief   Toggle a specific bit in a register/variable.
 * @param   reg     The register/variable to modify (lvalue).
 * @param   bit     The bit position to toggle (0-indexed).
 */
#define TOG_BIT(reg, bit)   ((reg) ^= (1u << (bit)))

/**
 * @brief   Read a specific bit from a register/variable.
 * @param   reg     The register/variable to read.
 * @param   bit     The bit position to read (0-indexed).
 * @return  Returns 1 if the bit is set, 0 otherwise.
 * 
 * @note    The result is guaranteed to be exactly 0 or 1.
 */
#define READ_BIT(reg, bit)  (((reg) >> (bit)) & 1u)

/**
 * @brief Extracts the lower 4 bits (low nibble) of an 8-bit value.
 * @param value An 8-bit value.
 * @return The lower nibble as a value from 0x00 to 0x0F.
 */
#define LOW_NIBBLE(value) ((u8_t)((value) & 0x0Fu))

/**
 * @brief Extracts the upper 4 bits (high nibble) of an 8-bit value.
 * @param value An 8-bit value.
 * @return The upper nibble as a value from 0x00 to 0x0F.
 */
#define HIGH_NIBBLE(value) ((u8_t)(((value) >> 4) & 0x0Fu))


/* ============================================
 * Rotate Macros (8-bit)
 * ============================================ */
#ifndef REGISTER_SIZE
#define REGISTER_SIZE 8
#endif /* REGISTER_SIZE */

/**
 * @brief   Rotate an 8-bit value left by `shift` positions.
 * @param   val     The 8-bit value to rotate (uint8_t).
 * @param   shift   Number of positions to shift (0-7).
 * @return  Rotated 8-bit value.
 */
#define ROR_BIT(reg, bit) reg = ( reg<<(REGISTER_SIZE-num) )|( reg>>(num) )

/**
 * @brief   Rotate an 8-bit value right by `shift` positions.
 * @param   val     The 8-bit value to rotate (uint8_t).
 * @param   shift   Number of positions to shift (0-7).
 * @return  Rotated 8-bit value.
 */
#define ROL_BIT(reg, bit) reg = ( reg>>(REGISTER_SIZE-num) )|( reg<<(num) )


#endif /* STD_MACROS_H_ */