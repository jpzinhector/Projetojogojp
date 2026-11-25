#include "skin_orc.h"
#include <stdio.h>

static const char *NOMES[ANIM_TOTAL] = { "idle", "walk", "attak", "hurt", "die" };

SkinOrc CarregarSkinOrc(int numero)
{
    SkinOrc skin = { 0 };

    char caminho[128];
    snprintf(caminho, sizeof(caminho), "assets/orcs/orc%d.txt", numero);

    FILE *arquivo = fopen(caminho, "r");
    if (arquivo == NULL) return skin;

    float largura, altura, ancoraX, ancoraY, alturaRef;
    int lidos = fscanf(arquivo, "%f %f %f %f %f", &largura, &altura, &ancoraX, &ancoraY, &alturaRef);
    fclose(arquivo);
    if (lidos != 5 || largura <= 0.0f || altura <= 0.0f || alturaRef <= 0.0f) return skin;

    skin.larguraQuadro = largura;
    skin.alturaQuadro = altura;
    skin.ancoraX = ancoraX;
    skin.ancoraY = ancoraY;
    skin.alturaRef = alturaRef;

    for (int i = 0; i < ANIM_TOTAL; i++) {
        snprintf(caminho, sizeof(caminho), "assets/orcs/orc%d_%s.png", numero, NOMES[i]);
        if (!FileExists(caminho)) continue;

        skin.tira[i] = LoadTexture(caminho);
        if (skin.tira[i].id == 0) continue;

        SetTextureFilter(skin.tira[i], TEXTURE_FILTER_BILINEAR);
        skin.quadros[i] = (int)((float)skin.tira[i].width / largura + 0.5f);
        if (skin.quadros[i] <= 0) skin.tira[i].id = 0;
    }

    skin.valida = SkinTemAnimacao(&skin, ANIM_ATAQUE) || SkinTemAnimacao(&skin, ANIM_PARADO);
    return skin;
}

void DescarregarSkinOrc(SkinOrc *skin)
{
    for (int i = 0; i < ANIM_TOTAL; i++) {
        if (skin->tira[i].id != 0) UnloadTexture(skin->tira[i]);
        skin->tira[i].id = 0;
        skin->quadros[i] = 0;
    }
    skin->valida = false;
}

bool SkinTemAnimacao(const SkinOrc *skin, AnimacaoOrc animacao)
{
    return skin->tira[animacao].id != 0 && skin->quadros[animacao] > 0;
}

int SkinQuadros(const SkinOrc *skin, AnimacaoOrc animacao)
{
    return SkinTemAnimacao(skin, animacao) ? skin->quadros[animacao] : 0;
}

void DesenharQuadroSkin(const SkinOrc *skin, AnimacaoOrc animacao, int quadro, Vector2 pe, float altura,
                        bool direita, float inclinacao, float escalaX, float escalaY, Color tinta)
{
    if (!SkinTemAnimacao(skin, animacao)) return;

    int total = skin->quadros[animacao];
    if (quadro < 0) quadro = 0;
    if (quadro >= total) quadro = total - 1;

    float escala = altura / skin->alturaRef;
    float largura = skin->larguraQuadro * escala * escalaX;
    float alto = skin->alturaQuadro * escala * escalaY;

    float ancoraX = direita ? skin->ancoraX : skin->larguraQuadro - skin->ancoraX;

    Rectangle origem = { quadro * skin->larguraQuadro, 0.0f, skin->larguraQuadro, skin->alturaQuadro };
    if (!direita) origem.width = -origem.width;

    Rectangle destino = { pe.x, pe.y, largura, alto };
    Vector2 pivo = { ancoraX * escala * escalaX, skin->ancoraY * escala * escalaY };

    DrawTexturePro(skin->tira[animacao], origem, destino, pivo, inclinacao, tinta);
}
