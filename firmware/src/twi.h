#ifndef TWI_H
#define TWI_H

#include <stdint.h>

void twi_init(void);

/* Returns 1 on ACK, 0 on NACK/failure for each step, so callers can
 * bail out instead of hanging forever if the RTC is unplugged. */
uint8_t twi_start(void);
void    twi_stop(void);
uint8_t twi_write(uint8_t data);
uint8_t twi_read_ack(void);   /* read a byte, then send ACK  (more bytes follow) */
uint8_t twi_read_nack(void);  /* read a byte, then send NACK (last byte)         */

/* Convenience helpers built on top of the above, used by ds3231.c */
uint8_t twi_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t value);
uint8_t twi_read_regs(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len);

#endif 
