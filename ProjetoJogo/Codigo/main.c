#include "raylib.h"
#include "config.h"
#include "entrada.h"
#include "partida.h"
#include "telas.h"

int main(void)
{
    InitWindow(LARGURA_JANELA, ALTURA_JANELA, TITULO_JANELA);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    SetWindowPosition(50, 50);

    static Partida partida;
    Telas telas;

    IniciarPartida(&partida);
    IniciarTelas(&telas);

    while (!WindowShouldClose() && !telas.sair)
    {
        Entrada entrada = LerEntrada();
        float dt = GetFrameTime();

        AtualizarTelas(&telas, &partida, &entrada, dt);

        BeginDrawing();

            ClearBackground(BLACK);
            DesenharTelas(&telas, &partida);

        EndDrawing();
    }

    FinalizarTelas(&telas);
    FinalizarPartida(&partida);

    CloseWindow();

    return 0;
}
