#include "adc.h"

void adc_init(void) {
	// AVcc como referencia
	ADMUX = (1 << REFS0);
	// Habilitar ADC, Prescaler 128 (16MHz/128 = 125kHz)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t adc_read(void) {
	ADMUX &= 0xF0; // Seleccionar canal 0 (PC0)
	ADCSRA |= (1 << ADSC); // Iniciar conversión
	while (ADCSRA & (1 << ADSC)); // Esperar
	return ADC;
}