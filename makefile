MCU = atmega328p
F_CPU = 16000000UL
BAUD = 57600
PROGRAMMER = arduino
PORT = /dev/ttyUSB0         # Поменяй на свой порт, если другой

AVRDUDE = avrdude
CC = avr-gcc
OBJCOPY = avr-objcopy

CFLAGS = -Wall -Wextra -Wpedantic -Os -DF_CPU=$(F_CPU) -mmcu=$(MCU)

all: blink.hex

blink.elf: blink.c
	$(CC) $(CFLAGS) -o $@ $<

blink.hex: blink.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flash: blink.hex
	$(AVRDUDE) -p $(MCU) -c $(PROGRAMMER) -P $(PORT) -b $(BAUD) -D -U flash:w:$<

clean:
	rm -f *.elf *.hex
