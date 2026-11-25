#ifndef PILHA_TELAS_H
#define PILHA_TELAS_H

#include <stdbool.h>

typedef enum {
    TELA_NENHUMA,
    TELA_MENU,
    TELA_CONTROLES,
    TELA_JOGO,
    TELA_PAUSA
} Tela;

typedef struct NoTela {
    Tela tela;
    struct NoTela *proximo;
} NoTela;

typedef struct {
    NoTela *topo;
    int tamanho;
} PilhaTelas;

void PilhaTelasIniciar(PilhaTelas *pilha);
bool PilhaTelasEmpilhar(PilhaTelas *pilha, Tela tela);
bool PilhaTelasDesempilhar(PilhaTelas *pilha, Tela *removida);
Tela PilhaTelasTopo(const PilhaTelas *pilha);
Tela PilhaTelasEm(const PilhaTelas *pilha, int profundidade);
bool PilhaTelasVazia(const PilhaTelas *pilha);
void PilhaTelasLimpar(PilhaTelas *pilha);

#endif
