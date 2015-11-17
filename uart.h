#ifndef UART_H_
#define UART_H_
#include <inttypes.h>

void init_uart(int *errnum);
void uart_write_byte(uint8_t *InputBufferPtr);
void uart_write(uint8_t *InputBufferPtr, uint8_t NumBytes);
void uart_read_byte(uint8_t *OutputBufferPtr);
void uart_read(uint8_t *OutputBufferPtr, uint8_t NumBytes);

#endif /* UART_H_ */
