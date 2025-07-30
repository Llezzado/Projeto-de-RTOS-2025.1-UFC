#ifndef CULTIVO_H
#define CULTIVO_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t humidade_ideal;
    uint8_t exposicao_solar_ideal;
    uint8_t temperatura_ideal;
    uint8_t horas_registradas;
    const char *nome;
    bool daytime;
} Cultivo_t;

typedef struct {
    uint8_t humidade;
    uint8_t exposicao;
    uint8_t temperatura;
    uint8_t horas;
    uint8_t horas_restantes;
} DataCultivo_t;

#endif