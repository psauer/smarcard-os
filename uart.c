#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>

#include "config.h"
#include "uart.h"
#include "common.h"

volatile uint8_t etuFlag, startBitFlag;

//
// Interrupts

// ETU interrupt
ISR(TIMER0_COMPA_vect) {
  etuFlag = 1;
}

// Smartcard io pin change interupt
ISR(PCINT1_vect) {
  // start bit?
  if (((PINB >> SMARTCARD_IO) & 0x01) == 0) {
    startBitFlag = 1;
  }
}

//
// Static functions
static void setEtuCycleBit(uint8_t value) {
  if (value) {
    SMARTCARD_HIGH_Z();
  } else {
    SMARTCARD_LOW();
  }
}

static void waitForStartBit(void) {
  startBitFlag = 0;

  // enable pin change interupt on smartcard io
  PCMSK1 |= (1 << PCINT14);
  PCICR |= (1 << PCIE1);

  // wait for the start bit
  while (!startBitFlag);

  // disable pin change interupt on smartcard io
  PCICR &= ~(1 << PCIE1);
  PCMSK1 &= ~(1 << PCINT14);
}

static void enableTimer(void) {
  // enable output compare event A
  TIMSK0 |= (1 << OCIE0A);
  // enable counter 0 and set prescaler to 8 => 46 * 8 ~ 372
  TCCR0B |= (1 << CS01);
}

static void resetTimer(void) {
  // disable output compare event A
  TIMSK0 &= ~(1 << OCIE0A);
  // disable counter 0
  TCCR0B &= !(1 << CS01);
  // reset counter 0
  TCNT0 = 0;
}

void init_uart(int *errnum) {
  // idle smartcard io
  setEtuCycleBit(1);

  // reset timer 0
  TCNT0 = 0;
  // enable CTC mode
  TCCR0A |= (1 << WGM01);
  // set output compare register A to 1 ETU (46)
  OCR0A = 0x2E;
  // enable global interupt
  sei();

  *errnum = NO_ERROR;
}

void uart_write_byte(uint8_t *InputBufferPtr) {
  enum iso7816_states_t state = START_BIT;
  uint8_t etuCycle = 0;
  uint8_t parityBit = 0;
  uint8_t payloadCopy = 0;

  #if DEBUG_ISO7816 == 1
    printf("Sending payload 0x%X.\n", *InputBufferPtr);
  #endif

  // set etuFlag to 1 so that START_BIT is set immediately
  etuFlag = 1;

  enableTimer();

  while (state != DONE) {
    // evaluate once every etu cycle
    if (etuFlag) {
      etuFlag = 0;

      switch (state) {
        case START_BIT:
          setEtuCycleBit(0);
          etuCycle++;

          payloadCopy = *InputBufferPtr;

          state = DATA_BITS;
          break;

        case DATA_BITS:
          setEtuCycleBit(payloadCopy & 0x01);
          parityBit ^= (payloadCopy & 0x01);

          payloadCopy >>= 1;
          etuCycle++;

          if (etuCycle == 9) {
            state = PARITY_BIT;
          }

          break;

        case PARITY_BIT:
          setEtuCycleBit(parityBit);

          etuCycle++;
          state = STOP_BITS;

          break;

        case STOP_BITS:
          setEtuCycleBit(1);
          etuCycle++;

          if (etuCycle == 12) {
            // parity error?
            if ((PINB >> SMARTCARD_IO) & 0x01) {
              state = DONE;
            } else {
              state = PARITY_ERROR;
            }
          }

          break;

        case PARITY_ERROR:
          setEtuCycleBit(1);
          state = START_BIT;
          etuCycle = 0;
          parityBit = 0;

          break;

        case DONE:
          break;
      }
    }
  }

  // wait for the last etu cycle to finish
  while (!etuFlag);
  // idle smartcard IO
  SMARTCARD_HIGH_Z();

  resetTimer();
}

void uart_write(uint8_t *InputBufferPtr, uint8_t NumBytes) {
  uint8_t i;

  for(i = 0; i < NumBytes; i++) {
    uart_write_byte(&InputBufferPtr[i]);
  }
}

void uart_read_byte(uint8_t *OutputBufferPtr) {
  enum iso7816_states_t state = START_BIT;
  uint8_t payload = 0;
  uint8_t etuCycle = 0;
  uint8_t etuCycleBit = 0;
  uint8_t parityBit = 0;

  etuFlag = 1;

  waitForStartBit();

  // The delay between the detection of the start bit and start of the timer is sufficient to ensure the bit on the smartcard IO is always read when it is stable
  enableTimer();

  while (state != DONE) {
    if (etuFlag) {
      etuFlag = 0;

      switch (state) {
        case START_BIT:
          state = DATA_BITS;
          break;

        case DATA_BITS:
          etuCycleBit = (PINB >> SMARTCARD_IO) & 0x01;
          parityBit ^= etuCycleBit;
          payload |= etuCycleBit << etuCycle;

          etuCycle++;

          if (etuCycle == 8) {
            state = PARITY_BIT;
          }

          break;

        case PARITY_BIT:
          etuCycleBit = (PINB >> SMARTCARD_IO) & 0x01;
          etuCycle++;
          state = STOP_BITS;

          break;

        case STOP_BITS:
          // parity error?
          if (etuCycleBit == parityBit) {
            etuCycle++;

            if (etuCycle == 11) {
              state = DONE;
            }
          } else {
            setEtuCycleBit(0);
            state = PARITY_ERROR;
          }

          break;

        case PARITY_ERROR:
          setEtuCycleBit(1);

          resetTimer();
          etuFlag = 1;
          etuCycle = 0;
          parityBit = 0;
          payload = 0;
          state = START_BIT;

          waitForStartBit();

          enableTimer();

          break;

        case DONE:
          break;
      }
    }
  }

  resetTimer();

  #if DEBUG_ISO7816 == 1
    printf("Received payload 0x%X.\n", payload);
  #endif

    *OutputBufferPtr = payload;
}

void uart_read(uint8_t *OutputBufferPtr, uint8_t NumBytes) {
  uint8_t i = 0;

  for (i = 0; i < NumBytes; i++) {
    uart_read_byte(&OutputBufferPtr[i]);
  }
}
