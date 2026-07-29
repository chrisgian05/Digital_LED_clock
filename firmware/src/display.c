#include "config.h"
#include "display.h"
#include <avr/io.h>
#include <avr/interrupt.h>

const uint8_t FONT_DIGIT[10] = {
    0x3F, /* 0 */
    0x06, /* 1 */
    0x5B, /* 2 */
    0x4F, /* 3 */
    0x66, /* 4 */
    0x6D, /* 5 */
    0x7D, /* 6 */
    0x07, /* 7 */
    0x7F, /* 8 */
    0x6F, /* 9 */
};

static volatile uint8_t seg_buffer[NUM_DIGITS];
static volatile uint32_t tick_ms = 0;

/* low level bit-bang helpers */

static inline void pulse_high(volatile uint8_t *port, uint8_t bit)
{
    *port |= (1 << bit); /*high*/
    *port &= ~(1 << bit); /*low*/
}

/* Shift one byte out MSB-first on the 74HC595 bus and latch it.
 * Sending MSB (bit7) first means the LAST bit shifted in (bit0) ends
 * up sitting at QA once RCLK latches - so bit0 of 'digit_mask' below
 * always maps to QA / DIG1, bit5 to QF / DIG6. */
static void shift_out_595(uint8_t data)
{
    for (int8_t i = 7; i >= 0; i--) {
        if (data & (1 << i))
            DIG595_PORT |= (1 << DIG595_SER);
        else
            DIG595_PORT &= ~(1 << DIG595_SER);

        pulse_high(&DIG595_PORT, DIG595_CLK);
    }
    pulse_high(&DIG595_PORT, DIG595_LATCH);
}

/* Same idea for the TLC5916. Bit0 = SEGA ... bit7 = SEGDP. */
static void shift_out_tlc5916(uint8_t data)
{
    for (int8_t i = 7; i >= 0; i--) {
        if (data & (1 << i))
            SEG_PORT |= (1 << SEG_SDI);
        else
            SEG_PORT &= ~(1 << SEG_SDI);

        pulse_high(&SEG_PORT, SEG_CLK);
    }
    pulse_high(&SEG_PORT, SEG_LE);
}

/* public API */

void display_set_digit(uint8_t pos, uint8_t segments)
{
    if (pos < NUM_DIGITS) {
        seg_buffer[pos] = segments;
    }
}

void display_show_time(uint8_t hour, uint8_t minute, uint8_t second,
                        uint8_t colon_on)
{
    uint8_t dp = colon_on ? SEG_DP : 0;
    display_set_digit(0, FONT_DIGIT[hour / 10]);
    display_set_digit(1, FONT_DIGIT[hour % 10] | dp);
    display_set_digit(2, FONT_DIGIT[minute / 10]);
    display_set_digit(3, FONT_DIGIT[minute % 10] | dp);
    display_set_digit(4, FONT_DIGIT[second / 10]);
    display_set_digit(5, FONT_DIGIT[second % 10]);
}

void display_show_temp(int16_t tenths_c)
{
    uint8_t negative = 0;
    if (tenths_c < 0) {
        negative = 1;
        tenths_c = (int16_t)(-tenths_c);
    }
    uint8_t whole = (uint8_t)(tenths_c / 10);
    uint8_t tenths = (uint8_t)(tenths_c % 10);

    /* Layout across the 6 digits, e.g. " -23.5" or "  8.0 " :
     *   pos 0 : minus sign (SEG_DASH) if negative, else blank
     *   pos 1 : tens digit of the whole degrees, blank if not needed
     *   pos 2 : ones digit of the whole degrees, with the decimal point
     *   pos 3 : tenths digit
     *   pos 4,5 : blank (spare - could show a 'C' pattern later) */
    display_set_digit(0, negative ? SEG_DASH : SEG_BLANK);
    display_set_digit(1, (whole >= 10) ? FONT_DIGIT[whole / 10] : SEG_BLANK);
    display_set_digit(2, FONT_DIGIT[whole % 10] | SEG_DP);
    display_set_digit(3, FONT_DIGIT[tenths]);
    display_set_digit(4, SEG_BLANK);
    display_set_digit(5, SEG_BLANK);
}

uint32_t millis(void)
{
    uint32_t val;
    /* tick_ms is written a byte at a time by the ISR under interrupts;
     * reading a 32-bit value non-atomically on an 8-bit MCU can tear,
     * so we briefly disable interrupts around the read. */
    cli();
    val = tick_ms;
    sei();
    return val;
}

void display_init(void)
{
    /* Set all the 595 + TLC5916 control lines as outputs via DDR */
    DIG595_DDR |= (1 << DIG595_SER) | (1 << DIG595_CLK) | (1 << DIG595_LATCH);
    SEG_DDR    |= (1 << SEG_SDI) | (1 << SEG_CLK) | (1 << SEG_LE);

    for (uint8_t i = 0; i < NUM_DIGITS; i++) seg_buffer[i] = SEG_BLANK;

    /* Start blanked so we never flash garbage on power-up */
    shift_out_tlc5916(SEG_BLANK);
    shift_out_595(0x00);

    /* Timer1, CTC mode, prescaler 64, fires every 1 ms (REFRESH_TICK_HZ).
     * Timer 1 orders AVR to stop and refresh the next digit. */
    TCCR1A = 0x00;
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
    OCR1A  = (uint16_t)((F_CPU / 64UL / REFRESH_TICK_HZ) - 1);
    TIMSK1 = (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect)
{
    static uint8_t cur = 0;

    tick_ms++;

    /* 1) Blank the currently-active digit BEFORE changing the segment
     *    data, otherwise you'd briefly show the new segments on the
     *    old digit (ghosting/smearing between digits). */
    shift_out_595(0x00);

    /* 2) Load the segment pattern for the digit we're about to show */
    uint8_t segs = seg_buffer[cur];
#if !SEGMENT_ACTIVE_HIGH
    segs = (uint8_t)~segs;
#endif
    shift_out_tlc5916(segs);

    /* 3) Enable that one digit */
#if DIGIT_ACTIVE_HIGH
    uint8_t mask = (uint8_t)(1 << cur);
#else
    uint8_t mask = (uint8_t)~(1 << cur);
#endif
    shift_out_595(mask);

    cur++;
    if (cur >= NUM_DIGITS) cur = 0;
}