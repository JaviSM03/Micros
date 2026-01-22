#include "lcd_i2c.h"

extern I2C_HandleTypeDef hi2c1;  // cambia a hi2c2 si usas el I2C2

#define SLAVE_ADDRESS_LCD 0x4E // cambia a 0x27 si no funciona con 0x4E

void lcd_send_cmd (char comando)
{
  char data_u, data_l;
  uint8_t data_t[4];
  // separamos el byte en dos mitades (Nibbles)
    data_u = (comando & 0xf0);      //cogemos los 4 bits de arriba
    data_l = ((comando << 4) & 0xf0); // movemos los 4 de abajo arriba
  data_t[0] = data_u|0x0C;  //alta con enable a 1
  data_t[1] = data_u|0x08;  //alta con enable a 0 : flanco de bajada para leer
  data_t[2] = data_l|0x0C;  // igual que enable a 1
  data_t[3] = data_l|0x08;  //igual que enable a 0
  //el bit rs siempre a 0 porque no es caracter lo que estamos viendo sino un comando
  HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char letra)
{
  char data_u, data_l;
  uint8_t data_t[4];
  data_u = (letra&0xf0); //aqui igual que en el data arriba dividimos en dos mitades
  data_l = ((letra<<4)&0xf0);
  data_t[0] = data_u|0x0D;  //funcoina igual que el comando arriba
  data_t[1] = data_u|0x09;
  data_t[2] = data_l|0x0D;
  data_t[3] = data_l|0x09;
  //aqui el rs a 1 porque es caracter lo que estamos viendo
  HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_init (void)
{
	// Se envían comandos repetidos para asegurar que el chip se despierta bien.
	// recomendacion del proveedor
  lcd_send_cmd (0x33);
  lcd_send_cmd (0x32);
  HAL_Delay(50);
  lcd_send_cmd (0x28);
  HAL_Delay(50);
  lcd_send_cmd (0x01);
  HAL_Delay(50);
  lcd_send_cmd (0x06);
  HAL_Delay(50);
  lcd_send_cmd (0x0C);
  HAL_Delay(50);
}

void lcd_send_string (char *texto)
{
	// mientras queden letras en la frase va viendo
	    while (*texto)
	    {
	        // envia la letra actual y pasa a la siguiente posición
	        lcd_send_data (*texto++);
	    }
}

void lcd_put_cur(int fila, int col)
{
    switch (fila)
    { //comandos para el posicionamiento del cursor
        case 0:
            col |= 0x80;
            break;
        case 1:
            col |= 0xC0;
            break;
    }
    lcd_send_cmd (col);
}

void lcd_clear (void)
{
    lcd_send_cmd(0x01); //eoliminamos la pantalla, si o si hay que poner un delay porque tiene un minimo retardo
    HAL_Delay(2);
}
