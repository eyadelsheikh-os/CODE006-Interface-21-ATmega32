/**
 * @file    mcal_spi.h
 * @brief   MCAL SPI (Serial Peripheral Interface) Driver for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Provides low-level functions to configure and control the
 *          on-chip SPI peripheral. Supports master and slave modes,
 *          all four SPI modes (CPOL/CPHA combinations), MSB/LSB-first
 *          data order, and 7 clock rates.
 * 
 *          Bus pins on ATmega32:
 *              SS   – PB4  (slave select, active LOW)
 *              MOSI – PB5  (master out, slave in)
 *              MISO – PB6  (master in, slave out)
 *              SCK  – PB7  (serial clock)
 * 
 * @warning When configured as MASTER, the SS pin (PB4) must be kept
 *          as OUTPUT (or tied HIGH) — if it is left as INPUT and pulled
 *          LOW externally, the hardware will automatically switch the
 *          SPI to slave mode and corrupt the transfer.
 */

#ifndef MCAL_SPI_H_
#define MCAL_SPI_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"			/* Provides u8_t, u16_t, bool_t */
#include "../common/std_macros.h"			/* Provides SET_BIT, CLR_BIT, etc. */
#include "../common/ud_config.h"			/* Provides F_CPU, etc. */
#include "../mcal/mcal_registers.h"			/* Provides SPCR, SPSR, SPDR */

/* ============================================
 * Bit Positions – SPI Control Register (SPCR)
 * ============================================ */
#define SPCR_SPR0    0   /**< SPI Clock Rate Select bit 0 */
#define SPCR_SPR1    1   /**< SPI Clock Rate Select bit 1 */
#define SPCR_CPHA    2   /**< Clock Phase: 0=leading, 1=trailing */
#define SPCR_CPOL    3   /**< Clock Polarity: 0=idle LOW, 1=idle HIGH */
#define SPCR_MSTR    4   /**< Master/Slave Select: 1=master, 0=slave */
#define SPCR_DORD    5   /**< Data Order: 0=MSB first, 1=LSB first */
#define SPCR_SPE     6   /**< SPI Enable */
#define SPCR_SPIE    7   /**< SPI Interrupt Enable */

/* ============================================
 * Bit Positions – SPI Status Register (SPSR)
 * ============================================ */
#define SPSR_SPI2X   0   /**< Double SPI Speed (master mode only) */
#define SPSR_WCOL    6   /**< Write Collision Flag */
#define SPSR_SPIF    7   /**< SPI Interrupt / Transfer Complete Flag */

/* ============================================
 * Public Enums – SPI
 * ============================================ */
typedef enum {
    SPI_MODE_SLAVE  = 0,   /**< Slave mode (MSTR = 0) */
    SPI_MODE_MASTER = 1    /**< Master mode (MSTR = 1) */
} SPI_Mode_t;

typedef enum {
    SPI_ORDER_MSB_FIRST = 0,   /**< DORD = 0 — MSB shifted out first */
    SPI_ORDER_LSB_FIRST = 1    /**< DORD = 1 — LSB shifted out first */
} SPI_DataOrder_t;

typedef enum {
    SPI_MODE0 = 0,   /**< CPOL=0, CPHA=0 — idle LOW,  sample on leading edge */
    SPI_MODE1 = 1,   /**< CPOL=0, CPHA=1 — idle LOW,  sample on trailing edge */
    SPI_MODE2 = 2,   /**< CPOL=1, CPHA=0 — idle HIGH, sample on leading edge */
    SPI_MODE3 = 3    /**< CPOL=1, CPHA=1 — idle HIGH, sample on trailing edge */
} SPI_ClockMode_t;

typedef enum {
    SPI_CLK_DIV4   = 0,   /**< F_CPU / 4   (SPI2X=0, SPR1:0=00) */
    SPI_CLK_DIV16  = 1,   /**< F_CPU / 16  (SPI2X=0, SPR1:0=01) */
    SPI_CLK_DIV64  = 2,   /**< F_CPU / 64  (SPI2X=0, SPR1:0=10) */
    SPI_CLK_DIV128 = 3,   /**< F_CPU / 128 (SPI2X=0, SPR1:0=11) */
    SPI_CLK_DIV2   = 4,   /**< F_CPU / 2   (SPI2X=1, SPR1:0=00) */
    SPI_CLK_DIV8   = 5,   /**< F_CPU / 8   (SPI2X=1, SPR1:0=01) */
    SPI_CLK_DIV32  = 6    /**< F_CPU / 32  (SPI2X=1, SPR1:0=10) */
    /* value 7 (SPI2X=1, SPR1:0=11) ? F_CPU / 64, same as DIV64 */
} SPI_ClockRate_t;

typedef enum {
    SPI_FLAG_TRANSFER_COMPLETE,   /**< SPIF — transfer complete */
    SPI_FLAG_WRITE_COLLISION      /**< WCOL — write to SPDR while busy */
} SPI_Flag_t;

/* ============================================
 * SPI – State (Opaque)
 * ============================================ */
typedef struct SPI_State SPI_State_t;
extern SPI_State_t spi;

/* ============================================
 * SPI – Public API
 * ============================================ */
/* ---- Configuration setters ---- */
void SPI_vReset(void);
void SPI_vSetMode(SPI_Mode_t mode);
void SPI_vSetDataOrder(SPI_DataOrder_t order);
void SPI_vSetClockMode(SPI_ClockMode_t clk_mode);
void SPI_vSetClockRate(SPI_ClockRate_t rate);
void SPI_vSetTransferInt(bool_t enable);

/* ---- Configuration getters ---- */
SPI_Mode_t SPI_GetMode(void);
SPI_DataOrder_t SPI_vGetDataOrder(void);
SPI_ClockMode_t SPI_vGetClockMode(void);
SPI_ClockRate_t SPI_vGetClockRate(void);

/* ---- Lifecycle ---- */


/**
 * @brief   Apply the stored configuration to the SPI hardware.
 *
 * @note    Performs pin configuration first (SS trap), then writes
 *          SPCR and SPSR. SPI is left ENABLED (SPE = 1) at the end.
 *          SPIF and WCOL are cleared before returning.
 */
void SPI_Init(void);        /**< Apply stored config to hardware */

void SPI_Enable(void);      /**< SPE = 1 */
void SPI_Disable(void);     /**< SPE = 0 */


/* ---- Runtime (blocking, master mode) ---- */

/**
 * @brief   Full-duplex byte exchange: sends one byte, returns the received one.
 * @param   data    Byte to transmit.
 * @return  Byte received from the slave.
 *
 * @note    In master mode this function drives SCK, so it must be called
 *          from the master only. In slave mode, use SPI_ReceiveByte /
 *          SPI_SendByte instead.
 *
 * @warning Blocks until SPIF is set (transfer done). If the master's
 *          SS pin is pulled LOW by accident, this call will block forever
 *          (SPI switches to slave mode and no clock is generated).
 */
u8_t SPI_Transfer(u8_t data);

/**
 * @brief   Master-only: transmit a byte, discard the reply.
 */
void  SPI_SendByte(u8_t data);

/**
 * @brief   Master-only: send a dummy byte (0xFF) and return what came back.
 *
 * @note    Some slaves require a particular dummy value (0x00, 0xFF, or
 *          a specific command byte). Adjust if your peripheral cares.
 */
u8_t  SPI_ReceiveByte(void);

/**
 * @brief   Master: transmit a buffer.
 * @param   buf     Pointer to bytes to send.
 * @param   len     Number of bytes.
 */
void SPI_SendBuffer(const u8_t* buf, u16_t len);

/**
 * @brief   Master: receive a buffer by clocking out dummy 0xFF bytes.
 * @param   buf     Destination buffer.
 * @param   len     Number of bytes to receive.
 */
void SPI_ReceiveBuffer(u8_t* buf, u16_t len);


/* ---- Slave mode helpers ---- */

/**
 * @brief   Slave: wait for the master to clock a byte in, return it.
 * @return  The byte the master sent.
 *
 * @note    Reads SPSR then SPDR, which clears SPIF.
 */
u8_t SPI_SlaveReceive(void);

/**
 * @brief   Slave: preload SPDR with the byte the master should read next.
 * @param   data    Byte to make available for the next master-initiated transfer.
 *
 * @note    In slave mode, SPDR serves both TX and RX. Write here when
 *          the previous transfer is complete (SPIF = 1); the master's
 *          next clock burst will shift this byte out.
 */
void SPI_SlaveSend(u8_t data);

/* ---- Status ---- */
bool_t SPI_IsTransferComplete(void);   /**< SPIF flag */
bool_t SPI_HasWriteCollision(void);    /**< WCOL flag */

/* ---- Interrupts ---- */
void SPI_EnableInterrupt(void);        /**< SPIE = 1 */
void SPI_DisableInterrupt(void);       /**< SPIE = 0 */

/**
 * @brief   Clear a SPI flag.
 * @param   flag    SPI_FLAG_TRANSFER_COMPLETE or SPI_FLAG_WRITE_COLLISION.
 *
 * @note    SPIF is cleared by reading SPSR (with SPIF set), then reading
 *          SPDR. WCOL is cleared the same way.
 *          In practice, calling SPI_Transfer() clears both implicitly.
 */
void SPI_ClearInterruptFlag(SPI_Flag_t flag);

#endif /* MCAL_SPI_H_ */