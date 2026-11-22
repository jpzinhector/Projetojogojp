#include "ondas.h"
#include "raymath.h"

#define DISTANCIA_MINIMA_SPAWN 250.0f
#define TENTATIVAS_SPAWN       200

void CarregarOndas(FilaOndas *fila)
{
    const Onda ondas[TOTAL_ONDAS] = {
        { 3, 1 },
        { 4, 1 },
        { 4, 2 },
        { 5, 2 },
        { 6, 3 }
    };

    for (int i = 0; i < TOTAL_ONDAS; i++) {
        FilaEnfileirar(fila, ondas[i]);
    }
}

void GerarOnda(ListaInimigos *lista, Onda onda, Texture2D sprite, Rectangle limites, Vector2 centroAlvo)
{
    for (int i = 0; i < onda.quantidade; i++) {
        Inimigo inimigo;

        IniciarInimigo(&inimigo, sprite, (Vector2){ 0, 0 }, onda.nivel);
        Vector2 tam = inimigo.tamanho;

        Vector2 pos = { limites.x, limites.y };

        for (int t = 0; t < TENTATIVAS_SPAWN; t++) {
            pos = (Vector2){
                (float)GetRandomValue((int)limites.x, (int)(limites.x + limites.width  - tam.x)),
                (float)GetRandomValue((int)limites.y, (int)(limites.y + limites.height - tam.y))
            };

            Vector2 centro = { pos.x + tam.x / 2.0f, pos.y + tam.y / 2.0f };
            if (Vector2Distance(centro, centroAlvo) >= DISTANCIA_MINIMA_SPAWN) break;
        }

        IniciarInimigo(&inimigo, sprite, pos, onda.nivel);
        ListaInserir(lista, inimigo);
    }
}
