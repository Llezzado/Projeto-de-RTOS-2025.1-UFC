#include "wifi.h"

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
    esp8266_read_response(resp, sizeof(resp));

    esp8266_send_cmd("AT+CIPSERVER=1,80");
    esp8266_read_response(resp, sizeof(resp));
    
    printf("AP iniciado! SSID: %s, senha: %s\n", SSID, PSWD);
}

void esp8266_wifi_server_handshake(SemaphoreHandle_t data_mutex) {
    char resp[256];
    char html_buffer[1536]; // 150% de 1024 bytes

    esp8266_read_response(resp, sizeof(resp));
    char *ipd = strstr(resp, "+IPD,");

    if (ipd) {
        printf("Nova conexão recebida: %s\n", ipd);
        int ch = 0;
        sscanf(ipd, "+IPD,%d,", &ch);

        // Protege acesso aos dados globais
        xSemaphoreTake(data_mutex, portMAX_DELAY);

        // Supondo que você tenha acesso a data_global e cultivo
        extern DataCultivo_t data_global;
        extern Cultivo_t cultivo;
        uint8_t horas_restantes = data_global.horas_restantes;

        // Monta HTML dinâmico
        snprintf(html_buffer, sizeof(html_buffer),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <title>Estufa Automatizada</title>\n"
            "    <style>\n"
            "        body {\n"
            "            font-family: Arial, sans-serif;\n"
            "            text-align: center;\n"
            "            background: linear-gradient(135deg, #e0ffe0 0%%, #b3e6ff 100%%);\n"
            "            min-height: 100vh;\n"
            "            margin: 0;\n"
            "        }\n"
            "        h1 { color: #118911ff; margin-top: 30px; }\n"
            "        .card {\n"
            "            background: #fff;\n"
            "            border-radius: 16px;\n"
            "            box-shadow: 0 4px 16px rgba(0,0,0,0.08);\n"
            "            display: inline-block;\n"
            "            padding: 24px 40px;\n"
            "            margin-top: 40px;\n"
            "        }\n"
            "        button {\n"
            "            margin-top: 24px;\n"
            "            padding: 12px 32px;\n"
            "            font-size: 1.1em;\n"
            "            border: none;\n"
            "            border-radius: 8px;\n"
            "            background: #118911ff;\n"
            "            color: #fff;\n"
            "            cursor: pointer;\n"
            "            transition: background 0.2s;\n"
            "        }\n"
            "        button:hover { background: #0c6c0c; }\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n"
            "    <h1>Estufa Automatizada</h1>\n"
            "    <div class=\"card\">\n"
            "        <h2>Cultivo: %s</h2>\n"
            "        <h2>Umidade: %d%%</h2>\n"
            "        <h2>Temperatura: %dC</h2>\n"
            "        <h2>Exposicao solar: %d%%</h2>\n"
            "        <h2>Horas restantes do ciclo: %d / %d</h2>\n"
            "        <button onclick=\"location.reload()\">Atualizar</button>\n"
            "    </div>\n"
            "</body>\n"
            "</html>\r\n",
            cultivo.nome,
            data_global.humidade,
            data_global.temperatura,
            data_global.exposicao,
            horas_restantes,
            cultivo.horas_registradas
        );

        xSemaphoreGive(data_mutex);

        // Envia o HTML dinâmico
        char cmd[64];
        sprintf(cmd, "AT+CIPSEND=%d,%d", ch, (int)strlen(html_buffer));
        esp8266_send_cmd(cmd);
        vTaskDelay(pdMS_TO_TICKS(500));
        esp8266_read_response(resp, sizeof(resp));
        printf("Resposta: %s\n", resp);

        uart_puts(ESP8266_UART_ID, html_buffer);
        vTaskDelay(pdMS_TO_TICKS(500));

        sprintf(cmd, "AT+CIPCLOSE=%d", ch);
        esp8266_send_cmd(cmd);
        vTaskDelay(pdMS_TO_TICKS(200));
        esp8266_read_response(resp, sizeof(resp));
        printf("Resposta: %s\n", resp);
    }
}

void wifi_init_task(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(2000)); // Aguarda inicialização do sistema
    printf("Iniciando tarefa de WiFi...\n");
    esp8266_uart_init();
    esp8266_uart_define();
    vTaskDelete(NULL); // Task termina após inicialização
}
