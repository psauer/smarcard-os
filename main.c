#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include "uart.h"
#include "common.h"
#include "aes.h"

#define CLA 0x88
#define GET_RESPONSE 0xC0
#define DECRYPT_DATA 0x10

typedef struct APDU_command_t{
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t p3;
    uint8_t data[16];
} APDU_command;

typedef struct APDU_response_t{
    uint8_t sw1;
    uint8_t sw2;
} APDU_response;

uint8_t key[16] = { 0xF3, 0x54, 0x1F, 0xA3, 0x4B, 0x33, 0x9C, 0x0D,
                   0x80, 0x23, 0x7A, 0xF9, 0x7C, 0x21, 0xD7, 0x3B };

int main(void) {
 
    int errnum = 0;

    APDU_command cmd;

    APDU_response response;
    ATR uint8_t = {0x3B, 0x90, 0x11, 0x00}; 
    //init functions here
    
    // send ATR    
    sim_uart_write(ATR, 4);
    while(1){
        //getting command from uart
        get_command(&cmd, &response, &errnum);
        if (errnum == ABORT_ERROR) {
            sim_uart_write_byte(&response.sw1);
            sim_uart_write_byte(&response.sw2);
            break;
        }
        //running command
        execute_command(&cmd, &response, &errnum);
        //wrting status bytes
        sim_uart_write_byte(&response.sw1);
        sim_uart_write_byte(&response.sw2);

        if (errnum == ABORT_ERROR) {
            break;
        }
    }
    return 0;
}
static void get_command(APDU_command *cmd, APDU_response *response, int *errnum) {
    int i = 0;
    uint8_t in_buffer[6];

    //reading command from uart. cmd is 5 bytes long
    for(i=0; i<5; i++) {
        sim_uart_read_byte(&in_buffer[i]);
    }

    cmd->cla = in_buffer[0]; 
    cmd->ins = in_buffer[1]; 
    cmd->p1 = in_buffer[2]; 
    cmd->p2 = in_buffer[3]; 
    cmd->p3 = in_buffer[4]; 

    if (cmd->cla != CLA) {
        //CLA invalid and aborting
        response->sw1 = 0x6e;
        response->sw2 = 0x00;
        *errnum = ABORT_ERROR;
    }
}

static void execute_command(APDU_command *cmd, APDU_response *response, int *errnum) {
    switch (cmd->ins) {
    case DECRYPT_DATA:
		//sending ack
		sim_uart_write_byte(&cmd->ins);
        //read data from UART
        sim_uart_read(cmd->data, cmd->p3);
        //decode data
        aes_decrypt(key, cmd->data);
        //send status bytes
        response->sw1 = 0x61;
        response->sw2 = 0x10;
		*errnum = NO_ERROR;
        break;
	case GET_RESPONSE:
		//sending ack
		sim_uart_write_byte(&cmd->ins);
        //sending decrypted data
        sim_uart_write(cmd->data, cmd->p3);
        //send status bytes
        response->sw1 = 0x90;
		response->sw2 = 0x00;
		*errnum = NO_ERROR;
        break;
    default:
        //command not supported
        response->sw1 = 0x6d;
        response->sw2 = 0x00;
        *errnum = WARNING_ERROR;
    }
}