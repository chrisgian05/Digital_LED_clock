#include "config.h"
#include "twi.h"
#include "ds3231.h"
#include "display.h"
#include "buttons.h"
#include <avr/interrupt.h>

#define RTC_POLL_INTERVAL_MS   200  /* how often we re-read HH:MM:SS   */
#define TEMP_POLL_INTERVAL_MS  1000 /* DS3231 only updates temp ~64s   */

int main(void)
{
    twi_init();
    ds3231_init();      /* if this returns 0, the RTC isn't answering -
                          * time will just read back as whatever junk
                          * twi_read_regs left in the struct (likely all
                          * zero), which is harmless, it'll just show
                          * 00:00:00 until you fix the wiring. */
    display_init();
    buttons_init();
    sei();               /* enable interrupts -> Timer1 multiplex ISR starts */

    rtc_time_t cur_time = {0, 0, 0};
    int16_t    cur_tenths_c = 0;
    uint8_t    show_temp = 0;

    uint32_t last_rtc_poll  = 0;
    uint32_t last_temp_poll = 0;

    for (;;) {
        uint8_t events = buttons_poll();
        uint8_t time_changed = 0;

        if (events & BTN_EVT_HOUR) {
            cur_time.hour = (uint8_t)((cur_time.hour + 1) % 24);
            time_changed = 1;
        }
        if (events & BTN_EVT_MIN) {
            cur_time.minute = (uint8_t)((cur_time.minute + 1) % 60);
            time_changed = 1;
        }
        if (events & BTN_EVT_SEC) {
            /* Classic "seconds reset" behaviour: snaps the RTC to the
             * top of the current minute. Change this to an increment
             * like the others if you'd rather it count seconds up. */
            cur_time.second = 0;
            time_changed = 1;
        }
        if (events & BTN_EVT_TEMP_TOG) {
            show_temp = (uint8_t)!show_temp;
        }

        if (time_changed) {
            ds3231_set_time(&cur_time);
            last_rtc_poll = millis(); 
        } else if ((millis() - last_rtc_poll) >= RTC_POLL_INTERVAL_MS) {
            ds3231_get_time(&cur_time);
            last_rtc_poll = millis();
        }

        if ((millis() - last_temp_poll) >= TEMP_POLL_INTERVAL_MS) {
            ds3231_get_temp_tenths(&cur_tenths_c);
            last_temp_poll = millis();
        }

        if (show_temp) {
            display_show_temp(cur_tenths_c);
        } else {
            uint8_t colon_on = (uint8_t)((cur_time.second & 0x01) == 0);
            display_show_time(cur_time.hour, cur_time.minute,
                               cur_time.second, colon_on);
        }
    }
}
