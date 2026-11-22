#include "lista_inimigos.h"
#include <stdlib.h>

void ListaIniciar(ListaInimigos *lista)
{
    lista->inicio = NULL;
    lista->tamanho = 0;
}

bool ListaInserir(ListaInimigos *lista, Inimigo inimigo)
{
    NoInimigo *novo = malloc(sizeof(NoInimigo));
    if (novo == NULL) return false;

    novo->inimigo = inimigo;
    novo->proximo = lista->inicio;
    lista->inicio = novo;
    lista->tamanho++;
    return true;
}

int ListaRemoverMortos(ListaInimigos *lista)
{
    int removidos = 0;
    NoInimigo **cursor = &lista->inicio;

    while (*cursor != NULL) {
        NoInimigo *atual = *cursor;
        if (atual->inimigo.vida <= 0 && atual->inimigo.tempoMorte <= 0.0f
            && atual->inimigo.tempoTextoDano <= 0.0f) {
            *cursor = atual->proximo;
            free(atual);
            lista->tamanho--;
            removidos++;
        } else {
            cursor = &atual->proximo;
        }
    }
    return removidos;
}

int ListaContarVivos(const ListaInimigos *lista)
{
    int vivos = 0;
    for (const NoInimigo *no = lista->inicio; no != NULL; no = no->proximo) {
        if (no->inimigo.vida > 0) vivos++;
    }
    return vivos;
}

bool ListaVazia(const ListaInimigos *lista)
{
    return lista->inicio == NULL;
}

void ListaLimpar(ListaInimigos *lista)
{
    NoInimigo *atual = lista->inicio;
    while (atual != NULL) {
        NoInimigo *proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    lista->inicio = NULL;
    lista->tamanho = 0;
}
