#include "fila_ondas.h"
#include <stdlib.h>

void FilaIniciar(FilaOndas *fila)
{
    fila->inicio = NULL;
    fila->fim = NULL;
    fila->tamanho = 0;
}

bool FilaEnfileirar(FilaOndas *fila, Onda onda)
{
    NoOnda *novo = malloc(sizeof(NoOnda));
    if (novo == NULL) return false;

    novo->onda = onda;
    novo->proximo = NULL;

    if (fila->fim == NULL) {
        fila->inicio = novo;
    } else {
        fila->fim->proximo = novo;
    }
    fila->fim = novo;
    fila->tamanho++;
    return true;
}

bool FilaDesenfileirar(FilaOndas *fila, Onda *saida)
{
    if (fila->inicio == NULL) return false;

    NoOnda *removido = fila->inicio;
    if (saida != NULL) *saida = removido->onda;

    fila->inicio = removido->proximo;
    if (fila->inicio == NULL) fila->fim = NULL;

    free(removido);
    fila->tamanho--;
    return true;
}

bool FilaEspiar(const FilaOndas *fila, Onda *saida)
{
    if (fila->inicio == NULL) return false;
    if (saida != NULL) *saida = fila->inicio->onda;
    return true;
}

bool FilaVazia(const FilaOndas *fila)
{
    return fila->inicio == NULL;
}

void FilaLimpar(FilaOndas *fila)
{
    while (FilaDesenfileirar(fila, NULL)) {
    }
}
