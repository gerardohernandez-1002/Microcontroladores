;
; Actividad2.asm
;
; Created: 30/09/2025 
; Author : gerco
;

.NOLIST
.INCLUDE "m328pdef.inc" ;
.LIST

;================================================================
; Definiciones y constantes
;================================================================
.DEF temp_val  = R16 ; Registro temporal para valores
.DEF counter   = R17 ; Contador general para los bucles
.DEF tens_dig  = R18 ; Registro para almacenar el dígito de las decenas
.DEF units_dig = R16 ; El registro temp_val se reutiliza para las unidades

.EQU N_VAL = 15 ; Valor de n (debe ser <= 99). 

.EQU SRAM_START_RAW = 0x0120 ; Dirección de inicio para números en binario.
.EQU SRAM_START_BCD = 0x0200 ; Dirección de inicio para números en BCD.

;================================================================
; Código
;================================================================
.CSEG
.ORG 0x0000
    RJMP main ; Salto al programa principal al resetear

;================================================================
; Programa Principal
;================================================================
main:
    ; Inicialización del Stack Pointer  
    LDI temp_val, HIGH(RAMEND)
    OUT SPH, temp_val
    LDI temp_val, LOW(RAMEND)
    OUT SPL, temp_val


;Generar y almacenar N números en formato binario/hexadecimal

    ; Cargar el puntero Y con la dirección de inicio para los datos brutos
    LDI YH, HIGH(SRAM_START_RAW)
    LDI YL, LOW(SRAM_START_RAW)

    ; Inicializar el valor a almacenar (empezando desde N_VAL)
    LDI temp_val, N_VAL

    ; Inicializar el contador del bucle (N+1 elementos, de N a 0)
    LDI counter, N_VAL + 1

store_loop:
    ST Y+, temp_val     ; Almacena el valor en SRAM y post-incrementa el puntero Y
    DEC temp_val        ; Decrementa el valor a almacenar (N, N-1, N-2, ...)
    DEC counter         ; Decrementa el contador del bucle
    BRNE store_loop     ; Si el contador no es cero, repetir el bucle


;Leer, convertir a BCD y almacenar en una nueva ubicación

    ;Recargar el puntero Y para leer desde el inicio de los datos brutos
    LDI YH, HIGH(SRAM_START_RAW)
    LDI YL, LOW(SRAM_START_RAW)

    ;Cargar el puntero Z con la dirección de destino para los datos BCD
    LDI ZH, HIGH(SRAM_START_BCD)
    LDI ZL, LOW(SRAM_START_BCD)

    ;Reiniciar el contador del bucle para procesar N+1 elementos
    LDI counter, N_VAL + 1

convert_loop:
    LD temp_val, Y+     ; Carga un número desde la SRAM (apuntada por Y) y post-incrementa Y
    RCALL bin2bcd       ; Llama a la subrutina de conversión a BCD
    ST Z+, temp_val     ; Almacena el resultado BCD en la SRAM (apuntada por Z) y post-incrementa Z
    DEC counter         ; Decrementa el contador del bucle
    BRNE convert_loop   ; Si el contador no es cero, repetir el bucle

;Fin del programa, bucle infinito para detener la ejecución 
done:
    RJMP done


; SUBRUTINA: Conversión de Binario a BCD (para números de 0 a 99)
; Entrada: R16 (temp_val) con el número binario
; Salida:  R16 (temp_val) con el número en formato BCD

bin2bcd:
    ; Algoritmo de resta sucesiva para encontrar las decenas
    LDI tens_dig, 0     ; Inicia el contador de decenas en 0

bcd_tens_loop:
    CPI temp_val, 10    ; Compara el número con 10
    BRLO bcd_done       ; Si es menor (BRanch if LOwer), ya no se puede restar 10. Fin.
    SUBI temp_val, 10   ; Resta 10 al número. El residuo serán las unidades.
    INC tens_dig        ; Incrementa el contador de decenas.
    RJMP bcd_tens_loop  ; Repite el proceso.

bcd_done:

    ; 'tens_dig' contiene el dígito de las decenas 
    ; 'temp_val' contiene el dígito de las unidades
    ; Ahora combinamos los dos dígitos en un solo byte BCD.
    ; Formato BCD: [Dígito Decenas | Dígito Unidades]
   

    SWAP tens_dig       ; Mueve el nibble bajo de tens_dig al nibble alto.
                        ; Si tens_dig = 0x01, se convierte en 0x10.
    OR temp_val, tens_dig ; Combina las decenas (en nibble alto) con las unidades (en nibble bajo)

    RET                 ; Retorna de la subrutina. El resultado está en temp_val (R16).
