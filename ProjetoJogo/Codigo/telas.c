#include "telas.h"
#include <stddef.h>
#include "ui.h"
#include "config.h"

#define OPCOES_MENU  3
#define OPCOES_PAUSA 3

static const char *const TEXTOS_MENU[OPCOES_MENU]   = { "JOGAR", "CONTROLES", "SAIR" };
static const char *const TEXTOS_PAUSA[OPCOES_PAUSA] = { "CONTINUAR", "CONTROLES", "MENU PRINCIPAL" };

void IniciarTelas(Telas *telas)
{
    PilhaTelasIniciar(&telas->pilha);
    PilhaTelasEmpilhar(&telas->pilha, TELA_MENU);
    telas->opcaoMenu = 0;
    telas->opcaoPausa = 0;
    telas->sair = false;
}

void FinalizarTelas(Telas *telas)
{
    PilhaTelasLimpar(&telas->pilha);
}

static int MoverSelecao(int atual, int total, const Entrada *entrada)
{
    if (entrada->cima)  atual = (atual + total - 1) % total;
    if (entrada->baixo) atual = (atual + 1) % total;
    return atual;
}

static void AtualizarMenu(Telas *telas, Partida *partida, const Entrada *entrada)
{
    telas->opcaoMenu = MoverSelecao(telas->opcaoMenu, OPCOES_MENU, entrada);

    if (!entrada->confirmar) return;

    switch (telas->opcaoMenu) {
        case 0:
            ReiniciarPartida(partida);
            PilhaTelasEmpilhar(&telas->pilha, TELA_JOGO);
            break;
        case 1:
            PilhaTelasEmpilhar(&telas->pilha, TELA_CONTROLES);
            break;
        default:
            telas->sair = true;
            break;
    }
}

static void AtualizarControles(Telas *telas, const Entrada *entrada)
{
    if (entrada->confirmar || entrada->voltar) {
        PilhaTelasDesempilhar(&telas->pilha, NULL);
    }
}

static void AtualizarPausa(Telas *telas, const Entrada *entrada)
{
    telas->opcaoPausa = MoverSelecao(telas->opcaoPausa, OPCOES_PAUSA, entrada);

    if (entrada->pausar) {
        PilhaTelasDesempilhar(&telas->pilha, NULL);
        telas->opcaoPausa = 0;
        return;
    }

    if (!entrada->confirmar) return;

    switch (telas->opcaoPausa) {
        case 0:
            PilhaTelasDesempilhar(&telas->pilha, NULL);
            break;
        case 1:
            PilhaTelasEmpilhar(&telas->pilha, TELA_CONTROLES);
            break;
        default:
            PilhaTelasDesempilhar(&telas->pilha, NULL);
            PilhaTelasDesempilhar(&telas->pilha, NULL);
            break;
    }

    if (PilhaTelasTopo(&telas->pilha) != TELA_PAUSA) telas->opcaoPausa = 0;
}

static void AtualizarJogo(Telas *telas, Partida *partida, const Entrada *entrada, float dt)
{
    if (partida->estado != ESTADO_JOGANDO) {
        if (entrada->voltar) {
            PilhaTelasDesempilhar(&telas->pilha, NULL);
        } else if (entrada->confirmar) {
            ReiniciarPartida(partida);
        }
        return;
    }

    if (entrada->pausar) {
        telas->opcaoPausa = 0;
        PilhaTelasEmpilhar(&telas->pilha, TELA_PAUSA);
        return;
    }

    AtualizarPartida(partida, entrada, dt);
}

void AtualizarTelas(Telas *telas, Partida *partida, const Entrada *entrada, float dt)
{
    switch (PilhaTelasTopo(&telas->pilha)) {
        case TELA_MENU:      AtualizarMenu(telas, partida, entrada);          break;
        case TELA_CONTROLES: AtualizarControles(telas, entrada);              break;
        case TELA_PAUSA:     AtualizarPausa(telas, entrada);                  break;
        case TELA_JOGO:      AtualizarJogo(telas, partida, entrada, dt);      break;
        default:             telas->sair = true;                              break;
    }
}

static void DesenharMenuPrincipal(const Telas *telas, const Partida *partida)
{
    DesenharArena(&partida->arena);
    DesenharEscurecer(150);

    float altura = 300.0f;
    Rectangle orc = { 70.0f, 400.0f, altura * partida->sprites.chefe.width / (float)partida->sprites.chefe.height, altura };
    Rectangle heroi = { 0 };
    heroi.height = 240.0f;
    heroi.width = heroi.height * (partida->player.larguraFrame / partida->player.alturaFrame);
    heroi.x = LARGURA_JANELA - 90.0f - heroi.width;
    heroi.y = 460.0f;

    Rectangle origemOrc = { 0, 0, (float)partida->sprites.chefe.width, (float)partida->sprites.chefe.height };
    DrawTexturePro(partida->sprites.chefe, origemOrc, orc, (Vector2){ 0, 0 }, 0.0f, WHITE);

    Rectangle origemHeroi = { 0, 0, partida->player.larguraFrame, partida->player.alturaFrame };
    origemHeroi.width = -origemHeroi.width;
    DrawTexturePro(partida->player.sprite, origemHeroi, heroi, (Vector2){ 0, 0 }, 0.0f, WHITE);

    DesenharTituloJogo(110);
    DesenharMenuOpcoes(NULL, NULL, TEXTOS_MENU, OPCOES_MENU, telas->opcaoMenu, 300);
    DrawText("W/S ou SETAS: escolher    ENTER: confirmar", (LARGURA_JANELA - MeasureText("W/S ou SETAS: escolher    ENTER: confirmar", 18)) / 2, ALTURA_JANELA - 50, 18, (Color){ 170, 200, 230, 255 });
}

static void DesenharPausa(const Telas *telas)
{
    DesenharEscurecer(150);
    DesenharMenuOpcoes("PAUSADO", NULL, TEXTOS_PAUSA, OPCOES_PAUSA, telas->opcaoPausa, 200);
}

void DesenharTelas(const Telas *telas, const Partida *partida)
{
    int base = 0;
    int profundidade = 0;

    for (int i = 0; PilhaTelasEm(&telas->pilha, i) != TELA_NENHUMA; i++) {
        Tela t = PilhaTelasEm(&telas->pilha, i);
        profundidade = i;
        if (t == TELA_MENU || t == TELA_JOGO) {
            base = i;
            break;
        }
    }
    if (PilhaTelasEm(&telas->pilha, base) == TELA_NENHUMA) base = profundidade;

    for (int i = base; i >= 0; i--) {
        switch (PilhaTelasEm(&telas->pilha, i)) {
            case TELA_MENU:      DesenharMenuPrincipal(telas, partida); break;
            case TELA_JOGO:      DesenharPartida(partida);              break;
            case TELA_PAUSA:     DesenharPausa(telas);                  break;
            case TELA_CONTROLES: DesenharTelaControles();               break;
            default: break;
        }
    }
}
