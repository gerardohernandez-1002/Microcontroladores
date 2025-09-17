
.include "m328pdef.inc"

;Definimos registros
.def temp_reg = r16       ;Propósito general
.def input_val = r17      ;Almacenar el valor de entrada

.org 0x0000
    rjmp    RESET          ;Salto al inicio del programa

RESET:
    ;Configura el Stack Pointer
    ldi     temp_reg, low(RAMEND)
    out     SPL, temp_reg
    ldi     temp_reg, high(RAMEND)
    out     SPH, temp_reg

    ;Entradas y Salidas
    ;Configurar PB5 como salida (pin 13 del Arduino) 
    sbi     DDRB, 5        ;bit 5 de DDRB en 1

    ; Configurar PD2 y PD3 como entradas 
    cbi     DDRD, 2        ;bit 2 de DDRD en 0
    cbi     DDRD, 3        ;bit 3 de DDRD en 0

    ; Resistencias Pull up
    sbi     PORTD, 2       ;Pone el bit 2 de PORTD en 1
    sbi     PORTD, 3       ;Pone el bit 3 de PORTD en 1

;Loop principal
MAIN_LOOP:
    ;Leer el estado de los pines de entrada del PORTD
    in      input_val, PIND

    ;Aislar los bits 2 y 3 para la comparación
    andi    input_val, 0b00001100

    ;Comparar con los valores de la tabla para seleccionar la frecuencia 
    ;Las entradas se leen como 0 cuando el pin está a tierra (activo bajo)
    ;Entrada 00 (PD3=0, PD2=0) -> Valor 0b00000000
    cpi     input_val, 0b00000000
    breq    GEN_5KHZ

    ;Entrada 01 (PD3=0, PD2=1) -> Valor 0b00000100
    cpi     input_val, 0b00000100
    breq    GEN_25KHZ

    ;Entrada 10 (PD3=1, PD2=0) -> Valor 0b00001000
    cpi     input_val, 0b00001000
    breq    GEN_60KHZ

    ;Entrada 11 (PD3=1, PD2=1) -> Valor 0b00001100
    cpi     input_val, 0b00001100
    breq    GEN_80KHZ

    rjmp    MAIN_LOOP      ;Volver a leer

;Subrutinas de generación de frecuencia
GEN_5KHZ:
    sbi     PORTB, 5       ; Pone PB5 en ALTO
    rcall   DELAY_100us    ; Retardo de medio periodo (100us)
    cbi     PORTB, 5       ; Pone PB5 en BAJO
    rcall   DELAY_100us    ; Retardo de medio periodo (100us)
    rjmp    MAIN_LOOP      ; Regresar a leer las entradas

GEN_25KHZ:
    sbi     PORTB, 5
    rcall   DELAY_20us
    cbi     PORTB, 5
    rcall   DELAY_20us
    rjmp    MAIN_LOOP

GEN_60KHZ:
    sbi     PORTB, 5
    rcall   DELAY_8_3us
    cbi     PORTB, 5
    rcall   DELAY_8_3us
    rjmp    MAIN_LOOP

GEN_80KHZ:
    sbi     PORTB, 5
    rcall   DELAY_6_25us
    cbi     PORTB, 5
    rcall   DELAY_6_25us
    rjmp    MAIN_LOOP

;Subrutinas de retardo
;
; El ciclo de llamada (RCALL) y retorno (RET) también consumen ciclos.

; Retardo de ~100us para 5KHz (1600 ciclos)
DELAY_100us:
    ldi     r18, 5         ; 1 ciclo
    ldi     r19, 106       ; 1 ciclo
L1:
    dec     r19            ; 1 ciclo * 106
    brne    L1             ; 2 ciclos * 105, 1 ciclo * 1
    dec     r18            ; 1 ciclo * 5
    brne    L1             ; 2 ciclos * 4, 1 ciclo * 1
    ret                    ; 4 ciclos
    ; Total ~ 5 * (1 + 106*3) = ~1600 ciclos

; Retardo de ~20us para 25KHz (320 ciclos)
DELAY_20us:
    ldi     r18, 105       ; 1 ciclo
L2:
    dec     r18            ; 1 ciclo * 105
    brne    L2             ; 2 ciclos * 104, 1 ciclo * 1
    nop                    ; 1 ciclo
    ret                    ; 4 ciclos
    ; Total ~ 1 + 105*3 + 1 + 4 = ~321 ciclos

; Retardo de ~8.3us para 60KHz (133 ciclos)
DELAY_8_3us:
    ldi     r18, 43        ; 1 ciclo
L3:
    dec     r18            ; 1 ciclo * 43
    brne    L3             ; 2 ciclos * 42, 1 ciclo * 1
    ret                    ; 4 ciclos
    ; Total ~ 1 + 43*3 + 4 = ~134 ciclos

; Retardo de ~6.25us para 80KHz (100 ciclos)
DELAY_6_25us:
    ldi     r18, 32        ; 1 ciclo
L4:
    dec     r18            ; 1 ciclo * 32
    brne    L4             ; 2 ciclos * 31, 1 ciclo * 1
    ret                    ; 4 ciclos
    ; Total ~ 1 + 32*3 + 4 = 101 ciclos