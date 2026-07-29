#include "ds3231.h"
#include "twi.h"

/* DS3231 register map :
 *   0x00  seconds  (BCD, bits 6:4 = tens, bits 3:0 = ones)
 *   0x01  minutes  (BCD)
 *   0x02  hours    (BCD, bit 6 = 12/24h select; we always write 0 = 24h)
 *   0x0F  status   (bit 7 = OSF, oscillator-stop flag)
 *   0x11  temp MSB (signed integer part, degrees C)
 *   0x12  temp LSB (bits 7:6 = fractional quarters of a degree)
 */

static inline uint8_t bcd2dec(uint8_t bcd)
{
    return (uint8_t)((bcd >> 4) * 10 + (bcd & 0x0F));
}

static inline uint8_t dec2bcd(uint8_t dec)
{
    return (uint8_t)(((dec / 10) << 4) | (dec % 10));
}

uint8_t ds3231_init(void)
{
    uint8_t status;
    if (!twi_read_regs(DS3231_I2C_ADDR, 0x0F, &status, 1)) {
        return 0; /* RTC not responding on the bus */
    }
    /* We don't force-clear the oscillator-stop flag automatically */
    return 1;
}

uint8_t ds3231_get_time(rtc_time_t *t)
{
    uint8_t raw[3];
    if (!twi_read_regs(DS3231_I2C_ADDR, 0x00, raw, 3)) {
        return 0;
    }
    t->second = bcd2dec(raw[0] & 0x7F);
    t->minute = bcd2dec(raw[1] & 0x7F);
    t->hour   = bcd2dec(raw[2] & 0x3F); /* mask off 12/24h + AM/PM bits */
    return 1;
}

uint8_t ds3231_set_time(const rtc_time_t *t)
{
    uint8_t ok = 1;
    ok &= twi_write_reg(DS3231_I2C_ADDR, 0x00, dec2bcd(t->second));
    ok &= twi_write_reg(DS3231_I2C_ADDR, 0x01, dec2bcd(t->minute));
    /* bit6=0 selects 24-hour mode */
    ok &= twi_write_reg(DS3231_I2C_ADDR, 0x02, dec2bcd(t->hour) & 0x3F);
    return ok;
}

uint8_t ds3231_get_temp_tenths(int16_t *tenths_c)
{
    uint8_t raw[2];
    if (!twi_read_regs(DS3231_I2C_ADDR, 0x11, raw, 2)) {
        return 0;
    }
    int8_t whole = (int8_t)raw[0];       /* signed integer part (two's complement)*/
    uint8_t quarters = (raw[1] >> 6) & 0x03; /* 0..3 -> 0.00/.25/.50/.75 */
    int16_t frac_tenths = (int16_t)((quarters * 25) / 10); /* 0,2,5,7 */

    if (whole >= 0) {
        *tenths_c = (int16_t)(whole * 10 + frac_tenths);
    } else {
        *tenths_c = (int16_t)(whole * 10 - frac_tenths);
    }
    return 1;
}
