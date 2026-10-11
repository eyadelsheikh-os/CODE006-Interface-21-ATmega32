/**
 * @file    mcal_usart.h
 * @brief   MCAL USART (Universal Sync/Async Serial) Driver for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Provides low-level functions to configure and control the
 *          on-chip USART peripheral. Supports asynchronous (UART) and
 *          synchronous (SPI-like) modes, all parity/stop-bit settings,
 *          5–9 bit character sizes, and interrupt-driven or polling I/O.
 * 
 * @warning UCSRC and UBRRH share the same I/O address (0x40). The URSEL
 *          bit (bit 7) selects which register is being accessed:
 *              URSEL = 1 ? UCSRC
 *              URSEL = 0 ? UBRRH
 *          Any write to those registers must set URSEL correctly.
 */

#ifndef MCAL_USART_H_
#define MCAL_USART_H_

/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"			/* Provides u8_t, u16_t, bool_t */
#include "../common/std_macros.h"			/* Provides SET_BIT, CLR_BIT, etc. */
#include "../common/ud_config.h"			/* Provides F_CPU, etc. */
#include "../mcal/mcal_registers.h"			/* Provides UDR, UCSRA/B/C, UBRRL/H */

/* ============================================
 * Bit Positions – USART Control & Status A (UCSRA)
 * ============================================ */
#define UCSRA_MPCM    0   /**< Multi-Processor Communication Mode */
#define UCSRA_U2X     1   /**< Double USART Transmission Speed */
#define UCSRA_PE      2   /**< Parity Error */
#define UCSRA_DOR     3   /**< Data OverRun */
#define UCSRA_FE      4   /**< Frame Error */
#define UCSRA_UDRE    5   /**< USART Data Register Empty */
#define UCSRA_TXC     6   /**< USART Transmit Complete */
#define UCSRA_RXC     7   /**< USART Receive Complete */

/* ============================================
 * Bit Positions – USART Control & Status B (UCSRB)
 * ============================================ */
#define UCSRB_TXB8    0   /**< Transmit Data Bit 8 */
#define UCSRB_RXB8    1   /**< Receive Data Bit 8 */
#define UCSRB_UCSZ2   2   /**< Character Size bit 2 */
#define UCSRB_TXEN    3   /**< Transmitter Enable */
#define UCSRB_RXEN    4   /**< Receiver Enable */
#define UCSRB_UDRIE   5   /**< USART Data Register Empty Interrupt Enable */
#define UCSRB_TXCIE   6   /**< TX Complete Interrupt Enable */
#define UCSRB_RXCIE   7   /**< RX Complete Interrupt Enable */

/* ============================================
 * Bit Positions – USART Control & Status C (UCSRC)
 * ============================================ */
#define UCSRC_UCPOL   0   /**< Clock Polarity (sync mode only) */
#define UCSRC_UCSZ0   1   /**< Character Size bit 0 */
#define UCSRC_UCSZ1   2   /**< Character Size bit 1 */
#define UCSRC_USBS    3   /**< Stop Bit Select: 0=1 bit, 1=2 bits */
#define UCSRC_UPM0    4   /**< Parity Mode bit 0 */
#define UCSRC_UPM1    5   /**< Parity Mode bit 1 */
#define UCSRC_UMSEL   6   /**< USART Mode Select: 0=Async, 1=Sync */
#define UCSRC_URSEL   7   /**< Register Select: 1 = UCSRC (see warning above) */

/* ============================================
 * Bit Positions – USART Baud Rate High (UBRRH)
 * ============================================ */
#define UBRRH_URSEL   7   /**< Register Select: 0 = UBRRH (see warning above) */

/* ============================================
 * Public Enums – USART
 * ============================================ */
typedef enum {
    USART_MODE_ASYNC = 0,   /**< Asynchronous (normal UART) */
    USART_MODE_SYNC  = 1    /**< Synchronous (clocked, SPI-like) */
} USART_Mode_t;

typedef enum {
    USART_PARITY_NONE = 0,  /**< No parity */
    USART_PARITY_EVEN = 2,  /**< Even parity */
    USART_PARITY_ODD  = 3   /**< Odd parity */
} USART_Parity_t;

typedef enum {
    USART_STOP_1BIT = 0,    /**< 1 stop bit */
    USART_STOP_2BIT = 1     /**< 2 stop bits */
} USART_StopBits_t;

typedef enum {
    USART_SIZE_5BIT = 0,    /**< 5 data bits */
    USART_SIZE_6BIT = 1,    /**< 6 data bits */
    USART_SIZE_7BIT = 2,    /**< 7 data bits */
    USART_SIZE_8BIT = 3,    /**< 8 data bits */
    USART_SIZE_9BIT = 4     /**< 9 data bits (UCSZ2=1 + UCSZ1:0=11) */
} USART_DataSize_t;

typedef enum {
    USART_SPEED_NORMAL = 0, /**< U2X = 0 (standard baud) */
    USART_SPEED_DOUBLE = 1  /**< U2X = 1 (double speed) */
} USART_Speed_t;

typedef enum {
    USART_FLAG_RX_COMPLETE,  /**< RXC  – receive complete */
    USART_FLAG_TX_COMPLETE,  /**< TXC  – transmit complete */
    USART_FLAG_DATA_EMPTY,   /**< UDRE – data register empty */
    USART_FLAG_FRAME_ERROR,  /**< FE   – frame error */
    USART_FLAG_DATA_OVERRUN, /**< DOR  – data overrun */
    USART_FLAG_PARITY_ERROR  /**< PE   – parity error */
} USART_Flag_t;

/* ============================================
 * USART – State (Opaque)
 * ============================================ */
typedef struct USART_State_t USART_State_t;
extern USART_State_t usart;


/* ============================================
 * USART – Public API
 * ============================================ */

/* ---- Configuration setters ---- */
void USART_vReset(void);
void USART_vSetMode(USART_Mode_t mode);
void USART_vSetParity(USART_Parity_t parity);
void USART_vSetStopBits(USART_StopBits_t stop);
void USART_vSetDataSize(USART_DataSize_t size);
void USART_vSetSpeed(USART_Speed_t speed);
void USART_vSetBaudRegister(u16_t ubrr);   /**< Direct UBRR value override */
void USART_vSetReceiverInt(bool_t enable);

/* ---- Configuration getters ---- */
USART_Mode_t USART_vGetMode(void);
USART_Parity_t USART_vGetParity(void);
USART_StopBits_t USART_vGetStopBits(void);
USART_DataSize_t USART_vGetDataSize(void);
USART_Speed_t USART_vGetSpeed(void);
u16_t USART_vGetBaudRegister(void);


/* ---- Lifecycle ---- */

/**
 * @brief   Apply the stored configuration to the USART hardware.
 *
 * @note    Order matters:
 *            1. Compute the baud rate
 *            2. Write UBRRH then UBRRL (URSEL = 0 for UBRRH)
 *            3. Write UCSRC (URSEL = 1)
 *            4. Enable TX/RX separately (USART_EnableTX/RX)
 *
 *          The transmitter and receiver are left DISABLED. Call
 *          USART_EnableTX() / USART_EnableRX() explicitly.
 */
void USART_Init(void); 

void USART_EnableRX(void);
void USART_DisableRX(void);
void USART_EnableTX(void);
void USART_DisableTX(void);


/* ---- Runtime (blocking) ---- */

/**
 * @brief   Send one byte. Blocks until the TX buffer is empty.
 * @param   data    Byte to transmit.
 */
void  USART_SendByte(u8_t data);

/**
 * @brief   Receive one byte. Blocks until a byte arrives.
 * @return  Received byte.
 */
u8_t  USART_ReceiveByte(void);

/**
 * @brief   Send a null-terminated string.
 * @param   str     Pointer to string.
 */
void  USART_SendString(const char* str);


/* ---- Status ---- */
bool_t USART_IsRxReady(void);               /**< TRUE if RXC set */
bool_t USART_IsTxReady(void);               /**< TRUE if UDRE set (buffer empty) */
bool_t USART_IsTxComplete(void);            /**< TRUE if TXC set */
bool_t USART_IsError(void);					/**< Check FE/DOR/PE */

/* ---- Interrupts ---- */
void USART_EnableRxInterrupt(void);         /**< RX complete (RXCIE) */
void USART_DisableRxInterrupt(void);
void USART_EnableTxInterrupt(void);         /**< TX complete (TXCIE) */
void USART_DisableTxInterrupt(void);
void USART_EnableDataEmptyInterrupt(void);  /**< UDRE (UDRIE) */
void USART_DisableDataEmptyInterrupt(void);

/**
 * @brief   Clear a USART status/interrupt flag.
 * @param   flag    Which flag to clear.
 *
 * @note    TXC is cleared by writing '1' to it (or by reading UCSRA then
 *          writing UDR). The other flags (FE, DOR, PE) are cleared
 *          automatically when UDR is read, so this function only
 *          meaningfully clears TXC.
 */
void USART_ClearInterruptFlag(USART_Flag_t flag);

#endif /* MCAL_USART_H_ */