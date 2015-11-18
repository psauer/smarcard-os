// Adapted from https://www.mikrocontroller.net/articles/AVR-GCC-Tutorial#Die_Nutzung_von_sprintf_und_printf
#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>

#include "config.h"
#include "debugUART.h"


static FILE uartStdout = FDEV_SETUP_STREAM( uartPutChar, NULL, _FDEV_SETUP_WRITE );

int uartPutChar( uint8_t c, FILE *stream )
{
  if( c == '\n' ) {
    uartPutChar( '\r', stream );
  }

  while( !(UCSR0A & (1 << UDRE0)) );
  UDR0 = c;
  return 0;
}

uint8_t uartGetChar()
{
  while( !(UCSR0A & (1 << RXC0)) );

  return UDR0;
}

void init_debug_uart()
{
  // Turn on UART TX and RX
  UCSR0B |= (1 << TXEN0) | (1 << RXEN0);
  // Asynchronous 8N1
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

  UBRR0H = (uint8_t) ( UART_UBRR_CALC( UART_BAUD_RATE, F_CPU ) >> 8 );
  UBRR0L = (uint8_t) UART_UBRR_CALC( UART_BAUD_RATE, F_CPU );

  stdout = &uartStdout;
}
