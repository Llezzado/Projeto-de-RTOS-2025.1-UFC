/*  

    Universidade Federal do Ceará - UFC
    Disciplina: Sistemas de tempo real - 2025.1
    Professor: FRANCISCO HELDER CANDIDO DOS SANTOS FILHO
    
    Alunos: JUAN PABLO RUFINO MESQUITA - 509982
            MONALISA SILVA BEZERRA - 535614
    
    email:  juanrufinmesq@alu.ufc.br 
            monasilva@alu.ufc.br

    Projeto: Sistema de Monitoramento e Controle de Cultivo de Alface
    Descrição: Este código implementa um sistema de monitoramento e controle para cultivo utilizando sensores de umidade e exposição solar.

*/

#include <FreeRTOS.h>
#include <task.h>
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "led.h"
#include "oled.h"
#include "servo_motor.h"
#include "Timer.h"
#include "dht11.h"
#include "wifi.h"
#include "cultivo.h"


#define ADC_Solo_Humidade 26
#define ADC_Exposicao_Solar 27
#define ADC_MAX_VALUE 4095
#define PERCENT_REF 100
#define LOW_PERCENTAGE 0.75
#define HIGH_PERCENTAGE 1.5

DataCultivo_t data_global;
SemaphoreHandle_t data_mutex;

void sensor_task(void *pvParameters);
void printout_task(void *pvParameters);
void tratamento_task(void *pvParameters);

Cultivo_t cultivo = {65, 50, 25, 6, "Alface", true};

int main() {
    BlinkParams_t led0 = {LED_0, NULL, LED_Sample_Rate, "LED 0"};

    data_mutex = xSemaphoreCreateMutex();
    configASSERT(data_mutex != NULL);

    stdio_init_all();

    adc_init();
    adc_gpio_init(ADC_Solo_Humidade);
    adc_gpio_init(ADC_Exposicao_Solar);
    servo_init(SERVO_PIN);
    
    oled_init();
    oled_fill_screen(OLED_CLS);

    esp8266_uart_init();
    esp8266_uart_define();

    sistema.semaphore = xSemaphoreCreateBinary();
    configASSERT(sistema.semaphore != NULL);

    sistema.timer = xTimerCreate("TimerChecagem", pdMS_TO_TICKS(SEC_timer*5), pdTRUE, NULL, timer_callback);
    xTimerStart(sistema.timer, 0);

    xTaskCreate(led_task, "LED_Task", 8, &led0, 1, NULL);
    xTaskCreate(sensor_task, "Sensor_Task", 256, &cultivo, 2, &sistema.task_handle);
    xTaskCreate(tratamento_task, "Servo_Task", 226, &cultivo, 3, NULL);
    xTaskCreate(printout_task, "Print_data_Task", 2048, &cultivo, 1, NULL);

    vTaskStartScheduler();
    for(;;);
}

void printout_task(void *pvParameters) {

    Cultivo_t cultivo = *(Cultivo_t *)pvParameters;
    DataCultivo_t data;

    printf("Iniciando tarefa de impressão de dados...\n");
    oled_power_on();
    while (1) {
        
        esp8266_wifi_server_handshake(data_mutex);

        if (xSemaphoreTake(data_mutex, portMAX_DELAY)) {
            data = data_global;
            print_oled_stats(data.humidade, data.exposicao, cultivo.exposicao_solar_ideal, data.temperatura, cultivo.nome);

            xSemaphoreGive(data_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(SEC_timer*5));
    }
}

void tratamento_task(void *pvParameters) {

    Cultivo_t cultivo = *(Cultivo_t *)pvParameters;
    DataCultivo_t data;
    uint8_t angulo_set = 0;
    uint8_t horas_restantes = cultivo.horas_registradas * 2;

    servo_move_to_angle(SERVO_PIN, 0, 50);

    while (1) {

        if (xSemaphoreTake(data_mutex, portMAX_DELAY)) {
            data = data_global;
            xSemaphoreGive(data_mutex);

            // Umidade
            if (data.humidade < cultivo.humidade_ideal)
                printf("! ATIVANDO IRRIGAÇÃO !\n");
            if (data.humidade > (cultivo.humidade_ideal * HIGH_PERCENTAGE))
                printf("! IRRIGAÇÃO SOBRE RISCO EXCESSO DE AGUA !\n");

            // Temperatura
            if (data.temperatura > cultivo.temperatura_ideal * HIGH_PERCENTAGE) {
                printf("! TEMPERATURA ACIMA DO IDEAL !\n");
                angulo_set = 45;
            } else if (data.temperatura < cultivo.temperatura_ideal * LOW_PERCENTAGE) {
                printf("! TEMPERATURA ABAIXO DO IDEAL !\n");
                angulo_set = 75;
            }

            // Exposição solar
            if ((horas_restantes > 0) && cultivo.daytime) {
                if (data.exposicao < cultivo.exposicao_solar_ideal * LOW_PERCENTAGE) {
                    printf("! ATIVANDO MOTOR SERVO !\n");
                    angulo_set = 90;
                } else if (data.exposicao > (cultivo.exposicao_solar_ideal * HIGH_PERCENTAGE)) {
                    printf("! MOTOR SERVO DESATIVADO !\n");
                    angulo_set = 0;
                } else {
                    horas_restantes--;
                    printf("! MOTOR SERVO MANTIDO NA POSIÇÃO !\n");
                }
            } else {
                printf("Ciclo solar concluido\n");
                cultivo.daytime = false;
                oled_power_off();
                close_servo(SERVO_PIN);
                sleep_timer_init();
                horas_restantes = cultivo.horas_registradas;
                oled_power_on();
                cultivo.daytime = true;
                continue;
            }

            servo_move_to_angle(SERVO_PIN, angulo_set, 50);
            printf("horas restantes: %d de %d \n", horas_restantes/2, cultivo.horas_registradas);
            vTaskDelay(pdMS_TO_TICKS(SEC_timer*5));
        }
    }
}

void sensor_task(void *pvParameters) {
    
    Cultivo_t cultivo = *(Cultivo_t *)pvParameters;
    DataCultivo_t data;
    uint8_t ambiente_humidade;


    while (1) {
        xSemaphoreTake(sistema.semaphore, portMAX_DELAY);
        if (!cultivo.daytime) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }
        xSemaphoreTake(data_mutex, portMAX_DELAY);

        adc_select_input(0);
        sistema.ultima_humidade = adc_read();
        adc_select_input(1);
        sistema.ultima_exposicao = adc_read();
        dht11_read(DHT_PIN, &sistema.ultima_temperatura, &ambiente_humidade);

        data.humidade = PERCENT_REF - (sistema.ultima_humidade * PERCENT_REF) / ADC_MAX_VALUE;
        data.exposicao = (sistema.ultima_exposicao * PERCENT_REF) / ADC_MAX_VALUE;
        data.temperatura = sistema.ultima_temperatura;
        data.horas = cultivo.horas_registradas;

        data_global = data;
        xSemaphoreGive(data_mutex);
    }
}