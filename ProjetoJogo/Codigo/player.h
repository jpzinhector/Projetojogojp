/* ============================================================
 *  player.h
 *  INTERFACE PÚBLICA do módulo Player.
 *  A struct fica aqui (e não escondida no .c) porque o main.c
 *  precisa saber o tamanho dela para fazer o malloc.
 * ============================================================ */
#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include "raylib.h"
#include "arena.h"            // AtualizarPlayer usa o tipo Arena (mesma pasta)

typedef struct {
    Vector2 posicao;
    Vector2 tamanho;      // tamanho com que o player é DESENHADO (e usado nos limites)
    Texture2D sprite;     // sprite sheet inteiro (grade de frames)
    float velocidade;     // pixels por segundo

    /* ---- Campos da animação ---- */
    float larguraFrame;   // largura de UM frame dentro do sprite sheet
    float alturaFrame;    // altura de UM frame dentro do sprite sheet
    int   frameAtual;     // índice do frame mostrado agora (0 a 9)
    float tempoFrame;     // segundos acumulados desde a última troca de frame
    bool  andando;        // true se alguma tecla de movimento está pressionada
    bool  olhandoDireita; // true = sprite original; false = espelhado

    int   vida;
    int   vidaMaxima;
    float tempoInvulneravel;
} Player;

// Carrega o sprite, define tamanho, posição inicial e estado da animação
void IniciarPlayer(Player *player);

void ResetarPlayer(Player *player);

bool DanoPlayer(Player *player, int dano);

Vector2 CentroPlayer(const Player *player);

// Lê o teclado, move o player, aplica os limites da arena e avança a animação
// A arena é "const": o player só precisa LER os limites dela
void AtualizarPlayer(Player *player, const Arena *arena);

// Desenha o frame atual (espelhado se estiver olhando para a esquerda)
void DesenharPlayer(const Player *player);

// Libera a textura do player
void FinalizarPlayer(Player *player);

#endif /* PLAYER_H */