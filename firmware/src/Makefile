MCU        = atmega328p
F_CPU      = 8000000UL
PROGRAMMER = usbasp
PORT       =

CC      = avr-gcc
OBJCOPY = avr-objcopy
SIZE    = avr-size

SRC_DIR = src
BUILD_DIR = build
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

CFLAGS  = -Wall -Wextra -Os -std=gnu11 -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS = -mmcu=$(MCU) -Wl,--gc-sections

TARGET = led_clock

all: $(BUILD_DIR)/$(TARGET).hex size

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

size: $(BUILD_DIR)/$(TARGET).elf
	$(SIZE) --format=avr --mcu=$(MCU) $<

# Flash over the ISP header (J5: PROG_MISO/PROG_SCK/PROG_MOSI/PROG_RESET).
# Adjust PROGRAMMER (e.g. usbasp, avrisp2, arduino) and PORT as needed.
flash: $(BUILD_DIR)/$(TARGET).hex
	avrdude -c $(PROGRAMMER) -p $(MCU) $(if $(PORT),-P $(PORT),) \
	        -U flash:w:$(BUILD_DIR)/$(TARGET).hex:i

# One-time fuse set for the internal 8 MHz RC oscillator, no CKDIV8
# (i.e. assumes NO crystal populated on the clock board). Only run
# this once per chip - double-check the fuse bytes for your exact
# part/programmer before using it blindly.
fuses:
	avrdude -c $(PROGRAMMER) -p $(MCU) $(if $(PORT),-P $(PORT),) \
	        -U lfuse:w:0xE2:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all size flash fuses clean
