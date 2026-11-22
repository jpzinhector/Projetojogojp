#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "config.h"
#include "arena.h"
#include "player.h"
#include "inimigo.h"
#include "arma.h"
#include "lista_inimigos.h"
#include "fila_ondas.h"
#include "ondas.h"
#include "ui.h"

#define ALCANCE_ATAQUE        120.0f
#define DANO_ATAQUE           10
#define PONTOS_POR_ORC        100
#define MARGEM_ATRAS_ATAQUE   20.0f
#define RAIO_CONTATO          55.0f
#define ESPERA_ENTRE_ONDAS    2.0f

typedef enum {
    ESTADO_JOGANDO,
    ESTADO_DERROTA,
    ESTADO_VITORIA
} Estado;

typedef struct {
    Estado estado;
    int    pontos;
    float  tempo;
    int    ondaAtual;
    float  esperaOnda;
} Partida;

static void ReiniciarPartida(Partida *partida, Player *player, Arma *arma, ListaInimigos *inimigos, FilaOndas *ondas)
{
    ListaLimpar(inimigos);
    FilaLimpar(ondas);
    CarregarOndas(ondas);
    ResetarPlayer(player);
    ResetarArma(arma);

    partida->estado        = ESTADO_JOGANDO;
    partida->pontos        = 0;
    partida->tempo         = 0.0f;
    partida->ondaAtual     = 0;
    partida->esperaOnda    = 0.0f;
}

static void TentarAtacar(Arma *arma, Player *player, const ListaInimigos *inimigos)
{
    Vector2 centro = CentroPlayer(player);
    bool direita = player->olhandoDireita;
    float menorDistancia = ALCANCE_ATAQUE + 1.0f;

    for (const NoInimigo *no = inimigos->inicio; no != NULL; no = no->proximo) {
        if (!InimigoPertoDe(&no->inimigo, centro, ALCANCE_ATAQUE)) continue;

        float dx = (no->inimigo.posicao.x + no->inimigo.tamanho.x / 2.0f) - centro.x;
        float dy = (no->inimigo.posicao.y + no->inimigo.tamanho.y / 2.0f) - centro.y;
        float distancia = sqrtf(dx * dx + dy * dy);

        if (distancia < menorDistancia) {
            menorDistancia = distancia;
            direita = (dx >= 0.0f);
        }
    }

    IniciarGolpe(arma, direita);
}

static void AplicarGolpe(Partida *partida, const Arma *arma, const Player *player, ListaInimigos *inimigos)
{
    Vector2 centro = CentroPlayer(player);

    for (NoInimigo *no = inimigos->inicio; no != NULL; no = no->proximo) {
        if (!InimigoPertoDe(&no->inimigo, centro, ALCANCE_ATAQUE)) continue;

        float dx = (no->inimigo.posicao.x + no->inimigo.tamanho.x / 2.0f) - centro.x;
        bool naFrente = arma->direita ? (dx >= -MARGEM_ATRAS_ATAQUE) : (dx <= MARGEM_ATRAS_ATAQUE);
        if (!naFrente) continue;

        if (GolpearInimigo(&no->inimigo, DANO_ATAQUE, centro)) {
            partida->pontos += PONTOS_POR_ORC;
        }
    }
}

static void AtualizarPartida(Partida *partida, Player *player, Arma *arma, const Arena *arena,
                             ListaInimigos *inimigos, FilaOndas *ondas, Texture2D spriteOrc, float dt)
{
    AtualizarPlayer(player, arena);
    Vector2 centro = CentroPlayer(player);

    partida->tempo += dt;

    if (IsKeyPressed(KEY_SPACE)) {
        TentarAtacar(arma, player, inimigos);
    }

    AtualizarArma(arma, dt);

    if (arma->atacando) {
        player->olhandoDireita = arma->direita;
    }

    if (ConsumirImpacto(arma)) {
        AplicarGolpe(partida, arma, player, inimigos);
    }

    for (NoInimigo *no = inimigos->inicio; no != NULL; no = no->proximo) {
        AtualizarInimigo(&no->inimigo, dt);
        MoverInimigo(&no->inimigo, centro, dt, arena->limites);

        if (InimigoPertoDe(&no->inimigo, centro, RAIO_CONTATO)) {
            if (DanoPlayer(player, DanoContatoInimigo(&no->inimigo))) {
                partida->estado = ESTADO_DERROTA;
            }
        }
    }

    for (NoInimigo *a = inimigos->inicio; a != NULL; a = a->proximo) {
        for (NoInimigo *b = a->proximo; b != NULL; b = b->proximo) {
            AfastarInimigos(&a->inimigo, &b->inimigo, arena->limites);
        }
    }

    ListaRemoverMortos(inimigos);

    if (partida->estado == ESTADO_DERROTA) return;

    if (ListaVazia(inimigos)) {
        if (FilaVazia(ondas)) {
            partida->estado = ESTADO_VITORIA;
        } else {
            partida->esperaOnda -= dt;

            if (partida->esperaOnda <= 0.0f) {
                Onda proxima;
                if (FilaDesenfileirar(ondas, &proxima)) {
                    GerarOnda(inimigos, proxima, spriteOrc, arena->limites, centro);
                    partida->ondaAtual++;
                }
                partida->esperaOnda = ESPERA_ENTRE_ONDAS;
            }
        }
    }
}

static void DesenharPartida(const Partida *partida, const Player *player, const Arma *arma, const Arena *arena,
                            const ListaInimigos *inimigos, const FilaOndas *ondas)
{
    DesenharArena(arena);

    for (const NoInimigo *no = inimigos->inicio; no != NULL; no = no->proximo) {
        DesenharInimigo(&no->inimigo);
    }

    DesenharPlayer(player);
    DesenharArma(arma, player);

    DesenharUI(partida->pontos, partida->tempo);
    DesenharPainelJogador(player, partida->ondaAtual, TOTAL_ONDAS, ListaContarVivos(inimigos));

    if (partida->estado == ESTADO_JOGANDO && ListaVazia(inimigos) && !FilaVazia(ondas) && partida->ondaAtual > 0) {
        DesenharAvisoOnda(partida->ondaAtual + 1);
    }

    if (partida->estado == ESTADO_VITORIA) {
        DesenharTelaFinal(true, partida->pontos, partida->tempo);
    } else if (partida->estado == ESTADO_DERROTA) {
        DesenharTelaFinal(false, partida->pontos, partida->tempo);
    }
}

int main(void)
{
    InitWindow(LARGURA_JANELA, ALTURA_JANELA, TITULO_JANELA);
    SetTargetFPS(60);

    SetWindowPosition(50, 50);

    Player *player = malloc(sizeof(Player));
    Arena *arena = malloc(sizeof(Arena));

    if (player == NULL || arena == NULL) {
        free(player);
        free(arena);
        CloseWindow();
        return 1;
    }

    IniciarPlayer(player);
    IniciarArena(arena);

    Arma arma;
    IniciarArma(&arma);

    Texture2D spriteOrc = CarregarSpriteOrc();

    ListaInimigos inimigos;
    FilaOndas ondas;
    Partida partida;

    ListaIniciar(&inimigos);
    FilaIniciar(&ondas);
    ReiniciarPartida(&partida, player, &arma, &inimigos, &ondas);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (partida.estado == ESTADO_JOGANDO) {
            AtualizarPartida(&partida, player, &arma, arena, &inimigos, &ondas, spriteOrc, dt);
        } else if (IsKeyPressed(KEY_ENTER)) {
            ReiniciarPartida(&partida, player, &arma, &inimigos, &ondas);
        }

        BeginDrawing();

            ClearBackground(BLACK);
            DesenharPartida(&partida, player, &arma, arena, &inimigos, &ondas);

        EndDrawing();
    }

    ListaLimpar(&inimigos);
    FilaLimpar(&ondas);

    FinalizarArma(&arma);
    FinalizarPlayer(player);
    FinalizarArena(arena);
    DescarregarSpriteOrc(spriteOrc);

    free(player);
    free(arena);

    CloseWindow();

    return 0;
}
