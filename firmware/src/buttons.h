#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

/* One bit per button in the value returned by buttons_poll() */
#define BTN_EVT_HOUR     (1 << 0)
#define BTN_EVT_MIN      (1 << 1)
#define BTN_EVT_SEC      (1 << 2)
#define BTN_EVT_TEMP_TOG (1 << 3)

void buttons_init(void);

/* Call this once per main loop iteration (it's cheap). Returns a
 * bitmask of buttons that "fired" this call: that's either a fresh
 * debounced press, or an auto-repeat pulse if the button has been
 * held down (handy for HOUR/MIN so you don't have to mash the button
 * to set the time). TEMP_TOG deliberately does NOT auto-repeat since
 * it's a toggle, not a counter. */
uint8_t buttons_poll(void);

#endif
