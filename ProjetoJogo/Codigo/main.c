/* ============================================================
 *  main.c
 *  Ponto de entrada do jogo. Aqui ficam só a janela, o laço principal
 *  e a UI; o resto mora nos módulos: arena, player e inimigo.
 * ============================================================ */
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "config.h"    // LARGURA_JANELA, ALTURA_JANELA, TITULO_JANELA
#include "arena.h"
#include "player.h"
#include "inimigo.h"

#define TOTAL_INIMIGOS         5
#define DISTANCIA_MINIMA_SPAWN 250.0f  // distância mínima do player ao nascer
#define ALCANCE_ATAQUE         120.0f  // distância máxima para o golpe acertar
#define DANO_ATAQUE            10      // vida tirada por golpe
#define PONTOS_POR_ORC         100     // pontos ganhos ao matar um orc

void DesenharUI (int pontos, float tempoDecorrido)
{
    int minutos = (int)tempoDecorrido / 60;
    int segundos = (int)tempoDecorrido % 60;

    char bufferPontos[32];
    char bufferTempo[32];

    // Formata os textos
    snprintf(bufferPontos, sizeof(bufferPontos), "PONTOS: %04d", pontos);
    snprintf(bufferTempo, sizeof(bufferTempo), "TEMPO: %02d:%02d", minutos, segundos);

    // Posição da UI (canto superior direito)
    float uiX = 720.0f;
    float uiY = 0.0f;
    float uiLargura = 230.0f;
    float uiAltura = 100.0f;

    Rectangle outerRect = {uiX, uiY, uiLargura, uiAltura};
    Rectangle innerRect = {uiX + 4, uiY + 4, uiLargura - 8, uiAltura - 8};

    // Cores do retângulo
    Color backgroundDark = (Color) {3, 19, 38, 255};
    Color borderHighlight = (Color) {77, 134, 184, 255};

    // Desenha as formas
    DrawRectangleRounded(outerRect, 0.2f, 8, (Color){55, 100, 141, 255});
    DrawRectangleRoundedLines(outerRect, 0.2f, 9, borderHighlight);
    DrawRectangleRounded(innerRect, 0.18f, 8, backgroundDark);

    // Renderiza os textos (DrawText recebe posição em int, então convertemos)
    DrawText (bufferPontos, (int)(uiX + 30), (int)(uiY + 25), 20, WHITE);
    DrawText (bufferTempo,  (int)(uiX + 30), (int)(uiY + 60), 20, WHITE);
}

int main(void)
{
    InitWindow(LARGURA_JANELA, ALTURA_JANELA, TITULO_JANELA);
    SetTargetFPS(60);

    SetWindowPosition(50, 50);

    Player *player = malloc(sizeof(Player));
    Arena *arena = malloc(sizeof(Arena));

    // CORREÇÃO: malloc pode falhar (devolve NULL). Antes o programa
    // usava o ponteiro sem checar e quebrava se faltasse memória.
    if (player == NULL || arena == NULL) {
        free(player);
        free(arena);
        CloseWindow();
        return 1;
    }

    IniciarPlayer(player);
    IniciarArena(arena);

    Texture2D spriteOrc = CarregarSpriteOrc();
    Inimigo inimigos[TOTAL_INIMIGOS];

    Vector2 centroPlayer = {
        player->posicao.x + player->tamanho.x / 2.0f,
        player->posicao.y + player->tamanho.y / 2.0f
    };

    for (int i = 0; i < TOTAL_INIMIGOS; i++) {
        int nivel = 1 + i % 3;

        // CORREÇÃO: o sorteio usava números fixos (39, 55, 90, 110) como se o
        // orc tivesse sempre esse tamanho. Agora iniciamos o orc primeiro
        // para descobrir o tamanho REAL dele e sortear dentro da arena.
        IniciarInimigo(&inimigos[i], spriteOrc, (Vector2){ 0, 0 }, nivel);
        Vector2 tam = inimigos[i].tamanho;

        Vector2 pos;

        // Sorteia de novo enquanto o orc nascer perto demais do player
        do {
            pos = (Vector2){
                (float)GetRandomValue((int)arena->limites.x, (int)(arena->limites.x + arena->limites.width  - tam.x)),
                (float)GetRandomValue((int)arena->limites.y, (int)(arena->limites.y + arena->limites.height - tam.y))
            };
        } while (Vector2Distance((Vector2){ pos.x + tam.x / 2.0f, pos.y + tam.y / 2.0f }, centroPlayer) < DISTANCIA_MINIMA_SPAWN);

        // Inicia de novo já na posição final (isso também vira o orc para o centro)
        IniciarInimigo(&inimigos[i], spriteOrc, pos, nivel);
    }

    int pontosAtuais = 0;
    float tempoAtivo = 0.0f;

    while (!WindowShouldClose())
    {
        // Lê o tempo UMA vez por frame e usa em tudo
        float dt = GetFrameTime();

        AtualizarPlayer(player, arena);

        for (int i = 0; i < TOTAL_INIMIGOS; i++) {
            AtualizarInimigo(&inimigos[i], dt);
        }

        if (IsKeyPressed(KEY_SPACE)) {
            Vector2 centroAtaque = {
                player->posicao.x + player->tamanho.x / 2.0f,
                player->posicao.y + player->tamanho.y / 2.0f
            };

            for (int i = 0; i < TOTAL_INIMIGOS; i++) {
                if (InimigoPertoDe(&inimigos[i], centroAtaque, ALCANCE_ATAQUE)) {
                    bool matou = DanoInimigo(&inimigos[i], DANO_ATAQUE);
                    if (matou) pontosAtuais += PONTOS_POR_ORC;
                }
            }
        }

        tempoAtivo += dt;

        BeginDrawing();

            ClearBackground(BLACK);
            DesenharArena(arena);

            for (int i = 0; i < TOTAL_INIMIGOS; i++) {
                DesenharInimigo(&inimigos[i]);
            }

            DesenharPlayer(player);

            DesenharUI(pontosAtuais, tempoAtivo);

        EndDrawing();
    }

    FinalizarPlayer(player);
    FinalizarArena(arena);
    DescarregarSpriteOrc(spriteOrc);

    free(player);
    free(arena);

    CloseWindow();

    return 0;
}
