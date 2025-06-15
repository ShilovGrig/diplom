#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h> // NOLINT
#include <stdbool.h>

#define F_CPU 16000000UL

void timer1_init_100us_interrupt() {
    TCCR1A = 0;                    // Нормальный режим
    TCCR1B = (1 << WGM12);         // CTC режим (TOP = OCR1A)
    // OCR1A = 199;                   // 100 мкс при делителе 8
    OCR1A = 2006 - 1;                   // 100 мкс при делителе 8
    TIMSK1 = (1 << OCIE1A);        // Разрешить прерывание по совпадению
    TCCR1B |= (1 << CS11);         // Делитель 8
}

void timer1_init_10ms_interrupt() {
    TCCR1A = 0;              // Нормальный режим
    TCCR1B = (1 << WGM12);   // CTC режим
    OCR1A = 2506 - 1;        // 10 мс при 64 делителе
    // OCR1A = 2500 - 1;        // 10 мс при 64 делителе
    TIMSK1 = (1 << OCIE1A);  // Включить прерывание по совпадению
    TCCR1B |= (1 << CS11) | (1 << CS10); // делитель 64
}

void timer1_init_100ms_interrupt() {
    TCCR1A = 0;              // Нормальный режим
    TCCR1B = (1 << WGM12);   // CTC режим
    OCR1A = 25000 - 1;        // 100 мс при 64 делителе
    TIMSK1 = (1 << OCIE1A);  // Включить прерывание по совпадению
    TCCR1B |= (1 << CS11) | (1 << CS10); // делитель 64
}

void uart_init(uint16_t ubrr) {
    UBRR0H = static_cast<uint8_t>(ubrr >> 8);
    UBRR0L = static_cast<uint8_t>(ubrr);
    UCSR0B = (1 << TXEN0);                    // Включить передатчик
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);   // 8 бит данных, 1 стоп-бит
}

void adc_init() {
    ADMUX = (1 << REFS0);                      // AVcc как опорное
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // делитель 64
}

// --- Отправка байта ---
void uart_send_byte(volatile uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0))); // Ждём, пока буфер освободится
    UDR0 = data;
}

// --- Отправка строки ---
void uart_send_string(volatile const char *str) {
    while (*str) {
        uart_send_byte(*str++);
    }
}

// --- Отправка числа (uint16_t) ---
void uart_send_uint16(volatile uint16_t value) {
    char buffer[6];
    itoa(value, buffer, 10);
    uart_send_string(buffer);
}

void uart_send_string_enh(volatile const char *str) {
    while (*str) {
        for (int bit_index=7; bit_index >= 0 ; --bit_index) {
            uart_send_uint16((*str & (1 << bit_index)) >> bit_index);
        }
        uart_send_byte(' ');
        uart_send_byte(*str++);
        uart_send_string("\r\n");
    }
}

uint16_t adc_read() {
    ADMUX = (ADMUX & 0xF0) | 0x00;                    // выбрать канал 0 (A0)
    ADCSRA |= (1 << ADSC);                     // старт
    while (ADCSRA & (1 << ADSC));              // ожидание завершения
    return ADC;
}

char is_one(const uint16_t value) {
    static const uint16_t MAX_ZERO = 400;
    if (value < MAX_ZERO) return 1;
    return 0;
}

#define BUFFER_SIZE 256
volatile char buffer[BUFFER_SIZE];
volatile int bit_index=0;
volatile int byte_index=0;
volatile bool read_started=false;
volatile bool ready_to_send=false;
ISR(TIMER1_COMPA_vect) {
    uint16_t value = adc_read();
    // uint16_t value = (adc_read() + adc_read() + adc_read()) / 3;
    // uart_send_uint16(value); // DEBUG
    // uart_send_string("\r\n"); // DEBUG
    if (read_started) {
        // uart_send_string("  inside  1\r\n"); // DEBUG
        buffer[byte_index] = (buffer[byte_index] << 1) | is_one(value);
        bit_index++;
        // uart_send_uint16(value); // DEBUG
        // uart_send_string("\r\n"); // DEBUG
        if (bit_index == 8) {
            if (buffer[byte_index]=='\0' || byte_index == BUFFER_SIZE-2) {
                buffer[byte_index+1] = 0;
                read_started=false;
                ready_to_send=true;
                bit_index = 0;
                byte_index = 0;
            } else {
                bit_index = 0;
                byte_index++;
            }
            // uart_send_string("  inside  2\r\n"); // DEBUG
        }
    } else if (is_one(value)) {
        read_started=true;
    }
}

int main(void) {
    DDRC &= ~(1 << PC0);

    timer1_init_10ms_interrupt();
    uart_init(8);
    adc_init();

    sei(); // Включить глобальные прерывания

    uart_send_string("start\r\n"); // DEBUG
    while (1) {
        if (ready_to_send) {
            // uart_send_string_enh(buffer);
            uart_send_string(buffer);
            uart_send_string("\r\n");
            ready_to_send=false;
        }
    }
    return 0;
}