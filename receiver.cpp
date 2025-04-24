#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#define F_CPU 16000000UL

void timer1_init_10ms_interrupt() {
    TCCR1A = 0;              // Нормальный режим
    TCCR1B = (1 << WGM12);   // CTC режим
    OCR1A = 12500 - 1;        // 5 мс при 64 делителе
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
void uart_send_byte(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0))); // Ждём, пока буфер освободится
    UDR0 = data;
}

// --- Отправка строки ---
void uart_send_string(const char *str) {
    while (*str) {
        uart_send_byte(*str++);
    }
}

// --- Отправка числа (uint16_t) ---
void uart_send_uint16(uint16_t value) {
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

ISR(TIMER1_COMPA_vect) {
    // Этот код вызывается каждые 5 мс
    // PORTB ^= (1 << PB5); // Пример: переключить встроенный светодиод (D13)
    uint16_t value = adc_read();
    uart_send_uint16(value);
    uart_send_string("\r\n"); // переход на новую строку
}

int main(void) {
    // Настроить A2 (PC2) как вход
    DDRC &= ~(1 << PC0);

    timer1_init_10ms_interrupt();
    uart_init(8);
    adc_init();

    sei(); // Включить глобальные прерывания

    while (1) {
        // Основной цикл, можно делать что угодно
    }

    return 0;
}