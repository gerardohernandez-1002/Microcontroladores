#include "usart.h"
#define F_CPU 16000000UL

void usart_init(unsigned long baud) {
	// Calculo automático para 16MHz
	unsigned int ubrr = F_CPU/16/baud - 1;
	
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	
	UCSR0B = (1<<RXEN0) | (1<<TXEN0); // Habilitar RX y TX
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // 8 bits, 1 stop
}

void usart_tx_char(char data) {
	while (!(UCSR0A & (1<<UDRE0))); // Esperar buffer vacío
	UDR0 = data;
}

void usart_print(char* str) {
	while(*str) {
		usart_tx_char(*str++);
	}
}