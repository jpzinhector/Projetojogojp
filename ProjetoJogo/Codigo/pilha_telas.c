#include "pilha_telas.h"
#include <stdlib.h>

void PilhaTelasIniciar(PilhaTelas *pilha)
{
    pilha->topo = NULL;
    pilha->tamanho = 0;
}

bool PilhaTelasEmpilhar(PilhaTelas *pilha, Tela tela)
{
    NoTela *novo = malloc(sizeof(NoTela));
    if (novo == NULL) return false;

    novo->tela = tela;
    novo->proximo = pilha->topo;
    pilha->topo = novo;
    pilha->tamanho++;
    return true;
}

bool PilhaTelasDesempilhar(PilhaTelas *pilha, Tela *removida)
{
    if (pilha->topo == NULL) return false;

    NoTela *no = pilha->topo;
    if (removida != NULL) *removida = no->tela;
    pilha->topo = no->proximo;
    free(no);
    pilha->tamanho--;
    return true;
}

Tela PilhaTelasTopo(const PilhaTelas *pilha)
{
    return PilhaTelasEm(pilha, 0);
}

Tela PilhaTelasEm(const PilhaTelas *pilha, int profundidade)
{
    const NoTela *no = pilha->topo;
    while (no != NULL && profundidade > 0) {
        no = no->proximo;
        profundidade--;
    }
    return (no != NULL) ? no->tela : TELA_NENHUMA;
}

bool PilhaTelasVazia(const PilhaTelas *pilha)
{
    return pilha->topo == NULL;
}

void PilhaTelasLimpar(PilhaTelas *pilha)
{
    while (PilhaTelasDesempilhar(pilha, NULL)) { }
}
