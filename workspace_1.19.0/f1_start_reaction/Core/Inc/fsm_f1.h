#ifndef INC_FSM_F1_H_
#define INC_FSM_F1_H_

#include "main.h" //  para reconocer los pines definidos en el STM32

// Definición de los estados del juego F1
typedef enum {
    ESTADO_IDLE,         // Esperando Start
    ESTADO_SECUENCIA,    // Encendido de luces rojas 1-5
    ESTADO_ESPERA_AZAR,  // Las 5 luces encendidas
    ESTADO_REACCION,     // luz apagada y cuenta el tiemop
    ESTADO_GANADOR,      // muestra ganador
    ESTADO_SALIDA_NULA,   // marca si alguien se ha adelantado
	ESTADO_JUEGO_TERMINADO
} f1_state_t;

// Funciones que llamaremos desde el main.c
void iniciar_variables_juego(void); //para darle valor inicial a las variables
void actualizar_logica_juego(void); // Esta ira dentro del while(1) y se repetira por lo tanto
#endif
