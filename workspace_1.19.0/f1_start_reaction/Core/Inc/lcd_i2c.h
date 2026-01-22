#ifndef LCD_I2C_H_
#define LCD_I2C_H_

#include "stm32f4xx_hal.h"

// Funciones para controlar la pantalla LCD
void lcd_init (void);                   // Configuracion inicial obligatoria
void lcd_send_cmd (char comando);       // Enviar ordenes ya sea borrar, mover...
void lcd_send_data (char letra);        // Enviar un caracter suelto
void lcd_send_string (char *texto);     // Escribir una frase completa
void lcd_put_cur(int fila, int col);    // Colocar el cursor (Fila 0-1, Columna 0-15)
void lcd_clear (void);                  // Limpiar pantalla

#endif /* LCD_I2C_H_ */
