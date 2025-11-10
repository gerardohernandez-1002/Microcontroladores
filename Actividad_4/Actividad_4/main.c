/*
 * main.c
 * Botón INT1 en PD3 con pull-up.
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "lcd_i2c.h"
#include "contador.h"

/*ADC simple para LM35 en ADC0*/
static void adc_init(void)
{
    ADMUX = 0;
    ADMUX |= (1 << REFS0); /*AVcc*/
    ADMUX = (ADMUX & 0xF0) | 0; /*ADC0*/
    ADCSRA = 0;
    ADCSRA |= (1 << ADEN);
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); /* prescaler 128 */
}

/*Lee ADC0 y retorna deci-ºC*/
static int16_t leer_temp_LM35_deciC(void)
{
    ADMUX = (ADMUX & 0xF0) | 0;
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    uint16_t adc = ADC;
    uint32_t mV = (uint32_t)adc * 5000UL;
    mV = (mV + 511UL) / 1023UL;
    if (mV > 32767) return 32767;
    return (int16_t)mV;
}

int main(void)
{
    lcd_i2c_init();
    adc_init();

    /*PD3 entrada con pull-up (botón a GND)*/
    DDRD &= ~(1 << PD3);
    PORTD |= (1 << PD3);

    contador_init();
    contador_start();

    sei();

    for (;;) {
        int16_t temp_deciC = leer_temp_LM35_deciC();
        contador_set_temperature_deciC(temp_deciC);
        contador_loop();
        _delay_ms(100);
    }
    return 0;
}
