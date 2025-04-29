#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define F_CPU 16000000UL

void timer1_init_10ms_interrupt() {
    TCCR1A = 0;                    // Нормальный режим
    TCCR1B = (1 << WGM12);         // CTC режим (TOP = OCR1A)
    OCR1A = 199;                   // 199 - 100 мкс при делителе 8
    TIMSK1 = (1 << OCIE1A);        // Разрешить прерывание по совпадению
    TCCR1B |= (1 << CS11);         // Делитель 8
}

volatile char message[] = " Hello world\n";
volatile int bit_index=7;
volatile int byte_index=0;
volatile bool stop_flag=false;
ISR(TIMER1_COMPA_vect) {
    // PORTD ^= (1 << PD2);
    if (!stop_flag) {
        if (!*(message+byte_index)) {
            stop_flag=true;
            PORTD &= ~(1 << PD2); // сбросить пин в 0
        }
        else {
            PORTD = ((*(message+byte_index) & (1 << bit_index)) << PD2);
            if (bit_index == 7) {
                bit_index=0;
                byte_index++;
            } else bit_index++;
        }
    }
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