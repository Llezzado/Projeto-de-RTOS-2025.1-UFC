#include "wifi.h"

const char *HTML_CONTENT = 
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html\r\n"
"Connection: close\r\n"
"\r\n"
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"    <title>Estufa Automatizada</title>\n"
"    <style>\n"
"        body { font-family: Arial; text-align: center; }\n"
"        h1 { color: #118911ff; }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <h1>Estufa Automatizada</h1>\n"
"    <h2>Exposição solar diaria: /6horas</h2>\n"
"    <h2>Umidade: 60%</h2>\n"
"</body>\n"
"</html>\r\n";

void esp8266_uart_init(void) {
    uart_init(ESP8266_UART_ID, ESP8266_BAUDRATE);
    gpio_set_function(ESP8266_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(ESP8266_RX_PIN, GPIO_FUNC_UART);
    printf("ESP8266 UART initialized at %d baud\n", ESP8266_BAUDRATE);
}

void esp8266_send_cmd(const char *cmd) {
    printf("Sending command: %s\n", cmd);
    uart_puts(ESP8266_UART_ID, cmd);
    uart_puts(ESP8266_UART_ID, "\r\n");
}

void esp8266_read_response(char *buffer, size_t maxlen) {
    size_t idx = 0;
    absolute_time_t timeout = make_timeout_time_ms(5000); // 5 segundos
    buffer[0] = '\0';

    while (idx < maxlen - 1 && !time_reached(timeout)) {
        if (uart_is_readable(ESP8266_UART_ID)) {
            char c = uart_getc(ESP8266_UART_ID);
            buffer[idx++] = c;
            buffer[idx] = '\0';
            // Verifica se chegou "OK" ou "ERROR"
            if (strstr(buffer, "OK") || strstr(buffer, "ERROR")) {
                printf("Resposta: %s\n", buffer);
                break;
            }
        }
    }
    if (idx == maxlen - 1) {
        buffer[maxlen - 1] = '\0'; // Garante que a string esteja terminada
    }
}

void esp8266_uart_define(void) {
    char resp[356];

    // 1. Reinicia o ESP8266
    esp8266_send_cmd("AT+RST");
    esp8266_read_response(resp, sizeof(resp));
    
    // 2. Configura modo AP
    esp8266_send_cmd("AT+CWMODE=2");
    esp8266_read_response(resp, sizeof(resp));

    esp8266_send_cmd("AT+CWMODE?");
    esp8266_read_response(resp, sizeof(resp));

    // 3. Configura o SSID e senha do AP usando as macros
    char ap_config[128];
    sprintf(ap_config, "AT+CWSAP=\"%s\",\"%s\",1,2", SSID, PSWD);
    esp8266_send_cmd(ap_config);
    esp8266_read_response(resp, sizeof(resp));

    esp8266_send_cmd("AT+CWSAP?");
    esp8266_read_response(resp, sizeof(resp));

    // 4. Inicia servidor TCP na porta 80
    esp8266_send_cmd("AT+CIPMUX=1");
    sleep_ms(1000);
    esp8266_read_response(resp, sizeof(resp));

    esp8266_send_cmd("AT+CIPSERVER=1,80");
    sleep_ms(1000);
    esp8266_read_response(resp, sizeof(resp));
    
    printf("AP iniciado! SSID: %s, senha: %s\n", SSID, PSWD);
}

void esp8266_ap_webserver_task(void *pvParameters) {
    char resp[256];

    // 5. Loop para responder conexões
    while (1) {
        
        esp8266_read_response(resp, sizeof(resp));
        // Procura por "+IPD" indicando nova conexão
        char *ipd = strstr(resp, "+IPD,");

        if (ipd) {
            printf("Nova conexão recebida: %s\n", ipd);
            // Extrai o canal de conexão
            int ch = 0;
            sscanf(ipd, "+IPD,%d,", &ch);

            // Monta resposta HTTP
            const char *http_response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html><body><h1>Hello world</h1><h2>Finalmete Funciona!!!!</h2></body></html>\r\n";

            char cmd[64];
            sprintf(cmd, "AT+CIPSEND=%d,%d", ch, (int)strlen(http_response));
            esp8266_send_cmd(cmd);
            vTaskDelay(pdMS_TO_TICKS(500));
            esp8266_read_response(resp, sizeof(resp));
            printf("Resposta: %s\n", resp);

            uart_puts(ESP8266_UART_ID, http_response);
            vTaskDelay(pdMS_TO_TICKS(500));

            sprintf(cmd, "AT+CIPCLOSE=%d", ch);
            esp8266_send_cmd(cmd);
            vTaskDelay(pdMS_TO_TICKS(200));
            esp8266_read_response(resp, sizeof(resp));
            printf("Resposta: %s\n", resp);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void wifi_init_task(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(2000)); // Aguarda inicialização do sistema
    printf("Iniciando tarefa de WiFi...\n");
    esp8266_uart_init();
    esp8266_uart_define();
    vTaskDelete(NULL); // Task termina após inicialização
}
