#ifndef ARMA_H
#define ARMA_H

#include <stdbool.h>
#include "raylib.h"
#include "player.h"

typedef struct {
    Texture2D sprite;
    float     tempo;
    float     recarga;
    bool      atacando;
    bool      impactoPendente;
    bool      direita;
    float     relogio;
} Arma;

void IniciarArma(Arma *arma);
void ResetarArma(Arma *arma);
void FinalizarArma(Arma *arma);

bool IniciarGolpe(Arma *arma, bool direita);
void AtualizarArma(Arma *arma, float deltaTime);
bool ConsumirImpacto(Arma *arma);
float AvancoArma(const Arma *arma);

void DesenharArma(const Arma *arma, const Player *player);

#endif
