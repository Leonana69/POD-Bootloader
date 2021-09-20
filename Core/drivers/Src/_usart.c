#include "_usart.h"
#include "usart.h"

char rxq[RXQ_LEN];
int rxqTail;
int rxqHead;

#ifndef POLLING_TX
char txq[TXQ_LEN];
int txqTail;
int txqHead;
#endif

unsigned int dropped;
volatile int devnull;

void _USART3_UART_Init(void) {
#ifndef POLLING_TX
  __HAL_UART_ENABLE_IT(&huart6, UART_IT_TC);
#endif
  __HAL_UART_ENABLE_IT(&huart6, UART_IT_RXNE);


  // enable flow control GPIO
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = READ_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(READ_PIN_PORT, &GPIO_InitStruct);
}

bool uartIsRxReady() {
  return rxqHead != rxqTail;
}

char uartGetc() {
  char data = 0;
  if (uartIsRxReady()) {
    data = rxq[rxqTail];
    rxqTail = (rxqTail + 1) % RXQ_LEN;
  }
  return data;
}

bool uartIsTxReady() {
#ifdef POLLING_TX
  return true;
#else
  return ((txqHead + 1) % TXQ_LEN) != txqTail;
#endif
}

void uartPutc(char data) {
#ifdef POLLING_TX
  while (HAL_GPIO_ReadPin(READ_PIN_PORT, READ_PIN) == GPIO_PIN_SET);
  HAL_UART_Transmit(&uartMain, (uint8_t*) &data, 1, 100);
  while (__HAL_UART_GET_FLAG(&uartMain, UART_FLAG_TC) == 0);
#else
  if (uartIsTxReady()) {
    txq[txqHead] = data;
    txqHead = ((txqHead + 1) % TXQ_LEN);

    if (__HAL_UART_GET_FLAG(&uartMain, UART_FLAG_TXE)) {
      HAL_UART_Transmit(&uartMain, &txq[txqTail], 1, 100);
      txqTail = (txqTail + 1) % TXQ_LEN;
    }
  } else {
	  dropped++;
  }
#endif
}

void uartPuts(char *string) {
  while (*string) {
    uartPutc(*string++);
  }
}

int uartDebugPutchar(int c) {
    HAL_UART_Transmit(&uartDebug, (uint8_t*) &c, 1, 100);
    return (unsigned char) c;
}