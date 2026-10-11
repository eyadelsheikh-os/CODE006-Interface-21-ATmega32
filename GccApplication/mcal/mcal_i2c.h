/**
 * @file    mcal_i2c.h
 * @brief   MCAL I2C (TWI) Driver for ATmega32 — multi-master capable
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Supports true peer-to-peer (multi-master) operation on the
 *          ATmega32 TWI peripheral. The driver tracks its current role
 *          (master / slave / idle) so the single TWI ISR can handle
 *          both master state codes and slave state codes correctly.
 * 
 *          Two API families are exposed:
 * 
 *          1. NON-BLOCKING (recommended for p2p)
 *             I2C_SendAsync / I2C_RecvAsync
 *             Kick off a transfer; the ISR advances it. Application
 *             polls I2C_IsBusy() or waits on a completion callback.
 * 
 *          2. BLOCKING (for single-master setups)
 *             I2C_HighWriteByte / I2C_HighReadByte / I2C_HighWriteBuffer /
 *             I2C_HighReadBuffer
 *             Wraps the async calls with a spin-wait. Do NOT call these
 *             from an ISR or while TWIE is enabled and a peer might
 *             address you.
 * 
 *          Bus pins on ATmega32:
 *              SCL — PC0  (clock)
 *              SDA — PC1  (data)
 * 
 * @warning SCL and SDA are open-drain. External pull-up resistors
 *          (typically 4.7 k? to VCC) are REQUIRED. Without them the
 *          bus stays LOW and no communication occurs.
 * 
 * @warning In multi-master mode, do NOT disable TWIE around master
 *          operations. The driver relies on the ISR running for every
 *          event. If you disable TWIE, the driver stalls.
 */

#ifndef MCAL_I2C_H_
#define MCAL_I2C_H_
/* ============================================
 * Standard Library Includes
 * ============================================ */
#include "../common/std_types.h"			/* Provides u8_t, u16_t, bool_t */
#include "../common/std_macros.h"			/* Provides SET_BIT, CLR_BIT, etc. */
#include "../common/ud_config.h"			/* Provides F_CPU, etc. */
#include "../mcal/mcal_registers.h"			/* Provides TWBR, TWCR, TWSR, TWDR, TWAR */

/* ============================================
 * Special Macros
 * ============================================ */
#define I2C_TIMEOUT        60000UL   /**< Poll loop guard (~75 ms @ 8 MHz) */
#define I2C_RETRY_LIMIT    3         /**< Retries before async transfer fails */

/* ============================================
 * Bit Positions – TWI Control Register (TWCR)
 * ============================================ */
#define TWCR_TWIE     0   /**< TWI Interrupt Enable */
#define TWCR_TWEN     2   /**< TWI Enable */
#define TWCR_TWWC     3   /**< TWI Write Collision Flag */
#define TWCR_TWSTO    4   /**< TWI STOP Condition */
#define TWCR_TWSTA    5   /**< TWI START Condition */
#define TWCR_TWEA     6   /**< TWI Enable Acknowledge */
#define TWCR_TWINT    7   /**< TWI Interrupt Flag (write 1 to clear/start op) */

/* ============================================
 * Bit Positions – TWI Status Register (TWSR)
 * ============================================ */
#define TWSR_TWPS0    0   /**< Prescaler bit 0 */
#define TWSR_TWPS1    1   /**< Prescaler bit 1 */
/* Bits 3–7 hold the status code — read via I2C_GetStatus() */

/* ============================================
 * Bit Positions – TWI Slave Address (TWAR)
 * ============================================ */
#define TWAR_TWGCE    0   /**< General Call Enable */
/* Bits 1–7 hold the slave address */

/* ============================================
 * TWI Master Status Codes (TWSR & 0xF8)
 * ============================================ */
#define I2C_START_OK              0x08  /**< START transmitted */
#define I2C_REP_START_OK          0x10  /**< Repeated START transmitted */
#define I2C_MT_SLA_ACK            0x18  /**< SLA+W sent, ACK received */
#define I2C_MT_SLA_NACK           0x20  /**< SLA+W sent, NACK received */
#define I2C_MT_DATA_ACK           0x28  /**< Data sent, ACK received */
#define I2C_MT_DATA_NACK          0x30  /**< Data sent, NACK received */
#define I2C_MT_ARB_LOST           0x38  /**< Arbitration lost — another master won */
#define I2C_MR_SLA_ACK            0x40  /**< SLA+R sent, ACK received */
#define I2C_MR_SLA_NACK           0x48  /**< SLA+R sent, NACK received */
#define I2C_MR_DATA_ACK           0x50  /**< Data received, ACK returned */
#define I2C_MR_DATA_NACK          0x58  /**< Data received, NACK returned */

/* ============================================
 * TWI Slave Status Codes (TWSR & 0xF8)
 * ============================================ */
#define I2C_SR_SLA_ACK            0x60  /**< Own SLA+W received, ACK returned */
#define I2C_SR_ARB_LOST_SLA_ACK   0x68  /**< Arbitration lost, own SLA+W */
#define I2C_SR_GEN_ACK            0x70  /**< General call, ACK returned */
#define I2C_SR_ARB_LOST_GEN_ACK   0x78  /**< Arbitration lost, general call */
#define I2C_SR_DATA_ACK           0x80  /**< Data received, ACK returned */
#define I2C_SR_DATA_NACK          0x88  /**< Data received, NACK returned */
#define I2C_SR_GEN_DATA_ACK       0x90  /**< General call data, ACK */
#define I2C_SR_GEN_DATA_NACK      0x98  /**< General call data, NACK */
#define I2C_SR_STOP_REP_START     0xA0  /**< STOP or repeated START received */
#define I2C_SR_SLA_R_ACK          0xA8  /**< Own SLA+R received, ACK returned */
#define I2C_SR_ARB_LOST_SLA_R     0xB0  /**< Arbitration lost, own SLA+R */
#define I2C_SR_DATA_TX_ACK        0xB8  /**< Data transmitted, ACK received */
#define I2C_SR_DATA_TX_NACK       0xC0  /**< Data transmitted, NACK received */
#define I2C_SR_LAST_DATA_TX       0xC8  /**< Last data byte sent (TWEA=0) */
#define I2C_NO_STATE_INFO         0xF8  /**< No relevant state info */
#define I2C_BUS_ERROR             0x00  /**< Illegal START/STOP condition */

/* ============================================
 * Public Enums — I2C
 * ============================================ */
typedef enum {
    I2C_PRESCALER_1  = 0,  /**< TWPS1:0 = 00 */
    I2C_PRESCALER_4  = 1,  /**< TWPS1:0 = 01 */
    I2C_PRESCALER_16 = 2,  /**< TWPS1:0 = 10 */
    I2C_PRESCALER_64 = 3   /**< TWPS1:0 = 11 */
} I2C_Prescaler_t;

typedef enum {
    I2C_ACK  = 1,          /**< Acknowledge — continue transfer */
    I2C_NACK = 0           /**< Not Acknowledge — end transfer */
} I2C_Ack_t;

typedef enum {
    I2C_FLAG_INTERRUPT,    /**< TWINT — interrupt / operation complete */
    I2C_FLAG_WRITE_COLLISION /**< TWWC — write to TWDR while busy */
} I2C_Flag_t;

/**
 * @brief   Current role of this device on the bus.
 * @note    IDLE means "not currently in a master transaction".
 *          The device is still a passive slave listening for its address.
 */
typedef enum {
    I2C_ROLE_IDLE   = 0,   /**< Passive slave, waiting for events */
    I2C_ROLE_MASTER = 1,   /**< Currently driving a master transaction */
    I2C_ROLE_SLAVE  = 2    /**< Currently responding to an address match */
} I2C_XferRole_t;

typedef enum {
	I2C_DIR_WRITE = 0,     /**< Master writes to slave (SLA+W) */
	I2C_DIR_READ  = 1      /**< Master reads from slave (SLA+R) */
} I2C_XferDirection_t;

/**
 * @brief   Reason for the last async transfer completion.
 */
typedef enum {
	I2C_RESULT_NONE = 0,
	I2C_RESULT_OK,
	I2C_RESULT_NACK,        /**< Peer did not ACK */
	I2C_RESULT_ARB_LOST,    /**< Another master won the bus */
	I2C_RESULT_BUS_ERROR
} I2C_XferResult_t;

/**
 * @brief   Callback invoked when an async transfer completes.
 * @param   result  I2C_RESULT_*
 * @return  next	Required action after completing a transaction and before stopping.
 * @note    Called from the TWI ISR. Keep it short — no blocking I/O.
 */
typedef void (*I2C_MasterCallback_t)(I2C_XferResult_t result);
typedef void (*I2C_SlaveRxCallback_t)(u8_t byte);
typedef u8_t (*I2C_SlaveTxCallback_t)(void);
typedef void (*I2C_SlaveStopCallback_t)(void);

/* ============================================
 * I2C — State (Opaque)
 * ============================================ */
typedef struct I2C_State I2C_State_t;
extern I2C_State_t i2c;

/* ============================================
 * I2C — Configuration Setters / Getters
 * ============================================ */
void I2C_vReset(void);
void I2C_vSetPrescaler(I2C_Prescaler_t prescaler);
void I2C_vSetBitRate(u8_t twbr);
void I2C_vSetFrequency(u32_t freq_hz);
void I2C_vSetAddress(u8_t addr7);           /**< This device's own address */
void I2C_vSetGeneralCall(bool_t enable);
void I2C_vSetInterrupt(bool_t enable);      /**< TWIE. Should stay TRUE for p2p */

I2C_Prescaler_t I2C_vGetPrescaler(void);
u8_t            I2C_vGetBitRate(void);
u8_t            I2C_vGetAddress(void);
I2C_XferRole_t  I2C_vGetRole(void);

/* ============================================
 * I2C — Lifecycle
 * ============================================ */
/**
 * @brief   Apply stored configuration to the TWI hardware.
 *
 * @note    In p2p mode, keep TWIE enabled permanently.
 *          The driver owns the ISR.
 */
void I2C_Init(void);

void I2C_Enable(void);      /**< TWEN = 1 */
void I2C_Disable(void);     /**< TWEN = 0 */

/* ============================================
 * I2C — Non-Blocking Master API (p2p-safe)
 * ============================================ */

void I2C_SetSlaveCallbacks(I2C_SlaveRxCallback_t rx, I2C_SlaveTxCallback_t tx, I2C_SlaveStopCallback_t sx);

/**
 * @brief   TRUE while a master transaction is in progress.
 */
bool_t I2C_IsBusy(void);

/**
 * @brief   Retrieve the result of the last completed async transfer.
 * @note    Reading this clears the completion flag.
 */
I2C_XferResult_t I2C_GetTransferResult(void);

/**
 * @brief   Cancel an in-progress transfer and release the bus.
 */
void I2C_MasterAbort(void);



/**
 * @brief   Queue an outgoing write transfer.
 * @param   addr7   7-bit peer address.
 * @param   buf     Bytes to send. Must remain valid until the callback fires.
 * @param   len     Number of bytes (1..N).
 * @return  TRUE if the transfer was accepted; FALSE if the driver is busy.
 *
 * @note    Non-blocking. Returns immediately. The TWI ISR advances the
 *          transaction byte by byte. The application must not call
 *          I2C_SendAsync again until I2C_IsBusy() returns FALSE.
 */
bool_t I2C_SendAsync(u8_t addr7, const u8_t* buf, u16_t len, I2C_MasterCallback_t mcb);

/**
 * @brief   Queue an incoming read transfer.
 * @param   addr7   7-bit peer address.
 * @param   buf     Destination buffer. Must remain valid until the callback.
 * @param   len     Number of bytes to read.
 * @return  TRUE if accepted; FALSE if the driver is busy.
 */
bool_t I2C_RecvAsync(u8_t addr7, u8_t* buf, u16_t len, I2C_MasterCallback_t mcb);

/* ============================================
 * I2C — Legacy Blocking API (single-master only)
 * ============================================ */
/**
 * @warning Do NOT use these in p2p mode. They spin the CPU and prevent
 *          the slave side of the driver from responding to incoming
 *          transactions. Use the async API instead.
 */

bool_t I2C_Start(void);       /**< Blocking START */
bool_t I2C_Restart(void);     /**< Blocking repeated START */
void   I2C_Stop(void);        /**< Fire-and-forget STOP */

bool_t I2C_WriteAddress(u8_t addr7, I2C_XferDirection_t dir);
bool_t I2C_WriteByte(u8_t data);
u8_t   I2C_ReadByte(I2C_Ack_t ack);

bool_t I2C_HighProbe(u8_t addr7);
bool_t I2C_HighWriteByte(u8_t addr7, u8_t data);
bool_t I2C_HighReadByte(u8_t addr7, u8_t* data);
bool_t I2C_HighWriteBuffer(u8_t addr7, const u8_t* buf, u16_t len);
bool_t I2C_HighReadBuffer(u8_t addr7, u8_t* buf, u16_t len);

/* ============================================
 * I2C — Slave ISR Helpers
 * ============================================ */
/**
 * @brief   Read the byte that just arrived in TWDR.
 * @return  The latched byte. Safe to call only when TWINT is set
 *          (i.e. from the TWI ISR).
 */
u8_t I2C_SlaveReadByte(void);

/**
 * @brief   Preload TWDR with the next byte the master will read.
 * @param   data    Byte to make available.
 * @note    Does NOT touch TWCR — the ISR is responsible for writing
 *          the appropriate TWCR value afterward (via I2C_Ack / I2C_Nack).
 */
void I2C_SlaveWriteByte(u8_t data);

/**
 * @brief   Acknowledge the current TWI event and advance the state machine.
 * @note    Must be called exactly once per ISR entry.
 */
void I2C_Ack(void);

/**
 * @brief   NACK the current event. Used to end slave reads cleanly.
 */
void I2C_Nack(void);

/* ============================================
 * I2C — Status / Flags
 * ============================================ */
u8_t  I2C_GetStatus(void);              /**< TWSR & 0xF8 */
bool_t I2C_IsOperationComplete(void);   /**< TWINT set */
bool_t I2C_HasWriteCollision(void);     /**< TWWC set */

/* ============================================
 * I2C — Interrupt Control
 * ============================================ */
void I2C_EnableInterrupt(void);
void I2C_DisableInterrupt(void);
void I2C_ClearInterruptFlag(I2C_Flag_t flag);

/* Unified event handler — call from your ISR 
 * Handles BOTH master async transfers and slave responses.
 * The application's ISR becomes one line:
 * ISR(TWI_vect) { I2C_OnEvent(); } */
void I2C_OnEvent(void);

#endif /* MCAL_I2C_H_ */