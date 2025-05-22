MCU = atmega328p
F_CPU = 16000000UL
BAUD = 57600
PROGRAMMER = arduino
PORT1 = /dev/ttyUSB0
PORT2 = /dev/ttyUSB1

AVRDUDE = avrdude
CC = avr-g++
OBJCOPY = avr-objcopy

CFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2 -DF_CPU=$(F_CPU) -mmcu=$(MCU)

all: transmitter.hex receiver.hex

transmitter.elf: transmitter.cpp
	$(CC) $(CFLAGS) -o $@ $<

transmitter.hex: transmitter.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flasht: transmitter.hex
	$(AVRDUDE) -p $(MCU) -c $(PROGRAMMER) -P $(PORT1) -b $(BAUD) -D -U flash:w:$<

receiver.elf: receiver.cpp
	$(CC) $(CFLAGS) -o $@ $<

receiver.hex: receiver.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

flashr: receiver.hex
	$(AVRDUDE) -p $(MCU) -c $(PROGRAMMER) -P $(PORT2) -b $(BAUD) -D -U flash:w:$<

clean:
	rm -f *.elf *.hex
