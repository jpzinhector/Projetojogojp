#include "pilha_pocoes.h"
#include <stddef.h>

#define CURA_PEQUENA 25
#define CURA_GRANDE  50

Pocao CriarPocao(TipoPocao tipo)
{
    Pocao pocao;
    pocao.tipo = tipo;
    pocao.cura = (tipo == POCAO_GRANDE) ? CURA_GRANDE : CURA_PEQUENA;
    return pocao;
}

void PilhaPocoesIniciar(PilhaPocoes *pilha)
{
    pilha->topo = 0;
}

bool PilhaPocoesEmpilhar(PilhaPocoes *pilha, Pocao pocao)
{
    if (pilha->topo >= CAPACIDADE_POCOES) return false;
    pilha->itens[pilha->topo++] = pocao;
    return true;
}

bool PilhaPocoesDesempilhar(PilhaPocoes *pilha, Pocao *removida)
{
    if (pilha->topo == 0) return false;
    pilha->topo--;
    if (removida != NULL) *removida = pilha->itens[pilha->topo];
    return true;
}

bool PilhaPocoesCheia(const PilhaPocoes *pilha)
{
    return pilha->topo >= CAPACIDADE_POCOES;
}

bool PilhaPocoesVazia(const PilhaPocoes *pilha)
{
    return pilha->topo == 0;
}

int PilhaPocoesTamanho(const PilhaPocoes *pilha)
{
    return pilha->topo;
}

void PilhaPocoesLimpar(PilhaPocoes *pilha)
{
    pilha->topo = 0;
}
