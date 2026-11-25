#ifndef PILHA_POCOES_H
#define PILHA_POCOES_H

#include <stdbool.h>

#define CAPACIDADE_POCOES 5

typedef enum {
    POCAO_PEQUENA,
    POCAO_GRANDE
} TipoPocao;

typedef struct {
    TipoPocao tipo;
    int cura;
} Pocao;

typedef struct {
    Pocao itens[CAPACIDADE_POCOES];
    int topo;
} PilhaPocoes;

Pocao CriarPocao(TipoPocao tipo);
void PilhaPocoesIniciar(PilhaPocoes *pilha);
bool PilhaPocoesEmpilhar(PilhaPocoes *pilha, Pocao pocao);
bool PilhaPocoesDesempilhar(PilhaPocoes *pilha, Pocao *removida);
bool PilhaPocoesCheia(const PilhaPocoes *pilha);
bool PilhaPocoesVazia(const PilhaPocoes *pilha);
int PilhaPocoesTamanho(const PilhaPocoes *pilha);
void PilhaPocoesLimpar(PilhaPocoes *pilha);

#endif
