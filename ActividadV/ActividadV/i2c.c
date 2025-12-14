#include "i2c.h"

void i2c_init(void) {
	// Configurar a 100kHz con F_CPU 16MHz
	TWSR = 0x00; // Prescaler = 1
	TWBR = 72;   // Formula: ((16000000 / 100000) - 16) / 2 = 72
	TWCR = (1<<TWEN); // Habilitar TWI
}

void i2c_start(void) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT))); // Esperar a que termine
}

void i2c_stop(void) {
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
}

void i2c_write(uint8_t data) {
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT))); // Esperar a que termine
}