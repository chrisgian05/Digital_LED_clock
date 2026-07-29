#include "config.h"
#include "buttons.h"
#include "display.h" /* for millis() */
#include <avr/io.h>

#define DEBOUNCE_MS         25
#define REPEAT_START_MS     500
#define REPEAT_INTERVAL_MS  150
#define NUM_BUTTONS 4

typedef struct {
    uint8_t  pin;
    uint8_t  evt_bit;
    uint8_t  can_repeat;
    uint8_t  stable_pressed; /* debounced state: 1 = currently pressed */
    uint8_t  raw_last;       /* last raw sample, to spot bounce/changes */
    uint32_t last_change_ms; /* when raw_last last changed */
    uint32_t next_repeat_ms; /* when to fire the next auto-repeat event */
} button_t;

static button_t buttons[NUM_BUTTONS] = {
    { BTN_HOUR,     BTN_EVT_HOUR,     1, 0, 0, 0, 0 },
    { BTN_MIN,      BTN_EVT_MIN,      1, 0, 0, 0, 0 },
    { BTN_SEC,      BTN_EVT_SEC,      1, 0, 0, 0, 0 },
    { BTN_TEMP_TOG, BTN_EVT_TEMP_TOG, 0, 0, 0, 0, 0 },
};

void buttons_init(void)
{
    /* Inputs with internal pull-ups; buttons pull the pin to GND */
    BTN_DDR  &= (uint8_t)~((1 << BTN_HOUR) | (1 << BTN_MIN) |
                           (1 << BTN_SEC) | (1 << BTN_TEMP_TOG));
    BTN_PORT |=  (1 << BTN_HOUR) | (1 << BTN_MIN) |
                 (1 << BTN_SEC) | (1 << BTN_TEMP_TOG);
}

uint8_t buttons_poll(void)
{
    uint8_t events = 0;
    uint32_t now = millis();

    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        button_t *b = &buttons[i];
        uint8_t raw_pressed = (BTN_PIN_REG & (1 << b->pin)) ? 0 : 1;

        if (raw_pressed != b->raw_last) {
            /* Level changed (could be a real transition or contact
             * bounce) - restart the debounce timer. */
            b->raw_last = raw_pressed;
            b->last_change_ms = now;
        } else if ((now - b->last_change_ms) >= DEBOUNCE_MS &&
                   raw_pressed != b->stable_pressed) {
            /* Level has been stable long enough: accept it. */
            b->stable_pressed = raw_pressed;

            if (b->stable_pressed) {
                events |= b->evt_bit;              /* fresh press event */
                b->next_repeat_ms = now + REPEAT_START_MS;
            }
        }

        if (b->stable_pressed && b->can_repeat && now >= b->next_repeat_ms) {
            events |= b->evt_bit;
            b->next_repeat_ms = now + REPEAT_INTERVAL_MS;
        }
    }

    return events;
}