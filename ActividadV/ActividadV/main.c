/*
 * main.c
 *
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

// Librerías de periféricos
#include "usart.h"
#include "adc.h"
#include "i2c.h"
#include "lcd_i2c.h"
#include "ext_int.h"

// Variable volátil para control de estado por interrupción
volatile uint8_t bandera_emergencia = 0;

/*
 * Rutina de Servicio de Interrupción (ISR) - INT0
 * Función: Alternar el estado de emergencia del sistema.
 */
ISR(INT0_vect)
{
    bandera_emergencia = !bandera_emergencia;
    
    // Retardo para debouncing (anti-rebote)
    _delay_ms(150); 
    
    // Limpiar bandera de interrupción externa
    EIFR |= (1 << INTF0);
}

int main(void)
{
    // Variables locales
    char buffer[40];
    uint16_t valor_adc = 0;
    
    // Variables para aritmética de punto fijo (evita uso de float en sprintf)
    uint32_t milivoltios = 0;
    uint8_t voltaje_entero = 0;
    uint8_t voltaje_decimal = 0;

    // --- Inicialización de Hardware ---
    
    // 1. Comunicación Serial
    usart_init(9600); 
    usart_print("--- Sistema Iniciado ---\r\n");

    // 2. Convertidor Analógico Digital
    adc_init();

    // 3. Interrupciones Externas
    ext_int_init();

    // 4. Habilitar Interrupciones Globales
    sei(); 

    // 5. Inicialización de Pantalla LCD (I2C)
    i2c_init();
    lcd_i2c_init();
    
    // Pantalla de bienvenida
    lcd_i2c_clr();
    lcd_i2c_set_cursor(1, 1);
    lcd_i2c_print("Sistema Activo");
    _delay_ms(1000);
    lcd_i2c_clr();

    // --- Bucle Principal ---
    while (1) 
    {
        if (bandera_emergencia) 
        {
            // Estado de Alerta
            lcd_i2c_set_cursor(1, 1);
            lcd_i2c_print("!! ALERTA !!    ");
            lcd_i2c_set_cursor(1, 2);
            lcd_i2c_print("SISTEMA DETENIDO");
            
            usart_print("ALERTA: Paro de emergencia activado.\r\n");
            _delay_ms(500);
        } 
        else 
        {
            // Operación Normal
            
            // Adquisición de datos
            valor_adc = adc_read();

            // Conversión a voltaje (0 - 5000 mV)
            // Se utiliza aritmética entera para optimizar recursos
            milivoltios = ((uint32_t)valor_adc * 5000) / 1023;
            
            voltaje_entero = milivoltios / 1000;
            voltaje_decimal = (milivoltios % 1000) / 10;

            // Visualización en LCD
            lcd_i2c_set_cursor(1, 1);
            sprintf(buffer, "ADC: %4d       ", valor_adc);
            lcd_i2c_print(buffer);

            lcd_i2c_set_cursor(1, 2);
            sprintf(buffer, "Volt: %d.%02d V   ", voltaje_entero, voltaje_decimal);
            lcd_i2c_print(buffer);

            // Transmisión de datos por Serial
            sprintf(buffer, "Lectura: %d | Voltaje: %d.%02d V\r\n", valor_adc, voltaje_entero, voltaje_decimal);
            usart_print(buffer);

            // Frecuencia de muestreo
            _delay_ms(500); 
        }
    }
}