#ifndef DEBUGUART_H
#define DEBUGUART_H


#define UART_UBRR_CALC(BAUD_,FREQ_) ((FREQ_)/((BAUD_)*16L)-1)


void init_debug_uart(void);
int uartPutChar(uint8_t c, FILE *stream);
uint8_t uartGetChar(void);


#endif
