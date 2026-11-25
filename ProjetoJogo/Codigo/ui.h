#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include "raylib.h"
#include "player.h"
#include "inimigo.h"
#include "pilha_pocoes.h"

void DesenharUI(int pontos, float tempoDecorrido);
void DesenharPainelJogador(const Player *player, int onda, int totalOndas, int inimigosRestantes);
void DesenharPainelEquipamento(const Player *player, const PilhaPocoes *inventario, Texture2D pocoes);
void DesenharBarraChefe(const Inimigo *chefe);
void DesenharAvisoOnda(int proximaOnda, bool comChefe);
void DesenharTelaFinal(bool venceu, int pontos, float tempoDecorrido);

void DesenharEscurecer(int alfa);
void DesenharMenuOpcoes(const char *titulo, const char *subtitulo, const char *const *opcoes, int quantidade, int selecionada, int y);
void DesenharTelaControles(void);
void DesenharTituloJogo(int y);

#endif
