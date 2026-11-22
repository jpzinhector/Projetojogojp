/* ============================================================
 *  arena.h
 *  INTERFACE PÚBLICA do módulo Arena.
 *  Quem usar a arena (main.c, player.c...) só precisa incluir este arquivo.
 *  Aqui ficam apenas: o tipo (struct) e os protótipos das funções.
 *  A implementação fica em arena.c.
 * ============================================================ */
#ifndef ARENA_H
#define ARENA_H

#include "raylib.h"

typedef struct {
    Texture2D fundo;     // imagem de fundo da arena
    Rectangle limites;   // área onde o jogador pode andar (x, y, largura, altura)
} Arena;

// Carrega a imagem e define os limites da área jogável
void IniciarArena(Arena *arena);

// Desenha o fundo esticado na janela inteira
// "const" = a função só LÊ a arena, não modifica
void DesenharArena(const Arena *arena);

// Libera a textura da memória da GPU
void FinalizarArena(Arena *arena);

#endif /* ARENA_H */