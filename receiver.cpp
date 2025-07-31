#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define F_CPU 16000000UL

// Таймер прерываний
void timer1_init_10ms_interrupt() {
    TCCR1A = 0;
    TCCR1B = (1 << WGM12);            // CTC
    OCR1A = 2506 - 1;                 // 10 мс при делителе 64
    TIMSK1 = (1 << OCIE1A);           // Включить прерывание
    TCCR1B |= (1 << CS11) | (1 << CS10); // Делитель 64
}

// UART
void uart_init(uint16_t ubrr) {
    UBRR0H = static_cast<uint8_t>(ubrr >> 8);
    UBRR0L = static_cast<uint8_t>(ubrr);
    UCSR0B = (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_send_byte(volatile uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void uart_send_string(volatile const char *str) {
    while (*str) {
        uart_send_byte(*str++);
    }
}

// ADC
void adc_init() {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);
}

uint16_t adc_read() {
    ADMUX = (ADMUX & 0xF0) | 0x00; // канал A0
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}

char is_one(const uint16_t value) {
    static const uint16_t MAX_ZERO = 400;
    return value < MAX_ZERO ? 1 : 0;
}

// PWM для SG90
void servo_init() {
    DDRB |= (1 << PB1); // D9 как выход

    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // делитель 8

    ICR1 = 40000; // 20 мс (50 Гц)
}

void servo_set_angle(uint8_t angle) {
    OCR1A = 2000 + ((uint32_t)angle * 2000UL) / 180; // от 2000 до 4000
}

// Приём данных
#define BUFFER_SIZE 256
volatile char buffer[BUFFER_SIZE];
volatile int bit_index = 0;
volatile int byte_index = 0;
volatile bool read_started = false;
volatile bool ready_to_send = false;

ISR(TIMER1_COMPA_vect) {
    uint16_t value = adc_read();

    if (read_started) {
        buffer[byte_index] = (buffer[byte_index] << 1) | is_one(value);
        bit_index++;

        if (bit_index == 8) {
            if (buffer[byte_index] == '\0' || byte_index == BUFFER_SIZE - 2) {
                buffer[byte_index + 1] = 0;
                read_started = false;
                ready_to_send = true;
                bit_index = 0;
                byte_index = 0;
            } else {
                bit_index = 0;
                byte_index++;
            }
        }
    } else if (is_one(value)) {
        read_started = true;
    }
}

// MAIN
int main(void) {
    DDRC &= ~(1 << PC0); // A0 вход

    timer1_init_10ms_interrupt();
    uart_init(8);       // 115200 бод
    adc_init();
    servo_init();

    sei(); // глобальные прерывания

    uart_send_string("start\r\n");

    while (1) {
        if (ready_to_send) {
            uart_send_string(buffer);
            uart_send_string("\r\n");

            if (strcmp((char*)buffer, "Hello world") == 0) {
                servo_set_angle(90);   // открыть
                _delay_ms(5000);       // подержать
                servo_set_angle(0);    // закрыть
            }

            ready_to_send = false;
        }
    }
    return 0;
}

