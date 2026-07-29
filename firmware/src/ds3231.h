#ifndef DS3231_H
#define DS3231_H

#include <stdint.h>

#define DS3231_I2C_ADDR 0x68

typedef struct {
    uint8_t hour;   /* 0-23 */
    uint8_t minute; /* 0-59 */
    uint8_t second; /* 0-59 */
} rtc_time_t;

/* Returns 1 on success, 0 if the RTC didn't respond (unplugged, etc). */
uint8_t ds3231_init(void);
uint8_t ds3231_get_time(rtc_time_t *t);
uint8_t ds3231_set_time(const rtc_time_t *t);

/* Temperature in tenths of a degree C, e.g. 235 = 23.5C. */
uint8_t ds3231_get_temp_tenths(int16_t *tenths_c);

#endif 
