#ifndef LISTA_PROJETEIS_H
#define LISTA_PROJETEIS_H

#include "raylib.h"
#include <stdbool.h>

typedef struct {
    Vector2 posicao;
    Vector2 velocidade;
    float angulo;
    int dano;
} Projetil;

typedef struct NoProjetil {
    Projetil projetil;
    struct NoProjetil *proximo;
} NoProjetil;

typedef struct {
    NoProjetil *inicio;
    int tamanho;
} ListaProjeteis;

void ListaProjeteisIniciar(ListaProjeteis *lista);
bool ListaProjeteisInserir(ListaProjeteis *lista, Vector2 posicao, Vector2 velocidade, int dano);
void ListaProjeteisAtualizar(ListaProjeteis *lista, float dt, Rectangle limites);
bool ListaProjeteisColidir(ListaProjeteis *lista, Rectangle alvo, int *dano);
void ListaProjeteisLimpar(ListaProjeteis *lista);
void DesenharProjeteis(const ListaProjeteis *lista, Texture2D flecha);

#endif
