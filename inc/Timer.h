#ifndef TIMER_H
#define TIMER_H

#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
#include <timers.h>

// #include "hardware/rtc.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

#include "queue.h"
#include "led.h"
#include "semphr.h"
#include <semphr.h>

#include "pico/stdio.h"
#include "pico/sync.h"
#include "hardware/gpio.h"

#define SEC_timer (1000) 
#define MIN_timer (60 * SEC_timer) 
#define HORA_MS (60 * MIN_timer) // 1 hora em milissegundos
#define MEIA_HORA_MS HORA_MS/2
#define Cliclo_diurno 12 // 12 horas de sono

// Estrutura para controle do sistema
typedef struct {
    TimerHandle_t timer; // Timer para checagem periódica
    TaskHandle_t task_handle; // Handle da tarefa de sensores
    SemaphoreHandle_t semaphore; // Semáforo para sincronização
    uint16_t ultima_humidade; // Última leitura de umidade
    uint16_t ultima_exposicao; // Última leitura de exposição solar
    uint8_t ultima_temperatura; // Última leitura de temperatura (reservado para futuras expansões)
} Sistema_t;

extern Sistema_t sistema;

void timer_callback(TimerHandle_t xTimer);
void sleep_timer_init();

#endif