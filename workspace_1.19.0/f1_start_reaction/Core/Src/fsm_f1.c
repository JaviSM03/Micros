#include "main.h" //la usamos para llamar a los pines
#include "stm32f4xx_hal.h" //la basica
#include "stdio.h" //para escribir en la pantalla
#include "fsm_f1.h"
#include "lcd_i2c.h" //para control pantalla


#define PUNTOS_PARA_GANAR 3

//son como las variables  privadas en informatica, solo se pueden leer desde este .c
static f1_state_t estado_actual = ESTADO_IDLE;
static uint8_t ganador = 0;

static uint32_t tick_referencia = 0;
static uint32_t tiempo_espera_azar = 0;
static uint8_t orden = 0;

static uint8_t nula_init = 0;
static uint32_t tiempo_entrada_estado = 0;

// Control para inicializar el LCD solo una vez
static uint8_t sistema_ya_iniciado = 0;

// inicio marcador
static int puntosJ1 = 0;
static int puntosJ2 = 0;

// gestion de la parte visual
static uint8_t pantalla_actualizada = 0;
static uint32_t ultimo_tiempo_reaccion = 0;

//  buzzer
static uint32_t tiempo_inicio_buzzer = 0;
static uint8_t buzzer_activo = 0;
static uint32_t duracion_buzzer = 0;

extern ADC_HandleTypeDef hadc1; //llamamos a su uso a ADC1 y TIM3 del main
extern TIM_HandleTypeDef htim3;

//funciones auxiliares

//Funcion que enciende ruido y guarda cuando empieza y cuanto debe durar
void F1_Sonar_Buzzer(uint16_t tono, uint32_t duracion) {
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, tono); //pwm para gestionar el tono
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    tiempo_inicio_buzzer = HAL_GetTick();
    duracion_buzzer = duracion;
    buzzer_activo = 1;
}

void F1_Mostrar_Marcador(void) {
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("MARCADOR ACTUAL:"); //fila de arriba

    char marcador_str[16];
    sprintf(marcador_str, "J1:%d    J2:%d", puntosJ1, puntosJ2);
    lcd_put_cur(1, 0); //Fila de abajo
    lcd_send_string(marcador_str);
}

//inicializacion para empezar
void iniciar_variables_juego(void) {
    estado_actual = ESTADO_IDLE;
    ganador  = 0;
    pantalla_actualizada = 0;
    nula_init = 0;

    // apagar todos los LEDs
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);

    // Apagar LEDs de los ganadores uno por uno
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

    // apagar Buzzer
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
    buzzer_activo = 0;

    // solo se inicia la lcd la primera vez porque es lento
    if (sistema_ya_iniciado == 0) {
        lcd_init();
        sistema_ya_iniciado = 1;
    }

    // Limpiamos pantalla y mostramos mensaje
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string(" F1 START GAME ");
    lcd_put_cur(1, 0);

    if (puntosJ1 > 0 || puntosJ2 > 0) {
        char msg[16];
        sprintf(msg, "Score: %d - %d", puntosJ1, puntosJ2);
        lcd_send_string(msg);
    } else {
        lcd_send_string("Pulse START...");
    }
}

//el bucle prinicpal
void actualizar_logica_juego(void) {

    // gestion del Buzzer usamos el tiempo de antes para comparar cuando apagar
    if (buzzer_activo && (HAL_GetTick() - tiempo_inicio_buzzer >= duracion_buzzer)) {
        HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
        buzzer_activo = 0;
    }

    switch (estado_actual) {
//gestion fsm
        case ESTADO_IDLE:

            // Solo leemos el ADC cuando pulsamos el botón.

            // Leer btn START (PA0)
            if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {

                // delay para evitar rebotes
                HAL_Delay(300);

                // Calculamos el "azar" con tecnica Start-Poll-Get de teoria
                HAL_ADC_Start(&hadc1);
                if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
                    tiempo_espera_azar = 2000 + HAL_ADC_GetValue(&hadc1);
                }
                HAL_ADC_Stop(&hadc1);

                estado_actual = ESTADO_SECUENCIA;
                orden = 0; //por si acaso para empezar desde el led 0
                tick_referencia = HAL_GetTick(); //comienza el tiemop para el primer led

                lcd_clear();//limpiamos y escribimos
                lcd_send_string("PILOTOS ATENTOS...");
            }
            break;

        case ESTADO_SECUENCIA:
            if (HAL_GetTick() - tick_referencia >= 300) {
                tick_referencia = HAL_GetTick();
                orden++;
//cada vez q pase nuestro tiempo dispuesto, sube orden y pasa a la siguiente LED
                switch(orden) {
                    case 1:
                    	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
                    	F1_Sonar_Buzzer(1000, 200);
                    	break;
                    case 2:
                    	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
                    	F1_Sonar_Buzzer(1000, 200);
                    	break;
                    case 3:
                    	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
                    	F1_Sonar_Buzzer(1000, 200);
                    	break;
                    case 4:
                    	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
                    	F1_Sonar_Buzzer(1000, 200);
                    	break;
                    case 5:
                        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);
                        F1_Sonar_Buzzer(900, 800);//cambiamos tono para que llame la atencion
                        estado_actual = ESTADO_ESPERA_AZAR;
                        tick_referencia = HAL_GetTick();
                        break;
                }
            }
            break;

        case ESTADO_ESPERA_AZAR:
            if (HAL_GetTick() - tick_referencia >= tiempo_espera_azar) {
            	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
            	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);            F1_Sonar_Buzzer(1000, 600);

                estado_actual = ESTADO_REACCION;
                tick_referencia = HAL_GetTick(); //al apagar los leds comienza el tiempo
            }									// para ver cual es la reaccion mas rapida
            break;

        case ESTADO_REACCION: //comprobamos quien pulsa en interrupcion
            if (HAL_GetTick() - tick_referencia > 5000) { //si pasan 5 segs nos vamos al ppio del tiron
                estado_actual = ESTADO_IDLE;
                iniciar_variables_juego();
            }
            break;

        case ESTADO_GANADOR: //secuencial distintas etapas
            // etapa1. Mostrar ganador ronda
            if (pantalla_actualizada == 0) {
                if (ganador == 1) {
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
                    puntosJ1++;
                } else {
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
                    puntosJ2++;
                }
                	//Lo primero es mostrar quien gana y su tiempo
                lcd_clear();
                lcd_put_cur(0, 0);
                if (ganador == 1) lcd_send_string(" GANADOR: J1 ");
                else lcd_send_string(" GANADOR: J2 ");

                char tiempo_str[16];
                sprintf(tiempo_str, "T: %lu ms", ultimo_tiempo_reaccion);
                lcd_put_cur(1, 0);
                lcd_send_string(tiempo_str);

                pantalla_actualizada = 1;
                tick_referencia = HAL_GetTick();
            }

            // etapa2. Mostrar Marcador llamamos funcion auxiliar marcador post 3 segs
            if (pantalla_actualizada == 1 && (HAL_GetTick() - tick_referencia > 3000)) {
                F1_Mostrar_Marcador();
                pantalla_actualizada = 2;
                tick_referencia = HAL_GetTick();
            }

            // etapa3. Decisión finaal : (Fin o Siguiente Ronda) post 3 segs
            if (pantalla_actualizada == 2 && (HAL_GetTick() - tick_referencia > 3000)) {
                if (puntosJ1 >= PUNTOS_PARA_GANAR || puntosJ2 >= PUNTOS_PARA_GANAR) { //si alguno llegó a la puntuacion establecida
                    estado_actual = ESTADO_JUEGO_TERMINADO;
                    pantalla_actualizada = 0;
                }
                else {

                	iniciar_variables_juego(); // Vuelve a inicio en caso de no haberlo
                }
            }
            break;

        case ESTADO_SALIDA_NULA:
            // etapa 1. Falta
            if (nula_init == 0) {
                tiempo_entrada_estado = HAL_GetTick();
                tick_referencia = HAL_GetTick();
                F1_Sonar_Buzzer(200, 1000);

                if (ganador == 1) { //restamos un punto y si ya esta en cero le dejamos asi
                    puntosJ1--;
                    if(puntosJ1 < 0) puntosJ1 = 0;
                } else {
                    puntosJ2--;
                    if(puntosJ2 < 0) puntosJ2 = 0;
                }

                lcd_clear();
                lcd_put_cur(0,0);
                lcd_send_string(" SALIDA NULA! ");
                lcd_put_cur(1,0);
                if(ganador == 1) lcd_send_string("J1 SE ADELANTO");
                else lcd_send_string("J2 SE ADELANTO");

                nula_init = 1;
            }

            // etapa 2. mostramos de forma visual que se ha adelantado alguno
            if (nula_init == 1) {
                if (HAL_GetTick() - tick_referencia >= 200) {
                    tick_referencia = HAL_GetTick();
                    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);
                    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_1);
                    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_2);
                    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_3);
                    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_4);                }

                if (HAL_GetTick() - tiempo_entrada_estado > 3000) { //al apso de 3 segs reseteamosl leds
                	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
                	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
                	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
                	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
                	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);
                	F1_Mostrar_Marcador();
                    nula_init = 2;
                    tiempo_entrada_estado = HAL_GetTick();
                }
            }

            // etapa 3. siguiente  Ronda
            if (nula_init == 2) {
                if (HAL_GetTick() - tiempo_entrada_estado > 3000) {
                	iniciar_variables_juego();
                }
            }

            if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {
            	iniciar_variables_juego();// start otra vez
            }
            break;

        case ESTADO_JUEGO_TERMINADO:
            if (pantalla_actualizada == 0) {
                F1_Sonar_Buzzer(1500, 1500);

                lcd_clear();
                lcd_put_cur(0, 0);
                lcd_send_string("!! VICTORIA !!");
                lcd_put_cur(1, 0);

                if (puntosJ1 >= PUNTOS_PARA_GANAR) {
                    lcd_send_string("CAMPEON: J1");
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
                } else {
                    lcd_send_string("CAMPEON: J2");
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
                }

                pantalla_actualizada = 1;
                tick_referencia = HAL_GetTick();
            }

            if (HAL_GetTick() - tick_referencia > 5000) {
                puntosJ1 = 0; //reseteamos puntuacion antes de volver a iniciar
                puntosJ2 = 0;
                iniciar_variables_juego();
            }
            break;
    }
}

//interrupciones del juego
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

    if (estado_actual == ESTADO_SECUENCIA || estado_actual == ESTADO_ESPERA_AZAR || estado_actual == ESTADO_REACCION) {

        uint8_t jugador_detectado = 0;

        if (GPIO_Pin == BTN_P1_Pin) { //COMPRUEBO QUIEN PULSA
        	jugador_detectado = 1;  }

        else if (GPIO_Pin == BTN_P2_Pin)
        	{jugador_detectado = 2;
        }

        if (jugador_detectado > 0) { //cualquiera de los dos
            if (estado_actual == ESTADO_SECUENCIA || estado_actual == ESTADO_ESPERA_AZAR) {//se ha pulsado cuando no toca
                estado_actual = ESTADO_SALIDA_NULA;
                ganador = jugador_detectado; //asigno quien ha pulsado a ganador para restarle puntos
                nula_init = 0;
            }
            else {
                ultimo_tiempo_reaccion = HAL_GetTick() - tick_referencia; //para mostrar por pantalla
                estado_actual = ESTADO_GANADOR;
                ganador = jugador_detectado;
                pantalla_actualizada = 0;
            }
        }
    }
}
