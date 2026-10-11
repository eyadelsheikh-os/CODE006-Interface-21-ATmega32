/**
 * @file    mcal_spi.c
 * @brief   MCAL SPI Driver Implementation for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Implements master and slave SPI with full configurable
 *          clock rate, SPI mode (CPOL/CPHA), and data order.
 *          All register accesses go through mcal_registers.h.
 * 
 * @warning When configured as MASTER, this driver forces SS (PB4) to
 *          OUTPUT and drives it HIGH. Without this, the SPI hardware
 *          auto-switches to slave mode if SS is pulled LOW externally.
 */

#include "mcal_spi.h"
/* ============================================================
 * SPI – Private State
 * ============================================================ */
struct SPI_State {
    SPI_Mode_t        mode;
    SPI_DataOrder_t   order;
    SPI_ClockMode_t   clk_mode;
    SPI_ClockRate_t   clk_rate;
    bool_t            enable_int;
};
SPI_State_t spi;

/* ============================================================
 * Private Helpers
 * ============================================================ */

/**
 * @brief   Configure the SPI bus pins for the chosen mode.
 * @param   mode    SPI_MODE_MASTER or SPI_MODE_SLAVE.
 *
 * @note    Master: MOSI + SCK + SS are outputs (SS driven HIGH).
 *                  MISO is input.
 *          Slave : MISO is output. MOSI + SCK + SS are inputs.
 *
 *          Uses direct DDRB/PORTB manipulation — MCAL modules should
 *          not depend on each other (no calls into mcal_dio here).
 */
static inline void SPI_ConfigurePins(SPI_Mode_t mode) {
    if (mode == SPI_MODE_MASTER) {
        /* SS (PB4): OUTPUT, HIGH — mandatory to stay in master mode */
        SET_BIT(DDRB,  4);
        SET_BIT(PORTB, 4);

        /* MOSI (PB5): OUTPUT */
        SET_BIT(DDRB,  5);

        /* MISO (PB6): INPUT */
        CLR_BIT(DDRB,  6);

        /* SCK (PB7): OUTPUT */
        SET_BIT(DDRB,  7);
    } else {
        /* SS (PB4): INPUT */
        CLR_BIT(DDRB,  4);

        /* MOSI (PB5): INPUT */
        CLR_BIT(DDRB,  5);

        /* MISO (PB6): OUTPUT */
        SET_BIT(DDRB,  6);

        /* SCK (PB7): INPUT */
        CLR_BIT(DDRB,  7);
    }
}

/* ============================================================
 * SPI – Configuration Setters
 * ============================================================ */

void SPI_vReset(void) {
    spi.mode       = SPI_MODE_MASTER;
    spi.order      = SPI_ORDER_MSB_FIRST;
    spi.clk_mode   = SPI_MODE0;
    spi.clk_rate   = SPI_CLK_DIV4;
    spi.enable_int = FALSE;
}

void SPI_vSetMode(SPI_Mode_t mode)               { spi.mode       = mode; }
void SPI_vSetDataOrder(SPI_DataOrder_t order)    { spi.order      = order; }
void SPI_vSetClockMode(SPI_ClockMode_t clk_mode) { spi.clk_mode   = clk_mode; }
void SPI_vSetClockRate(SPI_ClockRate_t rate)     { spi.clk_rate   = rate; }
void SPI_vSetTransferInt(bool_t enable)          { spi.enable_int = enable; }

SPI_Mode_t SPI_vGetMode(void)				{ return spi.mode; }
SPI_DataOrder_t SPI_vGetDataOrder(void)		{ return spi.order; }
SPI_ClockMode_t SPI_vGetClockMode(void)		{ return spi.clk_mode; }
SPI_ClockRate_t SPI_vGetClockRate(void)		{ return spi.clk_rate; }


/* ============================================================
 * SPI – Lifecycle
 * ============================================================ */

void SPI_Init(void) {
    /* --- Configure pins (SS trap handled here) --- */
    SPI_ConfigurePins(spi.mode);

    /* --- Build SPCR --- */
    u8_t spcr_value = 0;

    if (spi.mode == SPI_MODE_MASTER) {
        SET_BIT(spcr_value, SPCR_MSTR);
    }

    if (spi.order == SPI_ORDER_LSB_FIRST) {
        SET_BIT(spcr_value, SPCR_DORD);
    }

    if (spi.clk_mode & 0x02) {
        SET_BIT(spcr_value, SPCR_CPOL);
    }
    if (spi.clk_mode & 0x01) {
        SET_BIT(spcr_value, SPCR_CPHA);
    }

    /* SPR1:0 */
    u8_t spr_bits, x2_bit;
	
	spr_bits = spi.clk_rate & 0x03;
	x2_bit  = (spi.clk_rate >= SPI_CLK_DIV2) ? 1 : 0;
	
    spcr_value |= (spr_bits << SPCR_SPR0);

    if (spi.enable_int) {
        SET_BIT(spcr_value, SPCR_SPIE);
    }

    /* --- Write control registers --- */
    SPCR = spcr_value;

    /* SPSR: only SPI2X matters for config */
    if (x2_bit) {
        SET_BIT(SPSR, SPSR_SPI2X);
    } else {
        CLR_BIT(SPSR, SPSR_SPI2X);
    }

    /* --- Clear any stale flags --- */
    (void)SPSR;
    (void)SPDR;

    /* --- Enable SPI --- */
    SPI_Enable();
}

void SPI_Enable(void)  { SET_BIT(SPCR, SPCR_SPE); }
void SPI_Disable(void) { CLR_BIT(SPCR, SPCR_SPE); }

/* ============================================================
 * SPI – Runtime
 * ============================================================ */

u8_t SPI_Transfer(u8_t data) {
    SPDR = data;
    while (!(SPSR & (1u << SPSR_SPIF)));
    return SPDR;
}

void SPI_SendByte(u8_t data) {
    (void)SPI_Transfer(data);
}

u8_t SPI_ReceiveByte(void) {
    return SPI_Transfer(0xFF);
}

void SPI_SendBuffer(const u8_t* buf, u16_t len) {
	for (u16_t i = 0; i < len; i++) (void)SPI_Transfer(buf[i]);
}

void SPI_ReceiveBuffer(u8_t* buf, u16_t len) {
	for (u16_t i = 0; i < len; i++) buf[i] = SPI_Transfer(0xFF);
}


/* ============================================================
 * SPI – Slave Mode Helpers
 * ============================================================ */

u8_t SPI_SlaveReceive(void) {
    while (!READ_BIT(SPSR, SPSR_SPIF));
    return SPDR;
}

void SPI_SlaveSend(u8_t data) {
    SPDR = data;
}

/* ============================================================
 * SPI – Status
 * ============================================================ */

bool_t SPI_IsTransferComplete(void) {
    return READ_BIT(SPSR, SPSR_SPIF) ? TRUE : FALSE;
}

bool_t SPI_HasWriteCollision(void) {
    return READ_BIT(SPSR, SPSR_WCOL) ? TRUE : FALSE;
}

/* ============================================================
 * SPI – Interrupts
 * ============================================================ */

void SPI_EnableInterrupt(void) {
    SET_BIT(SPCR, SPCR_SPIE);
    spi.enable_int = TRUE;
}

void SPI_DisableInterrupt(void) {
    CLR_BIT(SPCR, SPCR_SPIE);
    spi.enable_int = FALSE;
}

void SPI_ClearInterruptFlag(SPI_Flag_t flag) {
    switch (flag) {
        case SPI_FLAG_TRANSFER_COMPLETE:
        case SPI_FLAG_WRITE_COLLISION:
        default:
            /* Read SPSR first, then SPDR — clears both SPIF and WCOL */
            (void)SPSR;
            (void)SPDR;
            break;
    }
}