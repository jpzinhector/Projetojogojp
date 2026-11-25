#include "lista_projeteis.h"
#include <stdlib.h>
#include <math.h>

#define MARGEM_FORA_ARENA 60.0f
#define COMPRIMENTO_FLECHA 50.0f
#define ESPESSURA_FLECHA   9.0f

void ListaProjeteisIniciar(ListaProjeteis *lista)
{
    lista->inicio = NULL;
    lista->tamanho = 0;
}

bool ListaProjeteisInserir(ListaProjeteis *lista, Vector2 posicao, Vector2 velocidade, int dano)
{
    NoProjetil *novo = malloc(sizeof(NoProjetil));
    if (novo == NULL) return false;

    novo->projetil.posicao = posicao;
    novo->projetil.velocidade = velocidade;
    novo->projetil.angulo = atan2f(velocidade.y, velocidade.x);
    novo->projetil.dano = dano;
    novo->proximo = lista->inicio;
    lista->inicio = novo;
    lista->tamanho++;
    return true;
}

static bool ForaDaArena(Vector2 p, Rectangle limites)
{
    return p.x < limites.x - MARGEM_FORA_ARENA
        || p.x > limites.x + limites.width + MARGEM_FORA_ARENA
        || p.y < limites.y - MARGEM_FORA_ARENA
        || p.y > limites.y + limites.height + MARGEM_FORA_ARENA;
}

void ListaProjeteisAtualizar(ListaProjeteis *lista, float dt, Rectangle limites)
{
    NoProjetil **cursor = &lista->inicio;

    while (*cursor != NULL) {
        NoProjetil *atual = *cursor;
        atual->projetil.posicao.x += atual->projetil.velocidade.x * dt;
        atual->projetil.posicao.y += atual->projetil.velocidade.y * dt;

        if (ForaDaArena(atual->projetil.posicao, limites)) {
            *cursor = atual->proximo;
            free(atual);
            lista->tamanho--;
        } else {
            cursor = &atual->proximo;
        }
    }
}

bool ListaProjeteisColidir(ListaProjeteis *lista, Rectangle alvo, int *dano)
{
    bool acertou = false;
    NoProjetil **cursor = &lista->inicio;

    while (*cursor != NULL) {
        NoProjetil *atual = *cursor;

        if (CheckCollisionPointRec(atual->projetil.posicao, alvo)) {
            if (!acertou && dano != NULL) *dano = atual->projetil.dano;
            acertou = true;
            *cursor = atual->proximo;
            free(atual);
            lista->tamanho--;
        } else {
            cursor = &atual->proximo;
        }
    }
    return acertou;
}

void ListaProjeteisLimpar(ListaProjeteis *lista)
{
    NoProjetil *atual = lista->inicio;
    while (atual != NULL) {
        NoProjetil *proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    lista->inicio = NULL;
    lista->tamanho = 0;
}

void DesenharProjeteis(const ListaProjeteis *lista, Texture2D flecha)
{
    Rectangle origem = { 0.0f, 0.0f, (float)flecha.width, (float)flecha.height };

    for (const NoProjetil *no = lista->inicio; no != NULL; no = no->proximo) {
        const Projetil *p = &no->projetil;
        Rectangle destino = { p->posicao.x, p->posicao.y, COMPRIMENTO_FLECHA, ESPESSURA_FLECHA };
        DrawEllipse((int)p->posicao.x, (int)p->posicao.y + 26, 12, 4, (Color){ 0, 0, 0, 55 });
        DrawTexturePro(flecha, origem, destino, (Vector2){ COMPRIMENTO_FLECHA, ESPESSURA_FLECHA / 2.0f },
                       p->angulo * RAD2DEG, WHITE);
    }
}
