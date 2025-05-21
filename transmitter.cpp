#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>

#define F_CPU 16000000UL

void timer1_init_100us_interrupt() {
    TCCR1A = 0;                    // Нормальный режим
    TCCR1B = (1 << WGM12);         // CTC режим (TOP = OCR1A)
    OCR1A = 199;                   // 199 - 100 мкс при делителе 8
    TIMSK1 = (1 << OCIE1A);        // Разрешить прерывание по совпадению
    TCCR1B |= (1 << CS11);         // Делитель 8
}

void timer1_init_10ms_interrupt() {
    TCCR1A = 0;              // Нормальный режим
    TCCR1B = (1 << WGM12);   // CTC режим
    OCR1A = 2500 - 1;        // 10 мс при 64 делителе
    TIMSK1 = (1 << OCIE1A);  // Включить прерывание по совпадению
    TCCR1B |= (1 << CS11) | (1 << CS10); // делитель 64
}

void timer1_init_100ms_interrupt() {
    TCCR1A = 0;              // Нормальный режим
    TCCR1B = (1 << WGM12);   // CTC режим
    OCR1A = 25000 - 1;        // 10 мс при 64 делителе
    TIMSK1 = (1 << OCIE1A);  // Включить прерывание по совпадению
    TCCR1B |= (1 << CS11) | (1 << CS10); // делитель 64
}

void uart_init(uint16_t ubrr) {
    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)ubrr;
    UCSR0B = (1 << TXEN0);                    // Включить передатчик
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);   // 8 бит данных, 1 стоп-бит
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

volatile char message[] = " Hello world\n\0";
volatile int bit_index=0; // с начала, чтобы вывести первую незначащую 1
volatile int byte_index=0;
volatile bool stop_flag=false;
ISR(TIMER1_COMPA_vect) {
    // PORTD ^= (1 << PD2);
    if (!stop_flag) {
        if (!message[byte_index]) { // если \0
            stop_flag=true;
            PORTD &= ~(1 << PD2); // сбросить пин в 0
        }
        else {
            int one_or_zero = (message[byte_index] & (1 << bit_index)) >> bit_index ; // DEBUG
            uart_send_uint16(one_or_zero); // DEBUG
            PORTD = (one_or_zero << PD2);
            bit_index--;
            if (bit_index == -1) {
                bit_index=7;
                byte_index++;
                uart_send_string("\r\n"); // DEBUG
            }
        }
    }
}

int main(void) {
    // Настроить D2 (PD2) как выход
    DDRD |= (1 << PD2);

    uart_init(8); // DEBUG
    timer1_init_100ms_interrupt();
    // timer1_init_100us_interrupt();
    message[0] = (char)1;
    sei(); // Включить глобальные прерывания

    volatile unsigned char side_effect = 0;
    while (1) {
        ++side_effect; // Побочный эффект чтобы цикл не был UB
        // Основной цикл, можно делать что угодно
    }

    return 0;
}