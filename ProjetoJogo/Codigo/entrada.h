#ifndef ENTRADA_H
#define ENTRADA_H

#include <stdbool.h>

typedef struct {
    float moverX;
    float moverY;
    bool atacar;
    bool esquivar;
    bool pocao;
    bool pausar;
    bool cima;
    bool baixo;
    bool confirmar;
    bool voltar;
} Entrada;

Entrada LerEntrada(void);

#endif
