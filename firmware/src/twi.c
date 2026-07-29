#include "config.h"
#include "twi.h"
#include <avr/io.h>
#include <util/twi.h>

void twi_init(void)
{
    /* No prescaling (TWPS = 0), so TWBR sets the frequency directly:
     * SCL = F_CPU / (16 + 2*TWBR*4^TWPS) */
    TWSR = 0x00;
    TWBR = (uint8_t)(((F_CPU / TWI_SCL_FREQ) - 16) / 2);
    TWCR = (1 << TWEN); /* enable TWI, no interrupt */
}

static uint8_t twi_wait(void)
{
    /* Wait for the current TWI operation to finish. Give up after a
     * generous number of spins so a missing/broken RTC can't hang the
     * whole clock. */
    uint16_t timeout = 20000;
    while (!(TWCR & (1 << TWINT))) {
        if (--timeout == 0) return 0;
    }
    return 1;
}

uint8_t twi_start(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); 
    /* TWINT=1 doesn't mean start an interrupt. It's a flag that clears 
    the previous interrupt flag. */
    if (!twi_wait()) return 0;
    uint8_t status = TW_STATUS;
    return (status == TW_START || status == TW_REP_START);
}

void twi_stop(void)
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    /* TWSTO clears itself once the stop condition is sent; no TWINT
     * flag follows a stop, so we just give the bus a brief moment. */
    uint16_t timeout = 20000;
    while ((TWCR & (1 << TWSTO)) && --timeout) { }
}

uint8_t twi_write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    if (!twi_wait()) return 0;
    uint8_t status = TW_STATUS;
    return (status == TW_MT_SLA_ACK || status == TW_MT_DATA_ACK ||
            status == TW_MR_SLA_ACK);
}

uint8_t twi_read_ack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    twi_wait();
    return TWDR;
}

uint8_t twi_read_nack(void)
{
    TWCR = (1 << TWINT) | (1 << TWEN);
    twi_wait();
    return TWDR;
}

uint8_t twi_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t value)
{
    uint8_t ok = 1;
    ok &= twi_start();
    ok &= twi_write((dev_addr << 1) | TW_WRITE);
    ok &= twi_write(reg);
    ok &= twi_write(value);
    twi_stop();
    return ok;
}

uint8_t twi_read_regs(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t ok = 1;
    ok &= twi_start();
    ok &= twi_write((dev_addr << 1) | TW_WRITE);
    ok &= twi_write(reg);

    ok &= twi_start(); /* repeated start into read mode */
    ok &= twi_write((dev_addr << 1) | TW_READ);
    if (!ok) {
        twi_stop();
        return 0;
    }

    for (uint8_t i = 0; i < len; i++) {
        buf[i] = (i == len - 1) ? twi_read_nack() : twi_read_ack();
    }
    twi_stop();
    return 1;
}
