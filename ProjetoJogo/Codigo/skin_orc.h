#ifndef SKIN_ORC_H
#define SKIN_ORC_H

#include "raylib.h"
#include <stdbool.h>

typedef enum {
    ANIM_PARADO,
    ANIM_ANDAR,
    ANIM_ATAQUE,
    ANIM_DANO,
    ANIM_MORTE,
    ANIM_TOTAL
} AnimacaoOrc;

typedef struct {
    Texture2D tira[ANIM_TOTAL];
    int quadros[ANIM_TOTAL];
    float larguraQuadro;
    float alturaQuadro;
    float ancoraX;
    float ancoraY;
    float alturaRef;
    bool valida;
} SkinOrc;

SkinOrc CarregarSkinOrc(int numero);
void DescarregarSkinOrc(SkinOrc *skin);

bool SkinTemAnimacao(const SkinOrc *skin, AnimacaoOrc animacao);
int SkinQuadros(const SkinOrc *skin, AnimacaoOrc animacao);

void DesenharQuadroSkin(const SkinOrc *skin, AnimacaoOrc animacao, int quadro, Vector2 pe, float altura,
                        bool direita, float inclinacao, float escalaX, float escalaY, Color tinta);

#endif
