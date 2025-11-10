/*
 * contador.h
 *
 *  Author: gerco
 */ 
#ifndef CONTADOR_H_
#define CONTADOR_H_

#include <stdint.h>
#include <stdbool.h>

/*Inicializa el módulo (configura Timer2 y las interrupciones externas */
void contador_init(void);

/*Arranca el conteo (habilita Timer2)*/
void contador_start(void);

/*Detiene el conteo (para Timer2)*/
void contador_stop(void);

/*Llamar periódicamente desde main() para refrescar LCD si hace falta*/
void contador_loop(void);

/*Notificar temperatura en deci-ºC (ej: 250 => 25.0°C)*/
void contador_set_temperature_deciC(int16_t temp_deciC);

/*Notificar desde ISR externo si prefieres manejar ISRs en otro módulo.
   extIntNum: 0 -> INT0, 1 -> INT1 */
void contador_extint_trigger(uint8_t extIntNum);

/*Obtener valor actual del contador*/
uint32_t contador_get_value(void);

/*Reiniciar contador a 0*/
void contador_reset(void);

#endif /* CONTADOR_H_ */
