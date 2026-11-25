#ifndef TELAS_H
#define TELAS_H

#include <stdbool.h>
#include "pilha_telas.h"
#include "partida.h"

typedef struct {
    PilhaTelas pilha;
    int opcaoMenu;
    int opcaoPausa;
    bool sair;
} Telas;

void IniciarTelas(Telas *telas);
void AtualizarTelas(Telas *telas, Partida *partida, const Entrada *entrada, float dt);
void DesenharTelas(const Telas *telas, const Partida *partida);
void FinalizarTelas(Telas *telas);

#endif
