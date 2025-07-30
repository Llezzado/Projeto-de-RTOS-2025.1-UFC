#include "oled.h"

// Envia comando para o OLED
void oled_send_command(uint8_t cmd) {
    uint8_t data[2] = {0x00, cmd};
    i2c_write(OLED_ADDR, data, 2);
}

// Envia dados para o OLED
void oled_send_data(uint8_t *data, size_t len) {
    uint8_t buf[OLED_WIDTH + 1];
    buf[0] = 0x40; // 0x40 indica dados
    for (size_t i = 0; i < len; i++) buf[i+1] = data[i];
    i2c_write(OLED_ADDR, buf, len + 1);
}

void uint8_to_string(uint8_t value, char* buffer) {
    char* p = buffer;
    uint8_t digits = 0;
    uint8_t temp = value;

    // Conta os dígitos
    do {
        digits++;
        temp /= 10;
    } while(temp);

    // Posiciona o terminador nulo
    p += digits;
    *p = '\0';

    // Converte cada dígito (começando pelo último)
    do {
        *--p = '0' + (value % 10);
        value /= 10;
    } while(value);
}

// Inicializa o OLED
void oled_init() {

    i2c_init_custom(); // Inicializa o I2C

    oled_send_command(0xAE); // Display OFF
    oled_send_command(0xD5); // Set display clock div
    oled_send_command(0x80);
    oled_send_command(0xA8); // Set multiplex
    oled_send_command(0x3F);
    oled_send_command(0xD3); // Set display offset
    oled_send_command(0x00);
    oled_send_command(0x40); // Set start line
    oled_send_command(0x8D); // Charge pump
    oled_send_command(0x14);
    oled_send_command(0x20); // Memory mode
    oled_send_command(0x00);
    oled_send_command(0xA1); // Seg remap
    oled_send_command(0xC8); // COM scan dec
    oled_send_command(0xDA); // Set compins
    oled_send_command(0x12);
    oled_send_command(0x81); // Set contrast
    oled_send_command(0xCF);
    oled_send_command(0xD9); // Set precharge
    oled_send_command(0xF1);
    oled_send_command(0xDB); // Set vcom detect
    oled_send_command(0x40);
    oled_send_command(0xA4); // Resume RAM content
    oled_send_command(0xA6); // Normal display
    oled_send_command(0xAF); // Display ON
}

void oled_fill_screen(int fill) {

    for (uint8_t page = 0; page < 8; page++) {
        oled_send_command(0xB0 + page); // Set page address
        oled_send_command(0x00);        // Set lower column
        oled_send_command(0x10);        // Set higher column
        uint8_t line[OLED_WIDTH];
        for (int i = 0; i < OLED_WIDTH; i++) line[i] = fill; // Todos pixels ON
        oled_send_data(line, OLED_WIDTH);
    }
}

// Escreve uma string na posição (x, page) usando font5x7
void oled_write_string(uint8_t x, uint8_t page, const char *str) {
    oled_send_command(0xB0 + page); // Seleciona a página (linha de 8px)
    oled_send_command(0x00 + (x & 0x0F)); // Coluna baixa
    oled_send_command(0x10 + ((x >> 4) & 0x0F)); // Coluna alta

    while (*str) {
        uint8_t c = *str;
        if (c < 32 || c > 126) c = 32; // Caracteres fora do range: espaço
        oled_send_data((uint8_t*)font5x7[c - 32], 5); // 5 bytes do caractere
        uint8_t space = 0x00;
        oled_send_data(&space, 1); // Espaço entre caracteres
        str++;
    }
}

void print_oled_stats(uint8_t humidade_percent, uint8_t exposicao_percent, uint16_t exposicao_solar_ideal,uint8_t temperatura,const char* nome) {
        // Preparar strings para exibição no OLED
        char str_hum[4];  // 3 dígitos + null terminator
        char str_sol[4];
        char str_tp[4];
        char str_sol_max[8];
        
        uint8_to_string(humidade_percent, str_hum);
        uint8_to_string(exposicao_percent, str_sol);       
        uint8_to_string(temperatura, str_tp);       
        uint8_to_string(exposicao_solar_ideal, str_sol_max);       

        // printf("\n--- LEITURA PERIÓDICA ---\n");
        // printf("Cultivo: %s\n", nome);
        // printf("Umidade: %d%%\n", humidade_percent);
        // printf("Temperatura: %dC\n", temperatura);
        // printf("Exposição Solar: %d%%\n", exposicao_percent);

        // Montar strings completas
        char display_line0[20];
        char display_line1[20];
        char display_line2[20];
        char display_line3[20];

        snprintf(display_line0, sizeof(display_line1), "Cultivo: %s",nome);
        snprintf(display_line1, sizeof(display_line1), "Umidade: %s%%   ", str_hum);
        snprintf(display_line2, sizeof(display_line2), "Temperatura: %sC   ", str_tp);
        snprintf(display_line3, sizeof(display_line3), "Exposicao Solar:%s%%   ", str_sol,str_sol_max);

        oled_write_string(0, 0,display_line0);
        oled_write_string(0, 2,display_line1);
        oled_write_string(0, 4,display_line2);
        oled_write_string(0, 6,display_line3);
}

void oled_power_off(void) {
    oled_send_command(0xAE); // Display OFF (SSD1306 command)
}

void oled_power_on(void) {
    oled_send_command(0xAF); // Display ON (SSD1306 command)
}