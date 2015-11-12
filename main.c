#include <avr/io.h>
#include <util/delay.h>

int main(void) {
  DDRB |= 0x04;

  while (1) {
    PORTB ^= 0x04;
    _delay_ms(250);
  }
}
