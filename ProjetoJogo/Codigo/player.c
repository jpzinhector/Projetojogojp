#include "player.h"
#include "config.h"
#include "raymath.h"
#include <stdio.h>

#define FRAMES_COLUNAS      5
#define FRAMES_LINHAS       2
#define TOTAL_FRAMES        10
#define FPS_ANIMACAO        12.0f
#define FRAME_PARADO        0
#define ALTURA_PLAYER       104.0f
#define VIDA_PLAYER         100
#define TEMPO_INVULNERAVEL  1.0f
#define DURACAO_ESQUIVA     0.2f
#define VELOCIDADE_ESQUIVA  720.0f
#define RECARGA_ESQUIVA     0.9f
#define FOLGA_ESQUIVA       0.08f
#define INTERVALO_RASTRO    0.025f
#define DURACAO_RASTRO      0.28f
#define DURACAO_TEXTO_CURA  0.9f

void IniciarPlayer(Player *player)
{
    player->sprite = LoadTexture("assets/player_walk.png");

    player->larguraFrame = (float)player->sprite.width  / FRAMES_COLUNAS;
    player->alturaFrame  = (float)player->sprite.height / FRAMES_LINHAS;

    float proporcao = (player->alturaFrame > 0.0f)
                    ? player->larguraFrame / player->alturaFrame
                    : 0.72f;
    player->tamanho = (Vector2){ ALTURA_PLAYER * proporcao, ALTURA_PLAYER };

    player->velocidade = 250.0f;
    player->vidaMaxima = VIDA_PLAYER;

    ResetarPlayer(player);
}

void ResetarPlayer(Player *player)
{
    player->posicao = (Vector2){
        LARGURA_JANELA / 2.0f - player->tamanho.x / 2.0f,
        ALTURA_JANELA  / 2.0f - player->tamanho.y / 2.0f
    };

    player->frameAtual     = FRAME_PARADO;
    player->tempoFrame     = 0.0f;
    player->andando        = false;
    player->olhandoDireita = true;

    player->vida              = player->vidaMaxima;
    player->tempoInvulneravel = 0.0f;

    player->tempoEsquiva   = 0.0f;
    player->recargaEsquiva = 0.0f;
    player->direcaoEsquiva = (Vector2){ 1.0f, 0.0f };
    player->tempoRastro    = 0.0f;
    for (int i = 0; i < RASTRO_MAX; i++) player->rastro[i].idade = DURACAO_RASTRO;

    player->tempoCura = 0.0f;
    player->ultimaCura = 0;
}

bool PlayerEsquivando(const Player *player)
{
    return player->tempoEsquiva > 0.0f;
}

bool PlayerInvulneravel(const Player *player)
{
    return player->tempoInvulneravel > 0.0f || player->tempoEsquiva > 0.0f;
}

float ProgressoEsquiva(const Player *player)
{
    if (player->recargaEsquiva <= 0.0f) return 1.0f;
    return 1.0f - player->recargaEsquiva / RECARGA_ESQUIVA;
}

bool DanoPlayer(Player *player, int dano)
{
    if (PlayerInvulneravel(player) || player->vida <= 0) return false;

    player->vida -= dano;
    player->tempoInvulneravel = TEMPO_INVULNERAVEL;

    if (player->vida <= 0) {
        player->vida = 0;
        return true;
    }
    return false;
}

int CurarPlayer(Player *player, int quantidade)
{
    if (player->vida <= 0 || player->vida >= player->vidaMaxima) return 0;

    int antes = player->vida;
    player->vida += quantidade;
    if (player->vida > player->vidaMaxima) player->vida = player->vidaMaxima;

    player->ultimaCura = player->vida - antes;
    player->tempoCura = DURACAO_TEXTO_CURA;
    return player->ultimaCura;
}

Vector2 CentroPlayer(const Player *player)
{
    return (Vector2){
        player->posicao.x + player->tamanho.x / 2.0f,
        player->posicao.y + player->tamanho.y / 2.0f
    };
}

Vector2 PePlayer(const Player *player)
{
    return (Vector2){
        player->posicao.x + player->tamanho.x / 2.0f,
        player->posicao.y + player->tamanho.y
    };
}

Rectangle CorpoPlayer(const Player *player)
{
    return (Rectangle){
        player->posicao.x + player->tamanho.x * 0.2f,
        player->posicao.y + player->tamanho.y * 0.1f,
        player->tamanho.x * 0.6f,
        player->tamanho.y * 0.85f
    };
}

static void LimitarNaArena(Player *player, const Arena *arena)
{
    if (player->posicao.x < arena->limites.x)
        player->posicao.x = arena->limites.x;

    if (player->posicao.y < arena->limites.y)
        player->posicao.y = arena->limites.y;

    if (player->posicao.x + player->tamanho.x > arena->limites.x + arena->limites.width)
        player->posicao.x = arena->limites.x + arena->limites.width - player->tamanho.x;

    if (player->posicao.y + player->tamanho.y > arena->limites.y + arena->limites.height)
        player->posicao.y = arena->limites.y + arena->limites.height - player->tamanho.y;
}

static void IniciarEsquiva(Player *player, Vector2 direcao)
{
    if (direcao.x == 0.0f && direcao.y == 0.0f) {
        direcao = (Vector2){ player->olhandoDireita ? 1.0f : -1.0f, 0.0f };
    } else {
        direcao = Vector2Normalize(direcao);
    }

    player->direcaoEsquiva = direcao;
    player->tempoEsquiva = DURACAO_ESQUIVA;
    player->recargaEsquiva = RECARGA_ESQUIVA;
    player->tempoRastro = 0.0f;

    if (direcao.x > 0.1f) player->olhandoDireita = true;
    if (direcao.x < -0.1f) player->olhandoDireita = false;
}

static void RegistrarRastro(Player *player)
{
    int mais_antigo = 0;
    for (int i = 1; i < RASTRO_MAX; i++) {
        if (player->rastro[i].idade > player->rastro[mais_antigo].idade) mais_antigo = i;
    }

    player->rastro[mais_antigo].posicao = player->posicao;
    player->rastro[mais_antigo].frame = player->frameAtual;
    player->rastro[mais_antigo].direita = player->olhandoDireita;
    player->rastro[mais_antigo].idade = 0.0f;
}

void AtualizarPlayer(Player *player, const Arena *arena, const Entrada *entrada, float deltaTime)
{
    if (player->tempoInvulneravel > 0.0f) {
        player->tempoInvulneravel -= deltaTime;
        if (player->tempoInvulneravel < 0.0f) player->tempoInvulneravel = 0.0f;
    }

    if (player->recargaEsquiva > 0.0f) {
        player->recargaEsquiva -= deltaTime;
        if (player->recargaEsquiva < 0.0f) player->recargaEsquiva = 0.0f;
    }

    if (player->tempoCura > 0.0f) {
        player->tempoCura -= deltaTime;
        if (player->tempoCura < 0.0f) player->tempoCura = 0.0f;
    }

    for (int i = 0; i < RASTRO_MAX; i++) {
        if (player->rastro[i].idade < DURACAO_RASTRO) player->rastro[i].idade += deltaTime;
    }

    Vector2 direcao = { entrada->moverX, entrada->moverY };

    if (entrada->esquivar && player->recargaEsquiva <= 0.0f && player->tempoEsquiva <= 0.0f && player->vida > 0) {
        IniciarEsquiva(player, direcao);
    }

    if (player->tempoEsquiva > 0.0f) {
        player->tempoEsquiva -= deltaTime;
        player->posicao.x += player->direcaoEsquiva.x * VELOCIDADE_ESQUIVA * deltaTime;
        player->posicao.y += player->direcaoEsquiva.y * VELOCIDADE_ESQUIVA * deltaTime;
        LimitarNaArena(player, arena);

        player->tempoRastro -= deltaTime;
        if (player->tempoRastro <= 0.0f) {
            RegistrarRastro(player);
            player->tempoRastro = INTERVALO_RASTRO;
        }

        if (player->tempoEsquiva <= 0.0f) {
            player->tempoEsquiva = 0.0f;
            if (player->tempoInvulneravel < FOLGA_ESQUIVA) player->tempoInvulneravel = FOLGA_ESQUIVA;
        }

        player->andando = true;
        return;
    }

    player->andando = (direcao.x != 0.0f || direcao.y != 0.0f);

    if (direcao.x > 0.0f) player->olhandoDireita = true;
    if (direcao.x < 0.0f) player->olhandoDireita = false;

    if (player->andando) {
        direcao = Vector2Normalize(direcao);
    }

    player->posicao.x += direcao.x * player->velocidade * deltaTime;
    player->posicao.y += direcao.y * player->velocidade * deltaTime;
    LimitarNaArena(player, arena);

    if (player->andando) {
        float duracaoFrame = 1.0f / FPS_ANIMACAO;

        player->tempoFrame += deltaTime;

        while (player->tempoFrame >= duracaoFrame) {
            player->tempoFrame -= duracaoFrame;
            player->frameAtual = (player->frameAtual + 1) % TOTAL_FRAMES;
        }
    } else {
        player->frameAtual = FRAME_PARADO;
        player->tempoFrame = 0.0f;
    }
}

static void DesenharFrame(const Player *player, Vector2 posicao, int frame, bool direita, Color tinta)
{
    int coluna = frame % FRAMES_COLUNAS;
    int linha  = frame / FRAMES_COLUNAS;

    Rectangle origem = {
        coluna * player->larguraFrame,
        linha  * player->alturaFrame,
        player->larguraFrame,
        player->alturaFrame
    };

    if (!direita) origem.width = -origem.width;

    Rectangle destino = { posicao.x, posicao.y, player->tamanho.x, player->tamanho.y };
    DrawTexturePro(player->sprite, origem, destino, (Vector2){ 0, 0 }, 0.0f, tinta);
}

void DesenharPlayer(const Player *player)
{
    for (int i = 0; i < RASTRO_MAX; i++) {
        const RastroPlayer *r = &player->rastro[i];
        if (r->idade >= DURACAO_RASTRO) continue;

        float v = 1.0f - r->idade / DURACAO_RASTRO;
        DesenharFrame(player, r->posicao, r->frame, r->direita, (Color){ 120, 200, 255, (unsigned char)(150 * v * v) });
    }

    Color tinta = WHITE;
    if (player->tempoEsquiva > 0.0f) {
        tinta = (Color){ 190, 230, 255, 215 };
    } else if (player->tempoInvulneravel > 0.0f) {
        tinta = (Color){ 255, 110, 110, 255 };
    } else if (player->tempoCura > DURACAO_TEXTO_CURA - 0.3f) {
        tinta = (Color){ 170, 255, 170, 255 };
    }

    DrawEllipse((int)(player->posicao.x + player->tamanho.x / 2.0f), (int)(player->posicao.y + player->tamanho.y - 3.0f),
                player->tamanho.x * 0.34f, player->tamanho.x * 0.11f, (Color){ 0, 0, 0, 70 });

    DesenharFrame(player, player->posicao, player->frameAtual, player->olhandoDireita, tinta);

    if (player->tempoCura > 0.0f) {
        float t = player->tempoCura / DURACAO_TEXTO_CURA;
        unsigned char alfa = (unsigned char)(255.0f * (t > 0.3f ? 1.0f : t / 0.3f));

        char texto[16];
        snprintf(texto, sizeof(texto), "+%d", player->ultimaCura);

        int largura = MeasureText(texto, 24);
        int x = (int)(player->posicao.x + player->tamanho.x / 2.0f) - largura / 2;
        int y = (int)(player->posicao.y - 22.0f - (1.0f - t) * 34.0f);

        Color contorno = { 0, 0, 0, alfa };
        DrawText(texto, x - 2, y, 24, contorno);
        DrawText(texto, x + 2, y, 24, contorno);
        DrawText(texto, x, y - 2, 24, contorno);
        DrawText(texto, x, y + 2, 24, contorno);
        DrawText(texto, x, y, 24, (Color){ 110, 245, 120, alfa });
    }
}

void FinalizarPlayer(Player *player)
{
    UnloadTexture(player->sprite);
}
