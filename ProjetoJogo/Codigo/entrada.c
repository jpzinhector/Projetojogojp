#include "entrada.h"
#include "raylib.h"

Entrada LerEntrada(void)
{
    Entrada e = { 0 };

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    e.moverY -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  e.moverY += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  e.moverX -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) e.moverX += 1.0f;

    e.atacar    = IsKeyPressed(KEY_SPACE);
    e.esquivar  = IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT);
    e.pocao     = IsKeyPressed(KEY_Q);
    e.pausar    = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P);
    e.cima      = IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
    e.baixo     = IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN);
    e.confirmar = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    e.voltar    = IsKeyPressed(KEY_ESCAPE);

    return e;
}
