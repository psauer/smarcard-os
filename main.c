#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <util/delay.h>
#include <inttypes.h>

#include "config.h"
#include "uart.h"
#include "debug_uart.h"
#include "common.h"
#include "aes.h"

static void get_command(APDU_command *cmd, APDU_response *response, int *errnum) {
  uint8_t i;
  uint8_t in_buffer[6];

  // reading command from uart. cmd is 5 bytes long
  for (i = 0; i < 5; i++) {
    uart_read_byte(&in_buffer[i]);
  }

  cmd->cla = in_buffer[0];
  cmd->ins = in_buffer[1];
  cmd->p1  = in_buffer[2];
  cmd->p2  = in_buffer[3];
  cmd->p3  = in_buffer[4];

  if (cmd->cla != CLA) {
    // CLA invalid and aborting
    response->sw1 = 0x6e;
    response->sw2 = 0x00;
    *errnum       = ABORT_ERROR;
  }
}

static void execute_command(APDU_command  *cmd,
                            APDU_response *response,
                            int           *errnum) {
  switch (cmd->ins) {
  case DECRYPT_DATA:

    // sending ack
    uart_write_byte(&cmd->ins);

    // read data from UART
    uart_read(cmd->data, cmd->p3);

    // decode data
    aes_decrypt(cmd->data);

    // send status bytes
    response->sw1 = 0x61;
    response->sw2 = 0x10;
    *errnum       = NO_ERROR;
    break;

  case GET_RESPONSE:

    // sending ack
    uart_write_byte(&cmd->ins);

    // sending decrypted data
    uart_write(cmd->data, cmd->p3);

    // send status bytes
    response->sw1 = 0x90;
    response->sw2 = 0x00;
    *errnum       = NO_ERROR;
    break;

  default:

    // command not supported
    response->sw1 = 0x6d;
    response->sw2 = 0x00;
    *errnum       = WARNING_ERROR;
  }

  // writing status bytes
  uart_write_byte(&response->sw1);
  uart_write_byte(&response->sw2);
}

int main(void) {
  int errnum = 0;
  APDU_command  cmd;
  APDU_response response;
  uint8_t ATR[4] = { 0x3B, 0x90, 0x11, 0x00 };

  // init functions here
  init_uart(&errnum);
  INIT_LED_PINS();
  #ifdef DEBUG
  init_debug_uart();
  #endif /* ifdef DEBUG */

  // send ATR
  uart_write(ATR, 4);

  while (1) {
    // getting command from uart
    get_command(&cmd, &response, &errnum);

    if (errnum == ABORT_ERROR) {
      uart_write_byte(&response.sw1);
      uart_write_byte(&response.sw2);
      break;
    }

    // running command
    execute_command(&cmd, &response, &errnum);

    if (errnum == ABORT_ERROR) {
      break;
    }
  }
  return 0;
}
