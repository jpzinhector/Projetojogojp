#include "lista_itens.h"
#include <stdlib.h>
#include <math.h>

#define DURACAO_ITEM   14.0f
#define PISCAR_ITEM    4.0f
#define LARGURA_FRAME  64

void ListaItensIniciar(ListaItens *lista)
{
    lista->inicio = NULL;
    lista->tamanho = 0;
}

bool ListaItensInserir(ListaItens *lista, Vector2 posicao, Pocao pocao)
{
    NoItem *novo = malloc(sizeof(NoItem));
    if (novo == NULL) return false;

    novo->item.posicao = posicao;
    novo->item.pocao = pocao;
    novo->item.tempoRestante = DURACAO_ITEM;
    novo->item.fase = (float)GetRandomValue(0, 628) / 100.0f;
    novo->proximo = lista->inicio;
    lista->inicio = novo;
    lista->tamanho++;
    return true;
}

void ListaItensAtualizar(ListaItens *lista, float dt)
{
    NoItem **cursor = &lista->inicio;

    while (*cursor != NULL) {
        NoItem *atual = *cursor;
        atual->item.tempoRestante -= dt;
        atual->item.fase += dt * 4.0f;

        if (atual->item.tempoRestante <= 0.0f) {
            *cursor = atual->proximo;
            free(atual);
            lista->tamanho--;
        } else {
            cursor = &atual->proximo;
        }
    }
}

int ListaItensColetar(ListaItens *lista, Vector2 alvo, float raio, PilhaPocoes *inventario)
{
    int coletados = 0;
    NoItem **cursor = &lista->inicio;

    while (*cursor != NULL) {
        NoItem *atual = *cursor;
        float dx = atual->item.posicao.x - alvo.x;
        float dy = atual->item.posicao.y - alvo.y;

        if (sqrtf(dx * dx + dy * dy) <= raio && PilhaPocoesEmpilhar(inventario, atual->item.pocao)) {
            *cursor = atual->proximo;
            free(atual);
            lista->tamanho--;
            coletados++;
        } else {
            cursor = &atual->proximo;
        }
    }
    return coletados;
}

void ListaItensLimpar(ListaItens *lista)
{
    NoItem *atual = lista->inicio;
    while (atual != NULL) {
        NoItem *proximo = atual->proximo;
        free(atual);
        atual = proximo;
    }
    lista->inicio = NULL;
    lista->tamanho = 0;
}

void DesenharItens(const ListaItens *lista, Texture2D pocoes)
{
    for (const NoItem *no = lista->inicio; no != NULL; no = no->proximo) {
        const Item *item = &no->item;

        if (item->tempoRestante < PISCAR_ITEM && ((int)(item->tempoRestante * 6.0f) % 2) == 0) continue;

        float flutuar = sinf(item->fase) * 4.0f;
        float escala = (item->pocao.tipo == POCAO_GRANDE) ? 0.95f : 0.8f;
        float lado = LARGURA_FRAME * escala;

        Rectangle origem = { (float)(item->pocao.tipo * LARGURA_FRAME), 0.0f, (float)LARGURA_FRAME, (float)pocoes.height };
        Rectangle destino = { item->posicao.x, item->posicao.y - flutuar, lado, lado * (float)pocoes.height / (float)LARGURA_FRAME };

        float brilho = 0.5f + 0.5f * sinf(item->fase * 0.7f);
        DrawCircleGradient((Vector2){ item->posicao.x, item->posicao.y - lado * 0.4f }, lado * 0.55f,
                           (Color){ 255, 240, 160, (unsigned char)(40 + 50 * brilho) }, (Color){ 255, 240, 160, 0 });
        DrawTexturePro(pocoes, origem, destino, (Vector2){ lado / 2.0f, destino.height }, 0.0f, WHITE);
    }
}
