#include "ondas.h"
#include "raymath.h"

#define DISTANCIA_MINIMA_SPAWN 250.0f
#define TENTATIVAS_SPAWN       200

void CarregarOndas(FilaOndas *fila)
{
    const Onda ondas[TOTAL_ONDAS] = {
        { 3, 0, 0, 1 },
        { 3, 1, 0, 1 },
        { 3, 2, 0, 2 },
        { 4, 2, 0, 2 },
        { 3, 1, 1, 3 }
    };

    for (int i = 0; i < TOTAL_ONDAS; i++) {
        FilaEnfileirar(fila, ondas[i]);
    }
}

bool OndaTemChefe(Onda onda)
{
    return onda.chefes > 0;
}

static void GerarGrupo(ListaInimigos *lista, TipoInimigo tipo, int quantidade, int nivel,
                       const SpritesInimigos *sprites, Rectangle limites, Vector2 centroAlvo)
{
    for (int i = 0; i < quantidade; i++) {
        Inimigo inimigo;

        IniciarInimigo(&inimigo, tipo, sprites, (Vector2){ 0, 0 }, nivel);
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

        IniciarInimigo(&inimigo, tipo, sprites, pos, nivel);
        ListaInserir(lista, inimigo);
    }
}

void GerarOnda(ListaInimigos *lista, Onda onda, const SpritesInimigos *sprites, Rectangle limites, Vector2 centroAlvo)
{
    GerarGrupo(lista, INIMIGO_CHEFE, onda.chefes, onda.nivel, sprites, limites, centroAlvo);
    GerarGrupo(lista, INIMIGO_ORC, onda.orcs, onda.nivel, sprites, limites, centroAlvo);
    GerarGrupo(lista, INIMIGO_ARQUEIRO, onda.arqueiros, onda.nivel, sprites, limites, centroAlvo);
}
