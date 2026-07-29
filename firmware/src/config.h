#ifndef CONFIG_H
#define CONFIG_H

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

/*74HC595 : digit select (clock board, local)*/
#define DIG595_PORT   PORTD
#define DIG595_DDR    DDRD
#define DIG595_SER    PD0   
#define DIG595_CLK    PD1   
#define DIG595_LATCH  PD2   

#define DIGIT_ACTIVE_HIGH 1

/*TLC5916 : segment data (over J1 PERIPH to the LED board)*/
#define SEG_PORT      PORTC
#define SEG_DDR       DDRC
#define SEG_SDI       PC0  
#define SEG_CLK       PC1   
#define SEG_LE        PC2  

#define SEGMENT_ACTIVE_HIGH 1 

/*Buttons (active LOW, using internal pull-ups)*/
#define BTN_PORT      PORTD
#define BTN_DDR       DDRD
#define BTN_PIN_REG   PIND
#define BTN_HOUR      PD4   
#define BTN_MIN       PD5   
#define BTN_SEC       PD6   
#define BTN_TEMP_TOG  PD7   

#define NUM_DIGITS 6

/* Refresh: each digit gets a 1 ms on-time slice -> ~166 Hz full-frame
 * refresh, well above the flicker threshold. Also used as a 1 kHz
 * system tick for button debouncing. */
#define REFRESH_TICK_HZ 1000UL

/* I2C bus speed to the DS3231 */
#define TWI_SCL_FREQ 100000UL /*100kHz*/

#endif 
