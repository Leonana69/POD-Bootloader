#ifndef __UUSART_H__
#define __UUSART_H__

/*
 * USART
 */
#include "stdbool.h"
#include "config.h"
#include "usart.h"

#define POLLING_TX

#define TXQ_LEN 64
#define RXQ_LEN 64

extern char rxq[RXQ_LEN];
extern int rxqTail;
extern int rxqHead;

#ifndef POLLING_TX
extern char txq[TXQ_LEN];
extern int txqTail;
extern int txqHead;
#endif

extern unsigned int dropped;
extern volatile int devnull;

int uartDebugPutchar(int c);
bool uartIsRxReady();
char uartGetc();
bool uartIsTxReady();
void uartPutc(char data);
void uartPuts(char *string);

void _UART_Init(void);

#endif