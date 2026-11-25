#ifndef LISTA_ITENS_H
#define LISTA_ITENS_H

#include "raylib.h"
#include <stdbool.h>
#include "pilha_pocoes.h"

typedef struct {
    Vector2 posicao;
    Pocao pocao;
    float tempoRestante;
    float fase;
} Item;

typedef struct NoItem {
    Item item;
    struct NoItem *proximo;
} NoItem;

typedef struct {
    NoItem *inicio;
    int tamanho;
} ListaItens;

void ListaItensIniciar(ListaItens *lista);
bool ListaItensInserir(ListaItens *lista, Vector2 posicao, Pocao pocao);
void ListaItensAtualizar(ListaItens *lista, float dt);
int ListaItensColetar(ListaItens *lista, Vector2 alvo, float raio, PilhaPocoes *inventario);
void ListaItensLimpar(ListaItens *lista);
void DesenharItens(const ListaItens *lista, Texture2D pocoes);

#endif
