#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "raylib.h"
#include "arena.h"
#include "entrada.h"

#define RASTRO_MAX 8

typedef struct {
    Vector2 posicao;
    int frame;
    bool direita;
    float idade;
} RastroPlayer;

typedef struct {
    Vector2 posicao;
    Vector2 tamanho;
    Texture2D sprite;
    float velocidade;

    float larguraFrame;
    float alturaFrame;
    int   frameAtual;
    float tempoFrame;
    bool  andando;
    bool  olhandoDireita;

    int   vida;
    int   vidaMaxima;
    float tempoInvulneravel;

    float tempoEsquiva;
    float recargaEsquiva;
    Vector2 direcaoEsquiva;
    float tempoRastro;
    RastroPlayer rastro[RASTRO_MAX];

    float tempoCura;
    int   ultimaCura;
} Player;

void IniciarPlayer(Player *player);
void ResetarPlayer(Player *player);

bool DanoPlayer(Player *player, int dano);
int CurarPlayer(Player *player, int quantidade);
bool PlayerInvulneravel(const Player *player);
bool PlayerEsquivando(const Player *player);
float ProgressoEsquiva(const Player *player);

Vector2 CentroPlayer(const Player *player);
Vector2 PePlayer(const Player *player);
Rectangle CorpoPlayer(const Player *player);

void AtualizarPlayer(Player *player, const Arena *arena, const Entrada *entrada, float deltaTime);
void DesenharPlayer(const Player *player);
void FinalizarPlayer(Player *player);

#endif
