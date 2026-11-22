#include "raylib.h"
#include <stdlib.h>
#include <stdbool.h>

#include "config.h"
#include "arena.h"
#include "player.h"
#include "inimigo.h"
#include "lista_inimigos.h"
#include "fila_ondas.h"
#include "ondas.h"
#include "ui.h"

#define ALCANCE_ATAQUE        120.0f
#define DANO_ATAQUE           10
#define PONTOS_POR_ORC        100
#define RECARGA_ATAQUE        0.3f
#define DURACAO_EFEITO_ATAQUE 0.12f
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
    float  recargaAtaque;
    float  efeitoAtaque;
} Partida;

static void ReiniciarPartida(Partida *partida, Player *player, ListaInimigos *inimigos, FilaOndas *ondas)
{
    ListaLimpar(inimigos);
    FilaLimpar(ondas);
    CarregarOndas(ondas);
    ResetarPlayer(player);

    partida->estado        = ESTADO_JOGANDO;
    partida->pontos        = 0;
    partida->tempo         = 0.0f;
    partida->ondaAtual     = 0;
    partida->esperaOnda    = 0.0f;
    partida->recargaAtaque = 0.0f;
    partida->efeitoAtaque  = 0.0f;
}

static void AtualizarPartida(Partida *partida, Player *player, const Arena *arena,
                             ListaInimigos *inimigos, FilaOndas *ondas, Texture2D spriteOrc, float dt)
{
    AtualizarPlayer(player, arena);
    Vector2 centro = CentroPlayer(player);

    partida->tempo += dt;

    if (partida->recargaAtaque > 0.0f) partida->recargaAtaque -= dt;
    if (partida->efeitoAtaque  > 0.0f) partida->efeitoAtaque  -= dt;

    if (IsKeyPressed(KEY_SPACE) && partida->recargaAtaque <= 0.0f) {
        partida->recargaAtaque = RECARGA_ATAQUE;
        partida->efeitoAtaque  = DURACAO_EFEITO_ATAQUE;

        for (NoInimigo *no = inimigos->inicio; no != NULL; no = no->proximo) {
            if (InimigoPertoDe(&no->inimigo, centro, ALCANCE_ATAQUE)) {
                if (DanoInimigo(&no->inimigo, DANO_ATAQUE)) partida->pontos += PONTOS_POR_ORC;
            }
        }
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

static void DesenharPartida(const Partida *partida, const Player *player, const Arena *arena,
                            const ListaInimigos *inimigos, const FilaOndas *ondas)
{
    DesenharArena(arena);

    for (const NoInimigo *no = inimigos->inicio; no != NULL; no = no->proximo) {
        DesenharInimigo(&no->inimigo);
    }

    DesenharPlayer(player);

    if (partida->efeitoAtaque > 0.0f) {
        Vector2 centro = CentroPlayer(player);
        DrawCircleV(centro, ALCANCE_ATAQUE, Fade(WHITE, 0.12f));
        DrawCircleLines((int)centro.x, (int)centro.y, ALCANCE_ATAQUE, Fade(WHITE, 0.6f));
    }

    DesenharUI(partida->pontos, partida->tempo);
    DesenharPainelJogador(player, partida->ondaAtual, TOTAL_ONDAS, inimigos->tamanho);

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

    Texture2D spriteOrc = CarregarSpriteOrc();

    ListaInimigos inimigos;
    FilaOndas ondas;
    Partida partida;

    ListaIniciar(&inimigos);
    FilaIniciar(&ondas);
    ReiniciarPartida(&partida, player, &inimigos, &ondas);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        if (partida.estado == ESTADO_JOGANDO) {
            AtualizarPartida(&partida, player, arena, &inimigos, &ondas, spriteOrc, dt);
        } else if (IsKeyPressed(KEY_ENTER)) {
            ReiniciarPartida(&partida, player, &inimigos, &ondas);
        }

        BeginDrawing();

            ClearBackground(BLACK);
            DesenharPartida(&partida, player, arena, &inimigos, &ondas);

        EndDrawing();
    }

    ListaLimpar(&inimigos);
    FilaLimpar(&ondas);

    FinalizarPlayer(player);
    FinalizarArena(arena);
    DescarregarSpriteOrc(spriteOrc);

    free(player);
    free(arena);

    CloseWindow();

    return 0;
}
