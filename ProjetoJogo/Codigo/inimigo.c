/* ============================================================
 *  inimigo.c
 *  Implementação do módulo Inimigo (Orc).
 * ============================================================ */
#include "inimigo.h"
#include <stdio.h>
#include <math.h>

#define ALTURA_ORC       110.0f   // altura com que o orc aparece na tela
#define VIDA_BASE        30       // vida de um orc = VIDA_BASE + VIDA_POR_NIVEL * nivel
#define VIDA_POR_NIVEL   10
#define DURACAO_FLASH    0.15f    // segundos que o orc fica avermelhado ao levar dano

Texture2D CarregarSpriteOrc(void)
{
    // Coloque a imagem do orc em assets/orc.png
    return LoadTexture("assets/orc.png");
}

void IniciarInimigo(Inimigo *inimigo, Texture2D sprite, Vector2 posicao, int nivel)
{
    inimigo->sprite  = sprite;
    inimigo->posicao = posicao;
    inimigo->nivel   = nivel;

    inimigo->vidaMaxima = VIDA_BASE + VIDA_POR_NIVEL * nivel;
    inimigo->vida       = inimigo->vidaMaxima;
    inimigo->tempoDano  = 0.0f;

    // Mantém a proporção da imagem original (evita o orc "esticado").
    // Se a textura não carregou (altura 0), usa uma proporção padrão.
    float proporcao = (sprite.height > 0) ? (float)sprite.width / (float)sprite.height : 0.7f;
    inimigo->tamanho = (Vector2){ ALTURA_ORC * proporcao, ALTURA_ORC };

    // Vira o orc para o centro da arena. O sprite original olha para a direita.
    float centroOrcX = posicao.x + inimigo->tamanho.x / 2.0f;
    inimigo->olhandoDireita = (centroOrcX < GetScreenWidth() / 2.0f);
}

void AtualizarInimigo(Inimigo *inimigo, float deltaTime)
{
    if (inimigo->tempoDano > 0.0f) {
        inimigo->tempoDano -= deltaTime;
        if (inimigo->tempoDano < 0.0f) inimigo->tempoDano = 0.0f;
    }
}

bool InimigoPertoDe(const Inimigo *inimigo, Vector2 alvo, float alcance)
{
    if (inimigo->vida <= 0) return false;   // morto não leva golpe

    // Centro do orc
    float centroX = inimigo->posicao.x + inimigo->tamanho.x / 2.0f;
    float centroY = inimigo->posicao.y + inimigo->tamanho.y / 2.0f;

    float dx = centroX - alvo.x;
    float dy = centroY - alvo.y;

    return sqrtf(dx * dx + dy * dy) <= alcance;
}

bool DanoInimigo(Inimigo *inimigo, int dano)
{
    if (inimigo->vida <= 0) return false;

    inimigo->vida -= dano;
    inimigo->tempoDano = DURACAO_FLASH;

    if (inimigo->vida <= 0) {
        inimigo->vida = 0;
        return true;    // este golpe matou o orc
    }
    return false;
}

void DesenharInimigo(const Inimigo *inimigo)
{
    if (inimigo->vida <= 0) return;   // morto: não desenha

    /* ---- Sprite ---- */
    Rectangle origem = {
        0, 0,
        (float)inimigo->sprite.width,
        (float)inimigo->sprite.height
    };

    // Largura negativa = espelha na horizontal
    if (!inimigo->olhandoDireita) {
        origem.width = -origem.width;
    }

    Rectangle destino = {
        inimigo->posicao.x, inimigo->posicao.y,
        inimigo->tamanho.x, inimigo->tamanho.y
    };

    // Avermelha o orc por um instante quando ele leva dano
    Color tinta = (inimigo->tempoDano > 0.0f) ? (Color){ 255, 110, 110, 255 } : WHITE;

    DrawTexturePro(inimigo->sprite, origem, destino, (Vector2){ 0, 0 }, 0.0f, tinta);

    /* ---- Barra de vida (acima da cabeça) ---- */
    float larguraBarra = inimigo->tamanho.x;
    float alturaBarra  = 6.0f;
    float barraX = inimigo->posicao.x;
    float barraY = inimigo->posicao.y - 12.0f;

    float porcentagem = (float)inimigo->vida / (float)inimigo->vidaMaxima;
    if (porcentagem < 0.0f) porcentagem = 0.0f;

    DrawRectangle((int)barraX - 1, (int)barraY - 1, (int)larguraBarra + 2, (int)alturaBarra + 2, BLACK);
    DrawRectangle((int)barraX, (int)barraY, (int)larguraBarra, (int)alturaBarra, (Color){ 90, 20, 20, 255 });
    DrawRectangle((int)barraX, (int)barraY, (int)(larguraBarra * porcentagem), (int)alturaBarra, (Color){ 220, 40, 40, 255 });

    /* ---- Nível ---- */
    char textoNivel[16];
    snprintf(textoNivel, sizeof(textoNivel), "Nv %d", inimigo->nivel);
    DrawText(textoNivel, (int)barraX, (int)barraY - 16, 14, WHITE);
}

void DescarregarSpriteOrc(Texture2D sprite)
{
    UnloadTexture(sprite);
}