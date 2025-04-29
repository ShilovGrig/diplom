#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>

#define F_CPU 16000000UL

void timer1_init_10ms_interrupt() {
    TCCR1A = 0;              // Нормальный режим
    TCCR1B = (1 << WGM12);   // CTC режим
    OCR1A = 2500 - 1;        // 10 мс при 64 делителе
    TIMSK1 = (1 << OCIE1A);  // Включить прерывание по совпадению
    TCCR1B |= (1 << CS11) | (1 << CS10); // делитель 64
}

void uart_init(uint16_t ubrr) {
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
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

uint16_t adc_read() {
    ADMUX = (ADMUX & 0xF0);                    // выбрать канал 0 (A0)
    ADCSRA |= (1 << ADSC);                     // старт
    while (ADCSRA & (1 << ADSC));              // ожидание завершения
    return ADC;
}


char is_one(uint16_t value) {
    static const uint16_t MIN_ONE = 90;
    if (value > MIN_ONE) return 1;
    return 0;
}

volatile bool read_started=false;
volatile bool ready_to_send=false;
#define BUFFER_SIZE 128
volatile char buffer[BUFFER_SIZE];
volatile int bit_index=0;
volatile int byte_index=0;
ISR(TIMER1_COMPA_vect) {
    uint16_t value = (adc_read() + adc_read() + adc_read()) / 3;
    // uart_send_uint16(value);
    if (read_started) {
        buffer[byte_index] = (buffer[byte_index] << 1) | is_one(value);
        bit_index++;
        if (bit_index == 8) {

            if (buffer[byte_index]=='\n' || byte_index == BUFFER_SIZE-2) {
                buffer[byte_index+1] = 0;
                read_started=false;
                ready_to_send=true;
                bit_index = 0;
                byte_index = 0;
            } else {
                bit_index = 0;
                byte_index++;
            }
        }
    } else if (is_one(value)) {
        read_started=true;
    }
}

int main(void) {
    // Настроить A2 (PC2) как вход
    DDRC &= ~(1 << PC0);

    timer1_init_10ms_interrupt();
    uart_init(8);
    adc_init();

    sei(); // Включить глобальные прерывания

    while (1) {
        if (ready_to_send) {
            uart_send_string(buffer);
            uart_send_string("\r\n");
            ready_to_send=false;
        }
    }

    return 0;
}