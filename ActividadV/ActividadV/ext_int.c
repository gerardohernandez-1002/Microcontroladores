#include "ext_int.h"

void ext_int_init(void) {
	// PD2 como entrada (por seguridad, aunque el reset lo hace)
	DDRD &= ~(1 << DDD2);
	// Habilitar resistencia Pull-Up interna en PD2 (OPCIONAL pero buena práctica)
	PORTD |= (1 << PORTD2);
	
	// Configurar INT0 para flanco de bajada (Falling Edge)
	EICRA |= (1 << ISC01);
	EICRA &= ~(1 << ISC00);
	
	// Habilitar la interrupción INT0
	EIMSK |= (1 << INT0);
}