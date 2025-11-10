/*
 * contador.c
 *
 * 
 *  Author: gerco
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

#include "contador.h"
#include "lcd_i2c.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

/*Config timer2: prescaler 128 + OCR2A=249 -> ~2 ms */
#define TIMER2_OCRA_VALUE 249u
#define TIMER2_PRESCALER_BITS ((1<<CS22)|(1<<CS20))
#define BASE_TICK_MS 2u
#define WORK_TICK_MS 100u
#define WORK_TICKS (WORK_TICK_MS / BASE_TICK_MS) /*50*/

#define TEMP_THRESHOLD_DECI_C 250 /*25.0°C */
#define EXTINT_MSG_MS 5000u
#define EXTINT_MSG_TICKS (EXTINT_MSG_MS / WORK_TICK_MS) /*50*/

#define CONTADOR_MAX 100000u

typedef enum {
	STATE_COUNTING = 0,
	STATE_TEMP_ALERT,
	STATE_EXTINT_ALERT
} state_t;

/*Variables compartidas*/
volatile static uint32_t contador_value = 0;
volatile static state_t current_state = STATE_COUNTING;
volatile static int16_t last_temp_deciC = 0;
volatile static bool lcd_needs_update = true;

/*Temporizadores*/
volatile static uint8_t timer2_base_tick_count = 0;
volatile static uint16_t extint_ticks_left = 0;
volatile static uint8_t last_extint = 0; /*1 = INT1*/

/*Prototipos internos*/
static void itoa_u32(uint32_t val, char *buf, uint8_t bufsize);
static void show_count_on_lcd(void);
static void show_temp_alert_on_lcd(int16_t temp_deciC);
static void show_extint_on_lcd(uint8_t extNum);

/*Inicializa Timer2 (CTC) e INT1*/
void contador_init(void)
{
	contador_value = 0;
	current_state = STATE_COUNTING;
	last_temp_deciC = 0;
	lcd_needs_update = true;
	timer2_base_tick_count = 0;
	extint_ticks_left = 0;
	last_extint = 0;

	/* Timer2 CTC */
	TCCR2A |= (1 << WGM21);
	OCR2A = TIMER2_OCRA_VALUE;
	TIMSK2 |= (1 << OCIE2A); /*COMPA interrupt*/

	/*INT1 en falling edge (botón a GND con pull-up)*/
	EICRA &= ~(1 << ISC10);
	EICRA |= (1 << ISC11);
	EIMSK |= (1 << INT1);
}

/*Arranca Timer2*/
void contador_start(void)
{
	TCCR2B &= ~((1<<CS22)|(1<<CS21)|(1<<CS20));
	TCCR2B |= TIMER2_PRESCALER_BITS;
	sei();
}

/*Para Timer2*/
void contador_stop(void)
{
	TCCR2B &= ~((1<<CS22)|(1<<CS21)|(1<<CS20));
}

/*Actualiza LCD si es necesario*/
void contador_loop(void)
{
	if (!lcd_needs_update) return;

	switch (current_state) {
		case STATE_COUNTING:
		show_count_on_lcd();
		break;
		case STATE_TEMP_ALERT:
		show_temp_alert_on_lcd(last_temp_deciC);
		break;
		case STATE_EXTINT_ALERT:
		show_extint_on_lcd(last_extint);
		break;
		default:
		show_count_on_lcd();
		break;
	}
	lcd_needs_update = false;
}

/*Recibe temperatura en ºC*/
void contador_set_temperature_deciC(int16_t temp_deciC)
{
	last_temp_deciC = temp_deciC;
	if (temp_deciC > TEMP_THRESHOLD_DECI_C) {
		current_state = STATE_TEMP_ALERT;
		lcd_needs_update = true;
		} else {
		if (current_state == STATE_TEMP_ALERT) {
			current_state = STATE_COUNTING;
			lcd_needs_update = true;
		}
	}
}

/*Trigger desde ISR propio*/
void contador_extint_trigger(uint8_t extIntNum)
{
	if (extIntNum != 1) return;
	last_extint = extIntNum;
	current_state = STATE_EXTINT_ALERT;
	extint_ticks_left = EXTINT_MSG_TICKS;
	lcd_needs_update = true;
}

/*Lectura segura del contador*/
uint32_t contador_get_value(void)
{
	uint32_t tmp;
	cli();
	tmp = contador_value;
	sei();
	return tmp;
}

/*Reset*/
void contador_reset(void)
{
	cli();
	contador_value = 0;
	sei();
	lcd_needs_update = true;
}

/*Convierte uint32 a string*/
static void itoa_u32(uint32_t val, char *buf, uint8_t bufsize)
{
	if (bufsize == 0) return;
	if (val == 0) {
		if (bufsize > 1) { buf[0] = '0'; buf[1] = '\0'; }
		else buf[0] = '\0';
		return;
	}
	char rev[12];
	uint8_t i = 0;
	while (val > 0 && i < sizeof(rev)) {
		rev[i++] = '0' + (val % 10);
		val /= 10;
	}
	uint8_t j = 0;
	if (i + 1 > bufsize) {
		uint8_t keep = bufsize - 1;
		for (j = 0; j < keep; ++j) buf[j] = rev[keep - 1 - j];
		buf[keep] = '\0';
		return;
	}
	while (i--) buf[j++] = rev[i];
	buf[j] = '\0';
}

/*Muestra contador*/
static void show_count_on_lcd(void)
{
	char buf[12];
	cli();
	uint32_t val = contador_value;
	sei();
	lcd_i2c_clr();
	lcd_i2c_col_row(0, 0);
	lcd_i2c_write_string("CONTADOR");
	lcd_i2c_col_row(0, 1);
	itoa_u32(val, buf, sizeof(buf));
	lcd_i2c_write_string(buf);
}

/*Muestra alerta temp*/
static void show_temp_alert_on_lcd(int16_t temp_deciC)
{
	char buf[16];
	lcd_i2c_clr();
	lcd_i2c_col_row(0, 0);
	lcd_i2c_write_string("ALERTA: TEMP");
	lcd_i2c_col_row(0, 1);
	int16_t whole = temp_deciC / 10;
	int16_t frac = temp_deciC % 10;
	if (frac < 0) frac = -frac;
	int pos = 0;
	if (whole == 0) { buf[pos++] = '0'; }
	else {
		int16_t t = whole;
		char rev[6];
		uint8_t r = 0;
		bool neg = false;
		if (t < 0) { neg = true; t = -t; }
		while (t > 0 && r < sizeof(rev)) {
			rev[r++] = '0' + (t % 10);
			t /= 10;
		}
		if (neg) buf[pos++] = '-';
		while (r--) buf[pos++] = rev[r];
	}
	buf[pos++] = '.';
	buf[pos++] = '0' + (uint8_t)frac;
	buf[pos++] = 'C';
	buf[pos] = '\0';
	lcd_i2c_write_string(buf);
}

/*Muestra INT1*/
static void show_extint_on_lcd(uint8_t extNum)
{
	lcd_i2c_clr();
	lcd_i2c_col_row(0, 0);
	if (extNum == 1) lcd_i2c_write_string("INT1 DETECTADA");
	else lcd_i2c_write_string("INT DESCONOCIDA");
	lcd_i2c_col_row(0, 1);
	lcd_i2c_write_string("Esperando...");
}

/*TIMER2 COMPA: base ~2 ms*/
ISR(TIMER2_COMPA_vect)
{
	timer2_base_tick_count++;
	if (timer2_base_tick_count >= WORK_TICKS) {
		timer2_base_tick_count = 0;
		if (current_state == STATE_COUNTING) {
			contador_value++;
			if (contador_value > CONTADOR_MAX) contador_value = 0;
			lcd_needs_update = true;
			} else if (current_state == STATE_EXTINT_ALERT) {
			if (extint_ticks_left > 0) {
				extint_ticks_left--;
				if (extint_ticks_left == 0) {
					current_state = STATE_COUNTING;
					lcd_needs_update = true;
				}
			}
		}
	}
}

/*INT1: botón*/
ISR(INT1_vect)
{
	contador_extint_trigger(1);
}