#ifndef USART_H_
#define USART_H_
#include <avr/io.h>

void usart_init(unsigned long baud);
void usart_tx_char(char data);
void usart_print(char* str);

#endif