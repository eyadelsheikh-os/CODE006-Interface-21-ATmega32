/**
 * @file    mcal_usart.c
 * @brief   MCAL USART Driver Implementation for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Implements asynchronous and synchronous serial communication
 *          with configurable baud rate, parity, stop bits, and data size.
 *          All register accesses go through mcal_registers.h.
 * 
 * @warning UCSRC and UBRRH share I/O address 0x40. The URSEL bit (bit 7)
 *          selects which register the CPU writes to. This driver always
 *          writes URSEL correctly when touching either register.
 */

#include "mcal_usart.h"
/* ============================================================
 * USART – Private State
 * ============================================================ */
struct USART_State_t {
    USART_Mode_t      mode;
    USART_Parity_t    parity;
    USART_StopBits_t  stop_bits;
    USART_DataSize_t  data_size;
    USART_Speed_t     speed;
    u16_t             ubrr;         /* user-supplied UBRR value */
	bool_t			  enable_rx_int;
};
USART_State_t usart;

/* ============================================================
 * Private Helpers
 * ============================================================ */

/**
 * @brief   Write UBRRH safely (URSEL = 0).
 * @param   ubrr    12-bit UBRR value.
 *
 * @note    UBRRH and UCSRC share address 0x40. URSEL = 0 selects UBRRH.
 *          Only the low 4 bits of UBRR go into UBRRH.
 */
static void USART_WriteUBRRH(u8_t high) {
    UBRRH = high & 0x0F;
}

/**
 * @brief   Write UCSRC safely (URSEL = 1).
 * @param   value   7-bit config value (URSEL added internally).
 */
static void USART_WriteUCSRC(u8_t value) {
    UCSRC = (u8_t)(value | (1 << UCSRC_URSEL));
}

/**
 * @brief   Build the UCSRC byte (without URSEL) from the stored config.
 * @return  7-bit value to write to UCSRC (URSEL is set by the writer).
 */
static inline u8_t USART_BuildUCSRC(void) {
    u8_t val = 0;

    /* --- Mode (UMSEL) --- */
    if (usart.mode == USART_MODE_SYNC) {
        SET_BIT(val, UCSRC_UMSEL);
    }

    /* --- Parity (UPM1:0) --- */
    switch (usart.parity) {
        case USART_PARITY_NONE:
            /* 00 — nothing to set */
            break;
        case USART_PARITY_EVEN:
            /* 10 */
            SET_BIT(val, UCSRC_UPM1);
            break;
        case USART_PARITY_ODD:
            /* 11 */
            SET_BIT(val, UCSRC_UPM1);
            SET_BIT(val, UCSRC_UPM0);
            break;
    }

    /* --- Stop bits (USBS) --- */
    if (usart.stop_bits == USART_STOP_2BIT) {
        SET_BIT(val, UCSRC_USBS);
    }

    /* --- Character size (UCSZ1:0) --- */
    u8_t sz = (u8_t)(usart.data_size & 0x03);
    val |= (u8_t)(sz << UCSRC_UCSZ0);

    return val;
}


/* ============================================================
 * USART – Configuration Setters
 * ============================================================ */
void USART_vReset(void) {
    usart.mode          = USART_MODE_ASYNC;
    usart.parity        = USART_PARITY_NONE;
    usart.stop_bits     = USART_STOP_1BIT;
    usart.data_size     = USART_SIZE_8BIT;
    usart.speed         = USART_SPEED_NORMAL;
    usart.ubrr          = 0;
	usart.enable_rx_int	= FALSE;
}

void USART_vSetMode(USART_Mode_t mode)				{ usart.mode      = mode; }
void USART_vSetParity(USART_Parity_t parity)		{ usart.parity    = parity; }
void USART_vSetStopBits(USART_StopBits_t stop)		{ usart.stop_bits = stop; }
void USART_vSetDataSize(USART_DataSize_t size)		{ usart.data_size = size; }
void USART_vSetSpeed(USART_Speed_t speed)			{ usart.speed     = speed; }
void USART_vSetBaudRegister(u16_t ubrr)				{ usart.ubrr      = ubrr; }
void USART_vSetReceiverInt(bool_t enable)			{ usart.enable_rx_int = enable; }

USART_Mode_t USART_vGetMode(void)					{ return usart.mode; }
USART_Parity_t USART_vGetParity(void)				{ return usart.parity; }
USART_StopBits_t USART_vGetStopBits(void)			{ return usart.stop_bits; }
USART_DataSize_t USART_vGetDataSize(void)			{ return usart.data_size; }
USART_Speed_t USART_vGetSpeed(void)					{ return usart.speed; }
u16_t USART_vGetBaudRegister(void)					{ return usart.ubrr; }


/* ============================================================
 * USART – Lifecycle
 * ============================================================ */
void USART_Init(void) {
	
    /* --- 1. Baud rate --- */
    USART_WriteUBRRH((u8_t)(usart.ubrr >> 8));	// UBRRH must be written FIRST (URSEL = 0)
    UBRRL = (u8_t)(usart.ubrr & 0xFF);

    /* --- 2. UCSRC (URSEL = 1) --- */
    USART_WriteUCSRC(USART_BuildUCSRC());

    /* --- 3. Double-speed flag --- */
    if (usart.speed == USART_SPEED_DOUBLE &&
        usart.mode  == USART_MODE_ASYNC) {
        SET_BIT(UCSRA, UCSRA_U2X);
    } else {
        CLR_BIT(UCSRA, UCSRA_U2X);
    }

    /* --- 4. Receiver interrupt --- */
	if (usart.enable_rx_int)	SET_BIT(UCSRB, UCSRB_RXCIE);
	else						CLR_BIT(UCSRB, UCSRB_RXCIE);

	/* --- 5. TXEN/RXEN enable --- */
	USART_EnableRX();
	USART_EnableTX();
	
}

void USART_EnableRX(void)  { SET_BIT(UCSRB, UCSRB_RXEN); }
void USART_DisableRX(void) { CLR_BIT(UCSRB, UCSRB_RXEN); }
void USART_EnableTX(void)  { SET_BIT(UCSRB, UCSRB_TXEN); }
void USART_DisableTX(void) { CLR_BIT(UCSRB, UCSRB_TXEN); }

/* ============================================================
 * USART – Runtime (blocking I/O)
 * ============================================================ */
void USART_SendByte(u8_t data) {
	// Wait for UDR transmit buffer to be empty
	while (READ_BIT(UCSRA, UCSRA_UDRE) == 0);
    UDR = data;
}

u8_t USART_ReceiveByte(void) {
	// Wait for UDR receiver buffer to be filled with data
    while (READ_BIT(UCSRA, UCSRA_RXC) == 0);
	return UDR;
}

void USART_SendString(const char* str) {
    while (*str) {
        USART_SendByte((u8_t)*str++);
    }
}

/* ============================================================
 * USART – Status
 * ============================================================ */
bool_t USART_IsRxReady(void) {
    return READ_BIT(UCSRA, UCSRA_RXC) ? TRUE : FALSE;
}

bool_t USART_IsTxReady(void) {
    return READ_BIT(UCSRA, UCSRA_UDRE) ? TRUE : FALSE;
}

bool_t USART_IsTxComplete(void) {
    return READ_BIT(UCSRA, UCSRA_TXC) ? TRUE : FALSE;
}

bool_t USART_IsError(void) {
	return READ_BIT(UCSRA, UCSRA_FE) | READ_BIT(UCSRA, UCSRA_DOR) | READ_BIT(UCSRA, UCSRA_PE);
}


/* ============================================================
 * USART – Interrupts
 * ============================================================ */
void USART_EnableRxInterrupt(void)  { SET_BIT(UCSRB, UCSRB_RXCIE); }
void USART_DisableRxInterrupt(void) { CLR_BIT(UCSRB, UCSRB_RXCIE); }

void USART_EnableTxInterrupt(void)  { SET_BIT(UCSRB, UCSRB_TXCIE); }
void USART_DisableTxInterrupt(void) { CLR_BIT(UCSRB, UCSRB_TXCIE); }

void USART_EnableDataEmptyInterrupt(void)  { SET_BIT(UCSRB, UCSRB_UDRIE); }
void USART_DisableDataEmptyInterrupt(void) { CLR_BIT(UCSRB, UCSRB_UDRIE); }

void USART_ClearInterruptFlag(USART_Flag_t flag) {
    switch (flag) {
        case USART_FLAG_TX_COMPLETE:
            SET_BIT(UCSRA, UCSRA_TXC);
            break;
        /* FE, DOR, PE are cleared by reading UDR */
        default:
            break;
    }
}