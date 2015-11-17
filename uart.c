#include "uart.h"
#include "common.h"

void init_uart(int *errnum) {
    //insert code here
    
    *errnum = NO_ERROR;
    return;
}

void uart_write_byte(uint8_t *InputBufferPtr) {
    //insert code here
}

void uart_write(uint8_t *InputBufferPtr, uint8_t NumBytes) {
	uint8_t i;

    for(i = 0; i < NumBytes; i++) {
    	sim_uart_write_byte(&InputBufferPtr[i]);
    }
}

void smartcard_uart_read_byte(uint8_t *OutputBufferPtr) {
    //insert code here
}

void sim_uart_read(uint8_t *OutputBufferPtr, uint8_t NumBytes) {
	uint8_t i = 0;

	for (i = 0; i < NumBytes; i++) {
		sim_uart_read_byte(&OutputBufferPtr[i]);
	}
}
