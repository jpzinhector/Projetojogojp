#include "ui.h"
#include "config.h"
#include <stdio.h>

static const Color COR_FUNDO   = { 3, 19, 38, 255 };
static const Color COR_BORDA   = { 77, 134, 184, 255 };
static const Color COR_MOLDURA = { 55, 100, 141, 255 };

static void DesenharPainel(float x, float y, float largura, float altura)
{
    Rectangle externo = { x, y, largura, altura };
    Rectangle interno = { x + 4, y + 4, largura - 8, altura - 8 };

    DrawRectangleRounded(externo, 0.2f, 8, COR_MOLDURA);
    DrawRectangleRoundedLines(externo, 0.2f, 9, COR_BORDA);
    DrawRectangleRounded(interno, 0.18f, 8, COR_FUNDO);
}

static void DesenharTextoCentralizado(const char *texto, int y, int tamanho, Color cor)
{
    int largura = MeasureText(texto, tamanho);
    DrawText(texto, (LARGURA_JANELA - largura) / 2, y, tamanho, cor);
}

void DesenharUI(int pontos, float tempoDecorrido)
{
    int minutos  = (int)tempoDecorrido / 60;
    int segundos = (int)tempoDecorrido % 60;

    char bufferPontos[32];
    char bufferTempo[32];

    snprintf(bufferPontos, sizeof(bufferPontos), "PONTOS: %04d", pontos);
    snprintf(bufferTempo, sizeof(bufferTempo), "TEMPO: %02d:%02d", minutos, segundos);

    float uiX = LARGURA_JANELA - 240.0f;
    float uiY = 0.0f;

    DesenharPainel(uiX, uiY, 230.0f, 100.0f);

    DrawText(bufferPontos, (int)(uiX + 30), (int)(uiY + 25), 20, WHITE);
    DrawText(bufferTempo,  (int)(uiX + 30), (int)(uiY + 60), 20, WHITE);
}

void DesenharPainelJogador(const Player *player, int onda, int totalOndas, int orcsRestantes)
{
    float uiX = 10.0f;
    float uiY = 0.0f;

    char bufferOnda[32];
    char bufferOrcs[32];

    snprintf(bufferOnda, sizeof(bufferOnda), "ONDA %d/%d", onda, totalOndas);
    snprintf(bufferOrcs, sizeof(bufferOrcs), "ORCS: %d", orcsRestantes);

    DesenharPainel(uiX, uiY, 230.0f, 100.0f);

    DrawText(bufferOnda, (int)(uiX + 30), (int)(uiY + 14), 20, WHITE);
    DrawText(bufferOrcs, (int)(uiX + 30), (int)(uiY + 40), 20, WHITE);

    float porcentagem = (float)player->vida / (float)player->vidaMaxima;
    if (porcentagem < 0.0f) porcentagem = 0.0f;

    float barraX = uiX + 82.0f;
    float barraY = uiY + 68.0f;
    float barraLargura = 120.0f;
    float barraAltura = 16.0f;

    DrawText("VIDA", (int)(uiX + 30), (int)(barraY), 16, WHITE);
    DrawRectangle((int)barraX - 1, (int)barraY - 1, (int)barraLargura + 2, (int)barraAltura + 2, BLACK);
    DrawRectangle((int)barraX, (int)barraY, (int)barraLargura, (int)barraAltura, (Color){ 90, 20, 20, 255 });
    DrawRectangle((int)barraX, (int)barraY, (int)(barraLargura * porcentagem), (int)barraAltura, (Color){ 60, 200, 80, 255 });
}

void DesenharAvisoOnda(int proximaOnda)
{
    char texto[32];
    snprintf(texto, sizeof(texto), "ONDA %d", proximaOnda);

    DesenharTextoCentralizado(texto, ALTURA_JANELA / 2 - 230, 56, WHITE);
    DesenharTextoCentralizado("PREPARE-SE!", ALTURA_JANELA / 2 - 165, 24, (Color){ 255, 220, 120, 255 });
}

void DesenharTelaFinal(bool venceu, int pontos, float tempoDecorrido)
{
    int minutos  = (int)tempoDecorrido / 60;
    int segundos = (int)tempoDecorrido % 60;

    char resumo[64];
    snprintf(resumo, sizeof(resumo), "PONTOS: %04d   TEMPO: %02d:%02d", pontos, minutos, segundos);

    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, (Color){ 0, 0, 0, 180 });

    if (venceu) {
        DesenharTextoCentralizado("VOCE VENCEU!", ALTURA_JANELA / 2 - 90, 56, (Color){ 120, 230, 120, 255 });
    } else {
        DesenharTextoCentralizado("FIM DE JOGO", ALTURA_JANELA / 2 - 90, 56, (Color){ 230, 80, 80, 255 });
    }

    DesenharTextoCentralizado(resumo, ALTURA_JANELA / 2 - 10, 24, WHITE);
    DesenharTextoCentralizado("ENTER: JOGAR DE NOVO", ALTURA_JANELA / 2 + 40, 22, (Color){ 255, 220, 120, 255 });
}
