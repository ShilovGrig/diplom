#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#define F_CPU 16000000UL

// Timer inits variations
void timer1_init_100us_interrupt() {
    TCCR1A = 0;                    // Нормальный режим
    TCCR1B = (1 << WGM12);         // CTC режим (TOP = OCR1A)
    // OCR1A = 199;                   // 199 - 100 мкс при делителе 8
    OCR1A = 2000 - 1;                   // 100 мкс при делителе 8
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
    OCR1A = 25000 - 1;        // 100 мс при 64 делителе
    TIMSK1 = (1 << OCIE1A);  // Включить прерывание по совпадению
    TCCR1B |= (1 << CS11) | (1 << CS10); // делитель 64
}

// UART write for debug
void uart_init(uint16_t ubrr) {
    UBRR0H = static_cast<uint8_t>(ubrr >> 8);
    UBRR0L = static_cast<uint8_t>(ubrr);
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

// Main code

void set_port(int one_or_zero) { // WARNING we're breaking all other PIND sets
    PORTD = (one_or_zero << PD2);
}


class Send_State {
    static volatile bool is_send_stopped_flag;

public:
    static void trigger_send_stop() {
        is_send_stopped_flag=true;
    }
    static void trigger_send() {
        is_send_stopped_flag=false;
    }
    static bool is_send_stopped() {
        return is_send_stopped_flag;
    }
};
volatile bool Send_State::is_send_stopped_flag=false;

// volatile char message[] = "\x01Hello world Hello world Hello world Hello world Hello world Hello world Hello world Hello world Hello world Hello world\n"; // TODO тоже в инварианты Sender, или новый класс
volatile char message[] = "\x01Hello world\n"; // TODO тоже в инварианты Sender, или новый класс

class Sender {
    static volatile int bit_index;
    static volatile int byte_index;
public:
    static void send_bit() {
        int one_or_zero = (message[byte_index] & (1 << bit_index)) >> bit_index ; // DEBUG
        // uart_send_uint16(one_or_zero); // DEBUG
        set_port(one_or_zero);
        bit_index--;
        if (bit_index == -1) {
            bit_index=7;
            byte_index++;
            // uart_send_string("\r\n"); // DEBUG
        }
        if (!message[Sender::byte_index]) { // если достигли \0
            Send_State::trigger_send_stop();
            // Сбрасываем состояние до начального
            Sender::bit_index=0;
            Sender::byte_index=0;
            set_port(0);
        }
    }
};
volatile int Sender::bit_index=0; // с начала, чтобы вывести первую незначащую 1
volatile int Sender::byte_index=0;

ISR(TIMER1_COMPA_vect) {
    if (!Send_State::is_send_stopped()) {
        Sender::send_bit();
    }
}

int main() {
    // Настроить D2 (PD2) как выход
    DDRD |= (1 << PD2);

    uart_init(8);
    timer1_init_10ms_interrupt();

    sei(); // Включить глобальные прерывания

    volatile unsigned char side_effect = 0;
    while (true) {
        ++side_effect; // Побочный эффект чтобы цикл не был UB
        // TODO считывание сообщения
    }
    return 0;
}
