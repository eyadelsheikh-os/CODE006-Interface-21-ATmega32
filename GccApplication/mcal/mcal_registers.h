/**
 * @file    mcal_registers.h
 * @brief   ATmega32 Memory-Mapped Register Definitions
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    This header provides direct access to all ATmega32
 *          hardware registers. All definitions are derived from
 *          the ATmega32 datasheet (Section: I/O Memory Map).
 * 
 * @warning Direct register manipulation is dangerous. Use MCAL
 *          driver functions whenever possible.
 * @see     mcal_dio.h, mcal_timers.h, mcal_uart.h
 */

#ifndef MCAL_REGISTERS_H_
#define MCAL_REGISTERS_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"          /* Provides u8_t, u16_t, reg8_t, reg16_t */

/* ============================================
 * I/O Ports (8-bit Registers)
 * ============================================ */

/* @name  Port A (PA0-PA7) */
#define PORTA   (*(reg8_t *)(0x3B))   /**< Data Register - Write to set output */
#define DDRA    (*(reg8_t *)(0x3A))   /**< Data Direction Register - 1=Output, 0=Input */
#define PINA    (*(const reg8_t *)(0x39)) /**< Input Pins Register - Read to get input state */

/* @name   Port B (PB0-PB7) */
#define PORTB   (*(reg8_t *)(0x38))   /**< Data Register - Write to set output */
#define DDRB    (*(reg8_t *)(0x37))   /**< Data Direction Register - 1=Output, 0=Input */
#define PINB    (*(const reg8_t *)(0x36)) /**< Input Pins Register - Read to get input state */

/* @name   Port C (PC0-PC7) */
#define PORTC   (*(reg8_t *)(0x35))   /**< Data Register - Write to set output */
#define DDRC    (*(reg8_t *)(0x34))   /**< Data Direction Register - 1=Output, 0=Input */
#define PINC    (*(const reg8_t *)(0x33)) /**< Input Pins Register - Read to get input state */

/* @name   Port D (PD0-PD7) */
#define PORTD   (*(reg8_t *)(0x32))   /**< Data Register - Write to set output */
#define DDRD    (*(reg8_t *)(0x31))   /**< Data Direction Register - 1=Output, 0=Input */
#define PIND    (*(const reg8_t *)(0x30)) /**< Input Pins Register - Read to get input state */


/* ============================================
 * Timer/Counter 0 (8-bit Timer)
 * ============================================ */
#define TCCR0   (*(reg8_t *)(0x53))   /**< Timer/Counter Control Register 0 */
#define TCNT0   (*(reg8_t *)(0x52))   /**< Timer/Counter 0 Value */
#define OCR0    (*(reg8_t *)(0x5C))   /**< Output Compare Register 0 */
#define TIMSK   (*(reg8_t *)(0x59))   /**< Timer/Counter Interrupt Mask Register */
#define TIFR    (*(reg8_t *)(0x58))   /**< Timer/Counter Interrupt Flag Register */


/* ============================================
 * Timer/Counter 1 (16-bit Timer)
 * ============================================ */
#define TCCR1A  (*(reg8_t *)(0x4F))   /**< Timer/Counter 1 Control Register A */
#define TCCR1B  (*(reg8_t *)(0x4E))   /**< Timer/Counter 1 Control Register B */
#define TCNT1L  (*(reg8_t *)(0x4C))   /**< Timer/Counter 1 Value (Low Byte) */
#define TCNT1H  (*(reg8_t *)(0x4D))   /**< Timer/Counter 1 Value (High Byte) */
#define TCNT1   (*(reg16_t *)(0x4C))  /**< Timer/Counter 1 Value (16-bit access) */
#define OCR1AL  (*(reg8_t *)(0x4A))   /**< Output Compare Register 1A (Low Byte) */
#define OCR1AH  (*(reg8_t *)(0x4B))   /**< Output Compare Register 1A (High Byte) */
#define OCR1A   (*(reg16_t *)(0x4A))  /**< Output Compare Register 1A (16-bit access) */
#define OCR1BL  (*(reg8_t *)(0x48))   /**< Output Compare Register 1B (Low Byte) */
#define OCR1BH  (*(reg8_t *)(0x49))   /**< Output Compare Register 1B (High Byte) */
#define OCR1B   (*(reg16_t *)(0x48))  /**< Output Compare Register 1B (16-bit access) */
#define ICR1L   (*(reg8_t *)(0x46))   /**< Input Capture Register 1 (Low Byte) */
#define ICR1H   (*(reg8_t *)(0x47))   /**< Input Capture Register 1 (High Byte) */
#define ICR1    (*(reg16_t *)(0x46))  /**< Input Capture Register 1 (16-bit access) */


/* ============================================
 * Timer/Counter 2 (8-bit Timer with Asynchronous Operation)
 * ============================================ */
#define TCCR2   (*(reg8_t *)(0x45))   /**< Timer/Counter 2 Control Register */
#define TCNT2   (*(reg8_t *)(0x44))   /**< Timer/Counter 2 Value */
#define OCR2    (*(reg8_t *)(0x43))   /**< Output Compare Register 2 */
#define ASSR    (*(reg8_t *)(0x42))   /**< Asynchronous Status Register */
#define SFIOR   (*(reg8_t *)(0x50))   /**< Special Function I/O Register */


/* ============================================
 * Watchdog Timer
 * ============================================ */
#define WDTCR   (*(reg8_t *)(0x41))   /**< Watchdog Timer Control Register */


/* ============================================
 * EEPROM
 * ============================================ */
#define EEARL   (*(reg8_t *)(0x3E))   /**< EEPROM Address Register (Low Byte) */
#define EEARH   (*(reg8_t *)(0x3F))   /**< EEPROM Address Register (High Byte) */
#define EEAR    (*(reg16_t *)(0x3E))  /**< EEPROM Address Register (16-bit access) */
#define EEDR    (*(reg8_t *)(0x3D))   /**< EEPROM Data Register */
#define EECR    (*(reg8_t *)(0x3C))   /**< EEPROM Control Register */


/* ============================================
 * USART (Universal Synchronous/Asynchronous Receiver/Transmitter)
 * ============================================ */
#define UDR     (*(reg8_t *)(0x2C))   /**< USART I/O Data Register */
#define UCSRA   (*(reg8_t *)(0x2B))   /**< USART Control and Status Register A */
#define UCSRB   (*(reg8_t *)(0x2A))   /**< USART Control and Status Register B */
#define UCSRC   (*(reg8_t *)(0x40))   /**< USART Control and Status Register C (shared with UBRRH) */
#define UBRRL   (*(reg8_t *)(0x29))   /**< USART Baud Rate Register (Low Byte) */
#define UBRRH   (*(reg8_t *)(0x40))   /**< USART Baud Rate Register (High Byte) */
#define UBRR    (*(reg16_t *)(0x29))  /**< USART Baud Rate Register (16-bit access) */


/* ============================================
 * SPI (Serial Peripheral Interface)
 * ============================================ */
#define SPCR    (*(reg8_t *)(0x2D))   /**< SPI Control Register */
#define SPSR    (*(reg8_t *)(0x2E))   /**< SPI Status Register */
#define SPDR    (*(reg8_t *)(0x2F))   /**< SPI Data Register */


/* ============================================
 * TWI (Two-Wire Interface / I2C)
 * ============================================ */
#define TWBR    (*(reg8_t *)(0x20))   /**< TWI Bit Rate Register */
#define TWCR    (*(reg8_t *)(0x56))   /**< TWI Control Register */
#define TWSR    (*(reg8_t *)(0x21))   /**< TWI Status Register */
#define TWDR    (*(reg8_t *)(0x23))   /**< TWI Data Register */
#define TWAR    (*(reg8_t *)(0x22))   /**< TWI Address Register */


/* ============================================
 * Analog-to-Digital Converter (ADC)
 * ============================================ */
#define ADMUX   (*(reg8_t *)(0x27))   /**< ADC Multiplexer Selection Register */
#define ADCSRA  (*(reg8_t *)(0x26))   /**< ADC Control and Status Register A */
#define ADCH    (*(reg8_t *)(0x25))   /**< ADC Data Register (High Byte) */
#define ADCL    (*(reg8_t *)(0x24))   /**< ADC Data Register (Low Byte) */
#define ADC     (*(reg16_t *)(0x24))  /**< ADC Data Register (16-bit access) */
#define SFIOR   (*(reg8_t *)(0x50))   /**< Special Function I/O Register (ADC trigger source) */


/* ============================================
 * Interrupts
 * ============================================ */
#define SREG    (*(reg8_t *)(0x5F))   /**< Status Register (Global Interrupt Enable bit) */
#define MCUCR   (*(reg8_t *)(0x55))   /**< MCU Control Register */
#define MCUCSR  (*(reg8_t *)(0x54))   /**< MCU Control and Status Register */
#define GICR    (*(reg8_t *)(0x5B))   /**< General Interrupt Control Register (formerly GIMSK) */
#define GIFR    (*(reg8_t *)(0x5A))   /**< General Interrupt Flag Register */


/* ============================================
 * Power Management
 * ============================================ */
#define SMCR    (*(reg8_t *)(0x57))   /**< Sleep Mode Control Register */
#define OSCCAL  (*(reg8_t *)(0x51))   /**< Oscillator Calibration Register */


#endif /* MCAL_REGISTERS_H_ */

