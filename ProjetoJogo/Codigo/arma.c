#include "arma.h"
#include <math.h>

#define PI_F             3.14159265f
#define DURACAO_GOLPE    0.28f
#define RECARGA_GOLPE    0.36f
#define MOMENTO_IMPACTO  0.50f
#define FIM_PREPARO      0.22f
#define FIM_GOLPE        0.62f
#define DURACAO_RASTRO   0.15f
#define ANGULO_REPOUSO   25.0f
#define ANGULO_PREPARO  -70.0f
#define ANGULO_FINAL     125.0f
#define ESCALA_ARMA      0.5f
#define PIVO_X           28.0f
#define PIVO_Y           126.0f
#define MAO_X            0.66f
#define MAO_Y            0.60f
#define RASTRO_RAIO_MIN  34.0f
#define RASTRO_RAIO_MAX  66.0f
#define RASTRO_ABERTURA  110.0f
#define RASTRO_PARTES    8

static float Suavizar(float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

static float ProgressoDoGolpe(const Arma *arma)
{
    float u = arma->tempo / DURACAO_GOLPE;
    if (u < 0.0f) u = 0.0f;
    if (u > 1.0f) u = 1.0f;
    return u;
}

static float AnguloDoGolpe(float u)
{
    if (u < FIM_PREPARO) {
        float t = u / FIM_PREPARO;
        return ANGULO_REPOUSO + (ANGULO_PREPARO - ANGULO_REPOUSO) * Suavizar(t);
    }

    if (u < FIM_GOLPE) {
        float t = (u - FIM_PREPARO) / (FIM_GOLPE - FIM_PREPARO);
        return ANGULO_PREPARO + (ANGULO_FINAL - ANGULO_PREPARO) * Suavizar(t);
    }

    float t = (u - FIM_GOLPE) / (1.0f - FIM_GOLPE);
    return ANGULO_FINAL + (ANGULO_REPOUSO - ANGULO_FINAL) * Suavizar(t);
}

static void DesenharRastro(Vector2 mao, float lado, float u)
{
    if (u < FIM_PREPARO) return;

    float visibilidade = 1.0f;
    float anguloPonta;

    if (u < FIM_GOLPE) {
        anguloPonta = AnguloDoGolpe(u);
    } else {
        anguloPonta = ANGULO_FINAL;
        visibilidade = 1.0f - (u - FIM_GOLPE) / DURACAO_RASTRO;
        if (visibilidade <= 0.0f) return;
    }

    float anguloCauda = anguloPonta - RASTRO_ABERTURA;
    if (anguloCauda < ANGULO_PREPARO) anguloCauda = ANGULO_PREPARO;

    for (int i = 0; i < RASTRO_PARTES; i++) {
        float a0 = anguloCauda + (anguloPonta - anguloCauda) * (float)i / RASTRO_PARTES;
        float a1 = anguloCauda + (anguloPonta - anguloCauda) * (float)(i + 1) / RASTRO_PARTES;
        float opacidade = 0.55f * visibilidade * (float)(i + 1) / RASTRO_PARTES;

        DrawRing(mao, RASTRO_RAIO_MIN, RASTRO_RAIO_MAX,
                 a0 * lado - 90.0f, a1 * lado - 90.0f, 6, Fade(RAYWHITE, opacidade));
    }
}

void IniciarArma(Arma *arma)
{
    arma->sprite = LoadTexture("assets/arma.png");
    SetTextureFilter(arma->sprite, TEXTURE_FILTER_BILINEAR);
    ResetarArma(arma);
}

void ResetarArma(Arma *arma)
{
    arma->tempo           = 0.0f;
    arma->recarga         = 0.0f;
    arma->atacando        = false;
    arma->impactoPendente = false;
    arma->direita         = true;
}

void FinalizarArma(Arma *arma)
{
    UnloadTexture(arma->sprite);
}

bool IniciarGolpe(Arma *arma, bool direita)
{
    if (arma->atacando || arma->recarga > 0.0f) return false;

    arma->atacando        = true;
    arma->impactoPendente = true;
    arma->tempo           = 0.0f;
    arma->recarga         = RECARGA_GOLPE;
    arma->direita         = direita;
    return true;
}

void AtualizarArma(Arma *arma, float deltaTime)
{
    if (arma->recarga > 0.0f) {
        arma->recarga -= deltaTime;
        if (arma->recarga < 0.0f) arma->recarga = 0.0f;
    }

    if (arma->atacando) {
        arma->tempo += deltaTime;
        if (arma->tempo >= DURACAO_GOLPE) {
            arma->atacando = false;
            arma->tempo = 0.0f;
        }
    }
}

bool ConsumirImpacto(Arma *arma)
{
    if (!arma->impactoPendente) return false;
    if (arma->atacando && ProgressoDoGolpe(arma) < MOMENTO_IMPACTO) return false;

    arma->impactoPendente = false;
    return true;
}

void DesenharArma(const Arma *arma, const Player *player)
{
    bool direita = arma->atacando ? arma->direita : player->olhandoDireita;
    float lado = direita ? 1.0f : -1.0f;

    float balanco = 0.0f;
    if (player->andando && !arma->atacando) {
        balanco = -fabsf(sinf((float)player->frameAtual * PI_F / 5.0f)) * 3.0f;
    }

    Vector2 mao = {
        player->posicao.x + (direita ? MAO_X : 1.0f - MAO_X) * player->tamanho.x,
        player->posicao.y + MAO_Y * player->tamanho.y + balanco
    };

    float u = arma->atacando ? ProgressoDoGolpe(arma) : 0.0f;
    float angulo = arma->atacando ? AnguloDoGolpe(u) : ANGULO_REPOUSO;

    if (arma->atacando) DesenharRastro(mao, lado, u);

    Rectangle origem = { 0, 0, (float)arma->sprite.width, (float)arma->sprite.height };
    if (!direita) origem.width = -origem.width;

    Rectangle destino = {
        mao.x, mao.y,
        arma->sprite.width * ESCALA_ARMA,
        arma->sprite.height * ESCALA_ARMA
    };

    Vector2 pivo = { PIVO_X * ESCALA_ARMA, PIVO_Y * ESCALA_ARMA };

    DrawTexturePro(arma->sprite, origem, destino, pivo, angulo * lado, WHITE);
}
