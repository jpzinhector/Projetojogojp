#ifndef ONDAS_H
#define ONDAS_H

#include "raylib.h"
#include "fila_ondas.h"
#include "lista_inimigos.h"

#define TOTAL_ONDAS 5

void CarregarOndas(FilaOndas *fila);

void GerarOnda(ListaInimigos *lista, Onda onda, Texture2D sprite, Rectangle limites, Vector2 centroAlvo);

#endif
