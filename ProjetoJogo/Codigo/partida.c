#include "partida.h"
#include "ondas.h"
#include "ui.h"
#include <math.h>
#include <stddef.h>

#define ALCANCE_ATAQUE        120.0f
#define DANO_ATAQUE           10
#define MARGEM_ATRAS_ATAQUE   20.0f
#define ESPERA_ENTRE_ONDAS    2.0f
#define ESPERA_PRIMEIRA_ONDA  1.6f
#define VELOCIDADE_FLECHA     430.0f
#define RAIO_COLETA           46.0f

void IniciarPartida(Partida *partida)
{
    IniciarArena(&partida->arena);
    IniciarPlayer(&partida->player);
    IniciarArma(&partida->arma);
    partida->sprites = CarregarSpritesInimigos();
    partida->pocoes = LoadTexture("assets/pocoes.png");

    ListaIniciar(&partida->inimigos);
    ListaItensIniciar(&partida->itens);
    ListaProjeteisIniciar(&partida->projeteis);
    FilaIniciar(&partida->ondas);
    PilhaPocoesIniciar(&partida->inventario);

    ReiniciarPartida(partida);
}

void ReiniciarPartida(Partida *partida)
{
    ListaLimpar(&partida->inimigos);
    ListaItensLimpar(&partida->itens);
    ListaProjeteisLimpar(&partida->projeteis);
    FilaLimpar(&partida->ondas);
    PilhaPocoesLimpar(&partida->inventario);
    CarregarOndas(&partida->ondas);
    ResetarPlayer(&partida->player);
    ResetarArma(&partida->arma);

    partida->estado     = ESTADO_JOGANDO;
    partida->pontos     = 0;
    partida->tempo      = 0.0f;
    partida->ondaAtual  = 0;
    partida->esperaOnda = ESPERA_PRIMEIRA_ONDA;
}

void FinalizarPartida(Partida *partida)
{
    ListaLimpar(&partida->inimigos);
    ListaItensLimpar(&partida->itens);
    ListaProjeteisLimpar(&partida->projeteis);
    FilaLimpar(&partida->ondas);

    FinalizarArma(&partida->arma);
    FinalizarPlayer(&partida->player);
    FinalizarArena(&partida->arena);
    DescarregarSpritesInimigos(partida->sprites);
    UnloadTexture(partida->pocoes);
}

static void TentarAtacar(Partida *partida)
{
    Vector2 centro = CentroPlayer(&partida->player);
    bool direita = partida->player.olhandoDireita;
    float menorDistancia = ALCANCE_ATAQUE + 1.0f;

    for (const NoInimigo *no = partida->inimigos.inicio; no != NULL; no = no->proximo) {
        if (!InimigoPertoDe(&no->inimigo, centro, ALCANCE_ATAQUE)) continue;

        float dx = (no->inimigo.posicao.x + no->inimigo.tamanho.x / 2.0f) - centro.x;
        float dy = (no->inimigo.posicao.y + no->inimigo.tamanho.y / 2.0f) - centro.y;
        float distancia = sqrtf(dx * dx + dy * dy);

        if (distancia < menorDistancia) {
            menorDistancia = distancia;
            direita = (dx >= 0.0f);
        }
    }

    IniciarGolpe(&partida->arma, direita);
}

static void SoltarItens(Partida *partida, const Inimigo *inimigo)
{
    Vector2 pe = PeDoInimigo(inimigo);

    if (inimigo->tipo == INIMIGO_CHEFE) {
        ListaItensInserir(&partida->itens, (Vector2){ pe.x - 28.0f, pe.y }, CriarPocao(POCAO_GRANDE));
        ListaItensInserir(&partida->itens, (Vector2){ pe.x + 28.0f, pe.y }, CriarPocao(POCAO_GRANDE));
        return;
    }

    if (GetRandomValue(0, 99) < ChanceDropInimigo(inimigo)) {
        ListaItensInserir(&partida->itens, pe, CriarPocao(POCAO_PEQUENA));
    }
}

static void RegistrarMorte(Partida *partida, const Inimigo *inimigo)
{
    partida->pontos += PontosInimigo(inimigo);
    SoltarItens(partida, inimigo);
}

static void AplicarGolpe(Partida *partida)
{
    Vector2 centro = CentroPlayer(&partida->player);

    for (NoInimigo *no = partida->inimigos.inicio; no != NULL; no = no->proximo) {
        if (!InimigoPertoDe(&no->inimigo, centro, ALCANCE_ATAQUE)) continue;

        float dx = (no->inimigo.posicao.x + no->inimigo.tamanho.x / 2.0f) - centro.x;
        bool naFrente = partida->arma.direita ? (dx >= -MARGEM_ATRAS_ATAQUE) : (dx <= MARGEM_ATRAS_ATAQUE);
        if (!naFrente) continue;

        if (GolpearInimigo(&no->inimigo, DANO_ATAQUE, centro)) {
            RegistrarMorte(partida, &no->inimigo);
        }
    }
}

static void UsarPocao(Partida *partida)
{
    if (PilhaPocoesVazia(&partida->inventario)) return;
    if (partida->player.vida >= partida->player.vidaMaxima || partida->player.vida <= 0) return;

    Pocao pocao;
    if (PilhaPocoesDesempilhar(&partida->inventario, &pocao)) {
        CurarPlayer(&partida->player, pocao.cura);
    }
}

static void ProcessarEvento(Partida *partida, const Inimigo *inimigo, EventoInimigo evento)
{
    if (evento == EVENTO_GOLPE_CORPO) {
        if (GolpeAtingeAlvo(inimigo, PePlayer(&partida->player))) {
            if (DanoPlayer(&partida->player, DanoCorpoInimigo(inimigo))) {
                partida->estado = ESTADO_DERROTA;
            }
        }
    } else if (evento == EVENTO_DISPARO) {
        Vector2 origem = PontaDoArco(inimigo);
        Vector2 velocidade = { cosf(inimigo->anguloMira) * VELOCIDADE_FLECHA, sinf(inimigo->anguloMira) * VELOCIDADE_FLECHA };
        ListaProjeteisInserir(&partida->projeteis, origem, velocidade, DanoFlechaInimigo(inimigo));
    }
}

static void AtualizarOndas(Partida *partida, float dt)
{
    if (!ListaVazia(&partida->inimigos)) return;

    if (FilaVazia(&partida->ondas)) {
        partida->estado = ESTADO_VITORIA;
        return;
    }

    partida->esperaOnda -= dt;
    if (partida->esperaOnda > 0.0f) return;

    Onda proxima;
    if (FilaDesenfileirar(&partida->ondas, &proxima)) {
        GerarOnda(&partida->inimigos, proxima, &partida->sprites, partida->arena.limites, CentroPlayer(&partida->player));
        partida->ondaAtual++;
    }
    partida->esperaOnda = ESPERA_ENTRE_ONDAS;
}

void AtualizarPartida(Partida *partida, const Entrada *entrada, float dt)
{
    if (partida->estado != ESTADO_JOGANDO) return;

    Player *player = &partida->player;
    Arma *arma = &partida->arma;

    partida->tempo += dt;

    AtualizarPlayer(player, &partida->arena, entrada, dt);

    if (entrada->atacar) TentarAtacar(partida);
    if (entrada->pocao) UsarPocao(partida);

    AtualizarArma(arma, dt);
    if (arma->atacando) player->olhandoDireita = arma->direita;
    if (ConsumirImpacto(arma)) AplicarGolpe(partida);

    Vector2 peAlvo = PePlayer(player);

    for (NoInimigo *no = partida->inimigos.inicio; no != NULL; no = no->proximo) {
        AtualizarInimigo(&no->inimigo, dt);
        EventoInimigo evento = AtualizarIAInimigo(&no->inimigo, peAlvo, dt, partida->arena.limites);
        ProcessarEvento(partida, &no->inimigo, evento);
    }

    for (NoInimigo *a = partida->inimigos.inicio; a != NULL; a = a->proximo) {
        for (NoInimigo *b = a->proximo; b != NULL; b = b->proximo) {
            AfastarInimigos(&a->inimigo, &b->inimigo, partida->arena.limites);
        }
    }

    ListaProjeteisAtualizar(&partida->projeteis, dt, partida->arena.limites);
    if (!PlayerInvulneravel(player)) {
        int dano = 0;
        if (ListaProjeteisColidir(&partida->projeteis, CorpoPlayer(player), &dano)) {
            if (DanoPlayer(player, dano)) partida->estado = ESTADO_DERROTA;
        }
    }

    ListaItensAtualizar(&partida->itens, dt);
    ListaItensColetar(&partida->itens, CentroPlayer(player), RAIO_COLETA, &partida->inventario);

    ListaRemoverMortos(&partida->inimigos);

    if (partida->estado == ESTADO_DERROTA) return;

    AtualizarOndas(partida, dt);
}

static const Inimigo *BuscarChefe(const ListaInimigos *lista)
{
    for (const NoInimigo *no = lista->inicio; no != NULL; no = no->proximo) {
        if (no->inimigo.tipo == INIMIGO_CHEFE && no->inimigo.vida > 0) return &no->inimigo;
    }
    return NULL;
}

void DesenharPartida(const Partida *partida)
{
    DesenharArena(&partida->arena);
    DesenharItens(&partida->itens, partida->pocoes);

    for (const NoInimigo *no = partida->inimigos.inicio; no != NULL; no = no->proximo) {
        DesenharTelegrafoInimigo(&no->inimigo);
    }

    for (const NoInimigo *no = partida->inimigos.inicio; no != NULL; no = no->proximo) {
        DesenharInimigo(&no->inimigo);
    }

    DesenharPlayer(&partida->player);
    DesenharArma(&partida->arma, &partida->player);
    DesenharProjeteis(&partida->projeteis, partida->sprites.flecha);

    DesenharUI(partida->pontos, partida->tempo);
    DesenharPainelJogador(&partida->player, partida->ondaAtual, TOTAL_ONDAS, ListaContarVivos(&partida->inimigos));
    DesenharPainelEquipamento(&partida->player, &partida->inventario, partida->pocoes);

    const Inimigo *chefe = BuscarChefe(&partida->inimigos);
    if (chefe != NULL) DesenharBarraChefe(chefe);

    if (partida->estado == ESTADO_JOGANDO && ListaVazia(&partida->inimigos) && !FilaVazia(&partida->ondas)) {
        Onda proxima = { 0 };
        FilaEspiar(&partida->ondas, &proxima);
        DesenharAvisoOnda(partida->ondaAtual + 1, OndaTemChefe(proxima));
    }

    if (partida->estado == ESTADO_VITORIA) {
        DesenharTelaFinal(true, partida->pontos, partida->tempo);
    } else if (partida->estado == ESTADO_DERROTA) {
        DesenharTelaFinal(false, partida->pontos, partida->tempo);
    }
}
