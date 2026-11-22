#ifndef LISTA_INIMIGOS_H
#define LISTA_INIMIGOS_H

#include <stdbool.h>
#include "inimigo.h"

typedef struct NoInimigo {
    Inimigo inimigo;
    struct NoInimigo *proximo;
} NoInimigo;

typedef struct {
    NoInimigo *inicio;
    int tamanho;
} ListaInimigos;

void ListaIniciar(ListaInimigos *lista);
bool ListaInserir(ListaInimigos *lista, Inimigo inimigo);
int  ListaRemoverMortos(ListaInimigos *lista);
int  ListaContarVivos(const ListaInimigos *lista);
bool ListaVazia(const ListaInimigos *lista);
void ListaLimpar(ListaInimigos *lista);

#endif
