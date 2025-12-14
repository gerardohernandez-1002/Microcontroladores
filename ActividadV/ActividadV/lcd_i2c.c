#include "i2c.h"
#include "lcd_i2c.h"

void lcd_send_byte(uint8_t val, uint8_t mode) {
	uint8_t data_high = (val & 0xF0) | mode | 0x08; // 0x08 es Backlight ON
	uint8_t data_low = ((val << 4) & 0xF0) | mode | 0x08;

	i2c_start();
	i2c_write(LCD_ADDR); // Dirección
	
	// Enviar nibble alto
	i2c_write(data_high);
	i2c_write(data_high | 0x04); // Enable High
	i2c_write(data_high & ~0x04); // Enable Low

	// Enviar nibble bajo
	i2c_write(data_low);
	i2c_write(data_low | 0x04); // Enable High
	i2c_write(data_low & ~0x04); // Enable Low
	
	i2c_stop();
}

void lcd_i2c_cmd(uint8_t cmd) {
	lcd_send_byte(cmd, 0);
}

void lcd_i2c_write_data(uint8_t data) {
	lcd_send_byte(data, 1);
}

void lcd_i2c_init(void) {
	_delay_ms(50);
	lcd_i2c_cmd(0x02); // Retorno a casa (modo 4 bits)
	lcd_i2c_cmd(0x28); // 4 bits, 2 líneas, 5x7
	lcd_i2c_cmd(0x0C); // Display ON, Cursor OFF
	lcd_i2c_cmd(0x06); // Incremento cursor
	lcd_i2c_cmd(0x01); // Limpiar pantalla
	_delay_ms(2);
}

void lcd_i2c_print(char* str) {
	while (*str) lcd_i2c_write_data(*str++);
}

void lcd_i2c_set_cursor(uint8_t col, uint8_t row) {
	uint8_t pos = (row == 1) ? 0x80 : 0xC0;
	lcd_i2c_cmd(pos + col - 1);
}

void lcd_i2c_clr(void) {
	lcd_i2c_cmd(0x01);
	_delay_ms(2);
}