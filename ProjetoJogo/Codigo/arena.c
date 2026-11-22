/* ============================================================
 *  arena.c
 *  Implementação do módulo Arena (fundo + limites da área jogável).
 * ============================================================ */
#include "arena.h"
#include "config.h"   // LARGURA_JANELA e ALTURA_JANELA

void IniciarArena(Arena *arena)
{
    // Carrega a imagem de fundo (o arquivo fica na pasta assets/)
    arena->fundo = LoadTexture("assets/arena.png");

    // Área onde o jogador pode andar: x, y, largura, altura
    arena->limites = (Rectangle){ 55, 100, 840, 590 };
}

void DesenharArena(const Arena *arena)
{
    // Estica a imagem inteira para cobrir a janela toda
    DrawTexturePro(
        arena->fundo,
        (Rectangle){ 0, 0, (float)arena->fundo.width, (float)arena->fundo.height },
        (Rectangle){ 0, 0, LARGURA_JANELA, ALTURA_JANELA },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );
}

void FinalizarArena(Arena *arena)
{
    UnloadTexture(arena->fundo);
}
