#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    // Установим 13-й пин (PB5) как выход
    DDRB |= (1 << PB5);

    while (1) {
        // Вкл светодиод
        PORTB |= (1 << PB5);
        _delay_ms(500);

        // Выкл светодиод
        PORTB &= ~(1 << PB5);
        _delay_ms(500);
    }
}
