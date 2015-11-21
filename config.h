#ifndef CONFIG_H
#define CONFIG_H


#define DEBUG

#ifdef DEBUG

// WARNING: enabling DEBUG_UART will cause the smartcard to stop working due to the huge delays from the printf statements
#define DEBUG_UART 0

#endif

#define UART_BAUD_RATE 9600
#define CLA 0x88
#define GET_RESPONSE 0xC0
#define DECRYPT_DATA 0x10

#define LED_0 PA0
#define LED_1 PA1
#define LED_2 PA2
#define LED_3 PA3
#define LED_4 PA4
#define LED_5 PA5
#define LED_6 PA6
#define LED_7 PA7
#define LED_DEBUG PB2

#define INIT_LED_PINS() DDRA = 0xFF; DDRB |= 0x04; PORTA = 0; PORTB |= 0x04;


#define SMARTCARD_IO PB6

#define SMARTCARD_HIGH_Z() DDRB &= 0xBF; PORTB |= 0x40;
#define SMARTCARD_LOW() DDRB |= 0x40; PORTB &= 0xBF;

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

#endif
