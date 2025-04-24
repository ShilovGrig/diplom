#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL

void timer1_init_10ms_interrupt() {
    TCCR1A = 0;              // Нормальный режим
    TCCR1B = (1 << WGM12);   // CTC режим
    OCR1A = 25000 - 1;        // 10 мс при 64 делителе
    TIMSK1 = (1 << OCIE1A);  // Включить прерывание по совпадению
    TCCR1B |= (1 << CS11) | (1 << CS10); // делитель 64
}

void switch_d2() {
    PORTD ^= (1 << PD2);
}

ISR(TIMER1_COMPA_vect) {
    switch_d2();
}

int main(void) {
    // Настроить D2 (PD2) как выход
    DDRD |= (1 << PD2);

    timer1_init_10ms_interrupt();

    sei(); // Включить глобальные прерывания

    while (1) {
        // Основной цикл, можно делать что угодно
    }

    return 0;
}