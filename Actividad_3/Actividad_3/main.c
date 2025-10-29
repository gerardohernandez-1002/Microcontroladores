/*
 * main.c
 * Control de dos servos (OC1A, OC1B)
 * F_CPU = 16MHz
 * Servo 1 (OC1A) -> Pin 9 (PB1)
 * Servo 2 (OC1B) -> Pin 10 (PB2)
*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

// Constantes de Posición (0.7ms a 2.3ms)
// Basado en Prescaler=8, Tick = 0.5 µs
#define POS_0_GRADOS    1400  // 0.7 ms
#define POS_90_GRADOS   3000  // 1.5 ms
#define POS_180_GRADOS  4600  // 2.3 ms

// Tiempo de espera entre movimientos
#define DELAY_PASO 1000

/**
 * Salidas: OC1A (Pin 9) y OC1B (Pin 10)
 */
void setup_timer1_pwm_servos(void)
{
    //Configurar pines PB1 (Pin 9) y PB2 (Pin 10) como salidas
    //(Corresponde a DDRB |= (1 << DDB1) | (1 << DDB2); 
    DDRB |= (1 << PB1) | (1 << PB2);

    //Configurar Modo 14 (Fast PWM, TOP = ICR1)
    TCCR1A |= (1 << WGM11);
    TCCR1B |= (1 << WGM12) | (1 << WGM13);

    //Configurar Modo no-inversor en ambos canales
    TCCR1A |= (1 << COM1A1) | (1 << COM1B1);

    //Establecer TOP (Periodo de 20ms / 50Hz)
    //ICR1 = 39999 (calculado para 16MHz y Prescaler=8)
    ICR1 = 39999;

    //Iniciar el temporizador (Prescaler N=8)
    TCCR1B |= (1 << CS11);
}


int main(void)
{
    setup_timer1_pwm_servos();

    // Posición inicial
    OCR1A = POS_0_GRADOS; 
    OCR1B = POS_0_GRADOS;
    _delay_ms(DELAY_PASO);

    while (1)
    {
        //SECUENCIA 1: Servo 1 de 0° 90° 180° 
        OCR1A = POS_90_GRADOS;
        _delay_ms(DELAY_PASO);
        OCR1A = POS_180_GRADOS;
        _delay_ms(DELAY_PASO);

        //SECUENCIA 2: Servo 2 de 0° 90° 180° 
        OCR1B = POS_90_GRADOS;
        _delay_ms(DELAY_PASO);
        OCR1B = POS_180_GRADOS;
        _delay_ms(DELAY_PASO);

        //SECUENCIA 3: Servo 1 de 180° 90° 0° 
        OCR1A = POS_90_GRADOS;
        _delay_ms(DELAY_PASO);
        OCR1A = POS_0_GRADOS;
        _delay_ms(DELAY_PASO);

        //SECUENCIA 4: Servo 2 de 180° 90° 0° 
        OCR1B = POS_90_GRADOS;
        _delay_ms(DELAY_PASO);
        OCR1B = POS_0_GRADOS;
        _delay_ms(DELAY_PASO);
    }
}

