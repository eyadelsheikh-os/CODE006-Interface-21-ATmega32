/**
 * @file    mcal_i2c.c
 * @brief   MCAL I2C (TWI) Driver Implementation for ATmega32
 * @author  eyads
 * @date    2026-05-07
 * 
 * @note    Multi-master capable. The ISR owns the TWI peripheral for both
 *          master and slave events. Application-level slave handling is done
 *          through callbacks registered with I2C_SetSlaveCallbacks().
 * 
 *          Never write TWCR with |= — always assign the full value. Some
 *          bits are sticky config (TWEN, TWIE, TWEA) and must be re-included
 *          on every write, otherwise they get cleared.
 */

#include "mcal_i2c.h"
/* ============================================================
 * I2C — Private State
 * ============================================================ */
struct I2C_State {
    /* ---- Configuration ---- */
    I2C_Prescaler_t  prescaler;
    u8_t             twbr;
    u8_t             own_addr;
    bool_t           general_call;
    bool_t           enable_int;

    /* ---- Async transfer ---- */
    I2C_XferRole_t			xfer_role;
    I2C_XferDirection_t		xfer_dir;
    I2C_XferResult_t		xfer_result;
    u8_t					xfer_addr;
    u8_t*					xfer_buf;
    u16_t					xfer_len;
    u16_t					xfer_idx;
    I2C_MasterCallback_t	xfer_mcb;
    I2C_SlaveRxCallback_t	xfer_srcb;
    I2C_SlaveTxCallback_t	xfer_stcb;
	I2C_SlaveStopCallback_t xfer_sscb;
};
I2C_State_t i2c;

/* ============================================================
 * Private Helpers
 * ============================================================ */

/**
 * @brief   Build a full TWCR value with TWINT + TWEN + optional TWIE,
 *          plus any caller-supplied flag bits.
 */
static inline u8_t I2C_BuildTWCR(u8_t flags) {
    u8_t val = (1 << TWCR_TWINT) | (1 << TWCR_TWEN) | flags;
    if (i2c.enable_int) SET_BIT(val, TWCR_TWIE);
    return val;
}

/**
 * @brief   Poll TWINT until set or timeout.
 */
static bool_t I2C_WaitFlag(void) {
    u16_t guard = 0;
    while (!READ_BIT(TWCR, TWCR_TWINT)) {
        if (++guard >= I2C_TIMEOUT) return FALSE;
    }
    return TRUE;
}

/**
 * @brief   Read the 5-bit status code from TWSR.
 */
static inline u8_t I2C_ReadStatus(void) {
    return (u8_t)(TWSR & 0xF8);
}

/**
 * @brief   Compute TWBR from a desired SCL frequency.
 */
static u8_t I2C_ComputeTWBR(u32_t freq) {
    if (freq == 0) return 0;

    u32_t presc   = 1UL << (2 * i2c.prescaler);
    u32_t divisor = 2UL * presc;
    u32_t fcpu    = (u32_t)F_CPU;

    if (fcpu / freq <= 16UL) return 0;

    u32_t twbr = (fcpu / freq - 16UL) / divisor;
    if (twbr > 255UL) return 0;
    return (u8_t)twbr;
}

/**
 * @brief   Finish the current async transfer and notify the caller.
 */
static void I2C_CompleteTransfer(I2C_XferResult_t result)
{
    I2C_MasterCallback_t mcb = i2c.xfer_mcb;
    i2c.xfer_mcb = NULL;
	i2c.xfer_result = result;

	if (result == I2C_RESULT_ARB_LOST){
		TWCR = I2C_BuildTWCR(1 << TWCR_TWEA);
	} else {
		TWCR = I2C_BuildTWCR((1 << TWCR_TWSTO) | (1 << TWCR_TWEA));
	}
	
	i2c.xfer_role = I2C_ROLE_IDLE;
	if (mcb) mcb(result);
}

static bool_t I2C_StartAsync(u8_t addr7, u8_t* buf, u16_t len, I2C_XferDirection_t dir, I2C_MasterCallback_t mcb)
{
	if (i2c.xfer_role != I2C_ROLE_IDLE)		return FALSE;
	if (len == 0)							return FALSE;

	i2c.xfer_addr   = addr7;
	i2c.xfer_buf    = buf;
	i2c.xfer_len    = len;
	i2c.xfer_idx    = 0;
	i2c.xfer_dir    = dir;
	i2c.xfer_mcb	= mcb;
	i2c.xfer_result = I2C_RESULT_NONE;
	i2c.xfer_role   = I2C_ROLE_MASTER;
	
	/* Kick off the START condition — the ISR takes over from here. */
	TWCR = I2C_BuildTWCR(1 << TWCR_TWSTA);
	return TRUE;
}

/* ============================================================
 * Configuration Setters / Getters
 * ============================================================ */
void I2C_vReset(void) {
    i2c.prescaler		= I2C_PRESCALER_1;
    i2c.twbr			= 32;         /* ~100 kHz at 8 MHz, prescaler 1 */
    i2c.own_addr		= 0x08;		  /* (0x00-0x07 general call, 0x50-0x57 EEPROM, 0x78-0x7F reserved) */
    i2c.general_call	= FALSE;
    i2c.enable_int		= FALSE;

    /* ---- Async transfer ---- */
	i2c.xfer_role		= I2C_ROLE_IDLE;
    i2c.xfer_result		= I2C_RESULT_NONE;
    i2c.xfer_mcb		= NULL;
    i2c.xfer_srcb		= NULL;
    i2c.xfer_stcb		= NULL;
    i2c.xfer_sscb		= NULL;
}

void I2C_vSetPrescaler(I2C_Prescaler_t p) { i2c.prescaler		= p; }
void I2C_vSetBitRate(u8_t twbr)           { i2c.twbr			= twbr; }
void I2C_vSetAddress(u8_t addr7)          { i2c.own_addr		= (addr7 & 0x7F); }
void I2C_vSetGeneralCall(bool_t enable)   { i2c.general_call	= enable; }
void I2C_vSetInterrupt(bool_t enable)     { i2c.enable_int		= enable; }

void I2C_vSetFrequency(u32_t freq_hz) {
    i2c.twbr = I2C_ComputeTWBR(freq_hz);
}


I2C_Prescaler_t I2C_vGetPrescaler(void) { return i2c.prescaler; }
u8_t            I2C_vGetBitRate(void)   { return i2c.twbr; }
u8_t            I2C_vGetAddress(void)   { return i2c.own_addr; }
I2C_XferRole_t  I2C_vGetRole(void)      { return i2c.xfer_role; }


/* ============================================================
 * Lifecycle
 * ============================================================ */
void I2C_Init(void) {
    /* Prescaler (TWSR bits 1:0) */
    TWSR = (TWSR & 0xFC) | (i2c.prescaler & 0x03);

    /* Bit rate */
    TWBR = i2c.twbr;

    /* Slave address (only meaningful when addressed by another master) */
    TWAR = (u8_t)((i2c.own_addr << 1) |
				  (i2c.general_call ? (1 << TWAR_TWGCE) : 0));

    /* Build TWCR once: TWEN + TWEA + optional TWIE, TWINT cleared */
    u8_t val = (1 << TWCR_TWEN) | (1 << TWCR_TWEA) | (1 << TWCR_TWINT);
    if (i2c.enable_int) SET_BIT(val, TWCR_TWIE);
    TWCR = val;
}

void I2C_Enable(void)  { SET_BIT(TWCR, TWCR_TWEN); }
void I2C_Disable(void) { CLR_BIT(TWCR, TWCR_TWEN); }

/* ============================================================
 * Non-Blocking Async Master API
 * ============================================================ */
/* Register the slave callbacks */
void I2C_SetSlaveCallbacks(I2C_SlaveRxCallback_t rx, I2C_SlaveTxCallback_t tx, I2C_SlaveStopCallback_t sx)
{
	i2c.xfer_srcb = rx;
	i2c.xfer_stcb = tx;
	i2c.xfer_sscb = sx;
}

I2C_XferResult_t I2C_GetTransferResult(void) {
	I2C_XferResult_t r = i2c.xfer_result;
	i2c.xfer_result = I2C_RESULT_NONE;
	return r;
}

bool_t I2C_IsBusy(void) {
    return (i2c.xfer_role == I2C_ROLE_IDLE) ? FALSE : TRUE;
}

void I2C_MasterAbort(void) {
	if (i2c.xfer_role != I2C_ROLE_MASTER) return;
	I2C_CompleteTransfer(I2C_RESULT_NONE);
}

bool_t I2C_SendAsync(u8_t addr7, const u8_t* buf, u16_t len, I2C_MasterCallback_t mcb)
{	
    return I2C_StartAsync(addr7, (u8_t*)buf, len, I2C_DIR_WRITE, mcb);
}

bool_t I2C_RecvAsync(u8_t addr7, u8_t* buf, u16_t len, I2C_MasterCallback_t mcb)
{
    return I2C_StartAsync(addr7, buf, len, I2C_DIR_READ, mcb);
}


/* ============================================================
 * Blocking Master API (single-master only)
 * ============================================================ */
bool_t I2C_Start(void) {
    TWCR = I2C_BuildTWCR(1 << TWCR_TWSTA);
    if (!I2C_WaitFlag()) return FALSE;

    u8_t s = I2C_ReadStatus();
    return (s == I2C_START_OK || s == I2C_REP_START_OK) ? TRUE : FALSE;
}

bool_t I2C_Restart(void) { return I2C_Start(); }

void I2C_Stop(void) {
    TWCR = I2C_BuildTWCR(1 << TWCR_TWSTO);
    /* Fire-and-forget; hardware releases the bus autonomously. */
}

bool_t I2C_WriteAddress(u8_t addr7, I2C_XferDirection_t dir) {
    TWDR = (u8_t)((addr7 << 1) | (dir & 0x01));
    TWCR = I2C_BuildTWCR(0);

    if (!I2C_WaitFlag()) return FALSE;

    u8_t s = I2C_ReadStatus();
    if (s == I2C_MT_ARB_LOST) { I2C_Stop(); return FALSE; }

    if (dir == I2C_DIR_WRITE) return (s == I2C_MT_SLA_ACK) ? TRUE : FALSE;
    else                      return (s == I2C_MR_SLA_ACK) ? TRUE : FALSE;
}

bool_t I2C_WriteByte(u8_t data) {
    TWDR = data;
    TWCR = I2C_BuildTWCR(0);

    if (!I2C_WaitFlag()) return FALSE;

    u8_t s = I2C_ReadStatus();
    if (s == I2C_MT_ARB_LOST) { I2C_Stop(); return FALSE; }
    return (s == I2C_MT_DATA_ACK) ? TRUE : FALSE;
}

u8_t I2C_ReadByte(I2C_Ack_t ack) {
    u8_t flags = (ack == I2C_ACK) ? (u8_t)(1 << TWCR_TWEA) : 0;
    TWCR = I2C_BuildTWCR(flags);

    (void)I2C_WaitFlag();
    return TWDR;
}

/* ============================================================
 * High-Level Blocking Wrappers (single-master only)
 * ============================================================ */
bool_t I2C_HighProbe(u8_t addr7) {
    if (!I2C_Start()) return FALSE;
    bool_t ok = I2C_WriteAddress(addr7, I2C_DIR_WRITE);
    I2C_Stop();
    return ok;
}

bool_t I2C_HighWriteByte(u8_t addr7, u8_t data) {
    if (!I2C_Start())                            return FALSE;
    if (!I2C_WriteAddress(addr7, I2C_DIR_WRITE)) { I2C_Stop(); return FALSE; }
    if (!I2C_WriteByte(data))                    { I2C_Stop(); return FALSE; }
    I2C_Stop();
    return TRUE;
}

bool_t I2C_HighReadByte(u8_t addr7, u8_t* data) {
    if (!I2C_Start())                            return FALSE;
    if (!I2C_WriteAddress(addr7, I2C_DIR_READ))  { I2C_Stop(); return FALSE; }
    *data = I2C_ReadByte(I2C_NACK);
    I2C_Stop();
    return TRUE;
}

bool_t I2C_HighWriteBuffer(u8_t addr7, const u8_t* buf, u16_t len) {
    if (len == 0)		return TRUE;
    if (!I2C_Start())	return FALSE;
    if (!I2C_WriteAddress(addr7, I2C_DIR_WRITE)) { I2C_Stop(); return FALSE; }
    for (u16_t i = 0; i < len; i++) {
        if (!I2C_WriteByte(buf[i])) { I2C_Stop(); return FALSE; }
    }
    I2C_Stop();
    return TRUE;
}

bool_t I2C_HighReadBuffer(u8_t addr7, u8_t* buf, u16_t len) {
    if (len == 0) return TRUE;
    if (!I2C_Start())                            return FALSE;
    if (!I2C_WriteAddress(addr7, I2C_DIR_READ))  { I2C_Stop(); return FALSE; }
    for (u16_t i = 0; i < len; i++) {
        I2C_Ack_t ack = (i == (len - 1)) ? I2C_NACK : I2C_ACK;
        buf[i] = I2C_ReadByte(ack);
    }
    I2C_Stop();
    return TRUE;
}

/* ============================================================
 * Slave Helpers (called from the ISR)
 * ============================================================ */
u8_t I2C_SlaveReadByte(void) { return TWDR; }

void I2C_SlaveWriteByte(u8_t data)  { TWDR = data; }

void I2C_Ack(void) {
    TWCR = I2C_BuildTWCR(1 << TWCR_TWEA);
}

void I2C_Nack(void) {
    TWCR = I2C_BuildTWCR(0);
}

/* ============================================================
 * Status / Interrupt Helpers
 * ============================================================ */
u8_t I2C_GetStatus(void)              { return I2C_ReadStatus(); }
bool_t I2C_IsOperationComplete(void)  { return READ_BIT(TWCR, TWCR_TWINT) ? TRUE : FALSE; }
bool_t I2C_HasWriteCollision(void)    { return READ_BIT(TWCR, TWCR_TWWC)  ? TRUE : FALSE; }

void I2C_EnableInterrupt(void)  { SET_BIT(TWCR, TWCR_TWIE); i2c.enable_int = TRUE;  }
void I2C_DisableInterrupt(void) { CLR_BIT(TWCR, TWCR_TWIE); i2c.enable_int = FALSE; }

void I2C_ClearInterruptFlag(I2C_Flag_t flag) {
    switch (flag) {
        case I2C_FLAG_INTERRUPT:
            SET_BIT(TWCR, TWCR_TWINT);
            break;
        case I2C_FLAG_WRITE_COLLISION:
            /* TWWC clears on TWDR write or TWINT clear */
            break;
        default: break;
    }
}

/* ============================================================
 * Unified Event Handler — call this from your ISR(TWI_vect)
 * ============================================================ */
void I2C_OnEvent(void) {
    u8_t status = I2C_ReadStatus();

    /* --------------------------------------------------------
     * MASTER-ROLE events
     * -------------------------------------------------------- */
	if (status <= I2C_MR_DATA_NACK) i2c.xfer_role = I2C_ROLE_MASTER;
	else							i2c.xfer_role = I2C_ROLE_SLAVE;
    if (i2c.xfer_role == I2C_ROLE_MASTER) {
        switch (status) {

            /* START or repeated START transmitted */
            case I2C_START_OK:
            case I2C_REP_START_OK:
                TWDR = (u8_t)((i2c.xfer_addr << 1) | (i2c.xfer_dir & 0x01));
                TWCR = I2C_BuildTWCR(0);
                return;

            /* Address written, slave ACKed */
            case I2C_MT_SLA_ACK:
                /* Send first data byte */
                TWDR = i2c.xfer_buf[0];
                i2c.xfer_idx   = 1;
                TWCR = I2C_BuildTWCR(0);
                return;

            /* Address written, slave NACKed */
            case I2C_MT_SLA_NACK:
                I2C_CompleteTransfer(I2C_RESULT_NACK);
                return;

            /* Data byte ACKed */
            case I2C_MT_DATA_ACK:
                if (i2c.xfer_idx < i2c.xfer_len) {
                    TWDR = i2c.xfer_buf[i2c.xfer_idx++];
                    TWCR = I2C_BuildTWCR(0);
                } else {
                    I2C_CompleteTransfer(I2C_RESULT_OK);
                }
                return;

			/* Data byte NACKed */
			case I2C_MT_DATA_NACK:
                I2C_CompleteTransfer(I2C_RESULT_NACK);
                return;

            /* Address read, slave ACKed */
			case I2C_MR_SLA_ACK:
                i2c.xfer_idx   = 0;
                /* ACK to keep receiving; switch to NACK on last byte */
                if (i2c.xfer_len == 1) TWCR = I2C_BuildTWCR(0);
                else                   TWCR = I2C_BuildTWCR(1 << TWCR_TWEA);
                return;

			/* Address read, slave NACKed */
			case I2C_MR_SLA_NACK:
                I2C_CompleteTransfer(I2C_RESULT_NACK);
                return;
				
            /* Data byte received, we ACKed it (more bytes coming) */
            case I2C_MR_DATA_ACK:
                i2c.xfer_buf[i2c.xfer_idx++] = TWDR;
                if ((i2c.xfer_idx + 1) >= i2c.xfer_len) {
                    /* Next byte is the last: NACK it */
                    TWCR = I2C_BuildTWCR(0);
                } else {
                    TWCR = I2C_BuildTWCR(1 << TWCR_TWEA);
                }
                return;

            /* Data byte received, we NACKed it (last byte) */
            case I2C_MR_DATA_NACK:
                i2c.xfer_buf[i2c.xfer_idx] = TWDR;
                I2C_CompleteTransfer(I2C_RESULT_OK);
                return;

            /* Arbitration lost — another master won the bus */
            case I2C_MT_ARB_LOST:
                I2C_CompleteTransfer(I2C_RESULT_ARB_LOST);
                return;
            
            default:
                /* Unknown master status — abort */
                I2C_CompleteTransfer(I2C_RESULT_BUS_ERROR);
                return;
        }
    }

    /* --------------------------------------------------------
     * SLAVE-ROLE events (or we were idle and got addressed)
     * -------------------------------------------------------- */
    switch (status) {

        /* Addressed for write */
        case I2C_SR_SLA_ACK:
        case I2C_SR_ARB_LOST_SLA_ACK:
		case I2C_SR_GEN_ACK:
		case I2C_SR_ARB_LOST_GEN_ACK:
            I2C_Ack();
            return;

		/* Data byte received from master */
        case I2C_SR_DATA_ACK:
        case I2C_SR_GEN_DATA_ACK:
        /* Data byte received, NACK required (we told master to stop) */
        case I2C_SR_DATA_NACK:
        case I2C_SR_GEN_DATA_NACK:
			if (i2c.xfer_srcb) i2c.xfer_srcb(I2C_SlaveReadByte());
			I2C_Ack();
			return;
		
        /* Addressed for read */
        case I2C_SR_SLA_R_ACK:
        case I2C_SR_ARB_LOST_SLA_R:
            if (i2c.xfer_stcb) I2C_SlaveWriteByte(i2c.xfer_stcb());
            I2C_Ack();
            return;

        /* Byte transmitted, master ACKed — load next byte */
        case I2C_SR_DATA_TX_ACK:
            if (i2c.xfer_stcb) I2C_SlaveWriteByte(i2c.xfer_stcb());
            I2C_Ack();
            return;

        /* Byte transmitted, master NACKed — transfer done */
        case I2C_SR_DATA_TX_NACK:
        case I2C_SR_LAST_DATA_TX:
            I2C_Ack();
            return;

        /* STOP or repeated START received */
        case I2C_SR_STOP_REP_START:
            i2c.xfer_role = I2C_ROLE_IDLE;
            if (i2c.xfer_sscb) i2c.xfer_sscb();
            I2C_Ack();
            return;

        /* Bus error */
        case I2C_BUS_ERROR:
			i2c.xfer_role = I2C_ROLE_IDLE;
            TWCR = I2C_BuildTWCR(1 << TWCR_TWSTO);
            return;

        default:
            /* Unknown — just ACK to unstick the peripheral */
            I2C_Ack();
            return;
    }
}