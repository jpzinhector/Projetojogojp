#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "player.h"

void DesenharUI(int pontos, float tempoDecorrido);

void DesenharPainelJogador(const Player *player, int onda, int totalOndas, int orcsRestantes);

void DesenharAvisoOnda(int proximaOnda);

void DesenharTelaFinal(bool venceu, int pontos, float tempoDecorrido);

#endif
