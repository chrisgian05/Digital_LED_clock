#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

/* Segment bit layout, matching TLC5916 OUT0..OUT7 = SEGA..SEGG,SEGDP:
 *   bit0=a bit1=b bit2=c bit3=d bit4=e bit5=f bit6=g bit7=dp
 */
#define SEG_DP 0x80

extern const uint8_t FONT_DIGIT[10]; /* '0'..'9' -> segment pattern */
#define SEG_BLANK 0x00
#define SEG_DASH  0x40 /* just the 'g' segment, a minus sign */

void display_init(void);
void display_set_digit(uint8_t pos, uint8_t segments);

/* High level helpers used by main.c */
void display_show_time(uint8_t hour, uint8_t minute, uint8_t second,
                        uint8_t colon_on);
void display_show_temp(int16_t tenths_c);

/* 1 kHz system tick, incremented inside the refresh ISR. Used for
 * button debouncing and scheduling RTC polls without a second timer. */
uint32_t millis(void);

#endif 
