#ifndef LCD_I2C_H_
#define LCD_I2C_H_
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

// --- CONFIGURACIÓN DE DIRECCIÓN ---
// Usa 0x70 para PCF8574A (Tu caso en Proteus)
// Usa 0x40 para PCF8574 (Estándar)
#define LCD_ADDR 0x70

void lcd_i2c_init(void);
void lcd_i2c_cmd(uint8_t cmd);
void lcd_i2c_write_data(uint8_t data);
void lcd_i2c_print(char* str);
void lcd_i2c_set_cursor(uint8_t col, uint8_t row);
void lcd_i2c_clr(void);

#endif