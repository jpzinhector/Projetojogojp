#ifndef PARTIDA_H
#define PARTIDA_H

#include "raylib.h"
#include <stdbool.h>
#include "entrada.h"
#include "arena.h"
#include "player.h"
#include "arma.h"
#include "inimigo.h"
#include "lista_inimigos.h"
#include "lista_itens.h"
#include "lista_projeteis.h"
#include "fila_ondas.h"
#include "pilha_pocoes.h"

typedef enum {
    ESTADO_JOGANDO,
    ESTADO_DERROTA,
    ESTADO_VITORIA
} EstadoPartida;

typedef struct {
    EstadoPartida estado;
    int pontos;
    float tempo;
    int ondaAtual;
    float esperaOnda;

    Arena arena;
    Player player;
    Arma arma;
    SpritesInimigos sprites;
    Texture2D pocoes;

    ListaInimigos inimigos;
    ListaItens itens;
    ListaProjeteis projeteis;
    FilaOndas ondas;
    PilhaPocoes inventario;
} Partida;

void IniciarPartida(Partida *partida);
void ReiniciarPartida(Partida *partida);
void AtualizarPartida(Partida *partida, const Entrada *entrada, float dt);
void DesenharPartida(const Partida *partida);
void FinalizarPartida(Partida *partida);

#endif
