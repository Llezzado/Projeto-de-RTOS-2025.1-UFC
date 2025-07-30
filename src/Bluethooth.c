#include "Bluethooth.h"

void Bluethooth_Setup() {
    // Configura os pinos do Bluetooth
    gpio_set_function(BLUETHOOTH_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(BLUETHOOTH_RX_PIN, GPIO_FUNC_UART);

    // Inicializa o UART
    uart_init(UART_ID, BAUD_RATE);
    
    // Configura os pinos do UART
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);
    
    // Configura o buffer de transmissão e recepção
    uart_set_fifo_enabled(uart1, true);
    
    
}

void bluetooth_send(const char *msg) {
    uart_puts(UART_ID, msg);
}

void Task_Bluetooth(void *pvParameters) {
    const char *message = "123456789\n";
    
    for(;;) {
        bluetooth_send(message);

        vTaskDelay(pdMS_TO_TICKS(2000)); // Envia a mensagem a cada segundo
    }
}

void Task_Bluetooth_Receive(void *pvParameters) {
    char buffer[64];
    int idx = 0;

    for (;;) {
        // Lê caracteres enquanto houver dados na UART
        while (uart_is_readable(UART_ID)) {
            char c = uart_getc(UART_ID);

            // Armazena no buffer até '\n' ou buffer cheio
            if (c == '\n' || idx >= (int)(sizeof(buffer) - 1)) {
                buffer[idx] = '\0';
                printf("Recebido via Bluetooth: %s\n", buffer);
                idx = 0; // Reinicia o buffer
            } else if (c != '\r') {
                buffer[idx++] = c;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Pequeno delay para não travar a CPU
    }
}

