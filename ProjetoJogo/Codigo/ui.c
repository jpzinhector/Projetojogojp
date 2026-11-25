#include "ui.h"
#include "config.h"
#include <stdio.h>

static const Color COR_FUNDO   = { 3, 19, 38, 255 };
static const Color COR_BORDA   = { 77, 134, 184, 255 };
static const Color COR_MOLDURA = { 55, 100, 141, 255 };
static const Color COR_DESTAQUE = { 255, 220, 120, 255 };

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

static void DesenharTextoContorno(const char *texto, int x, int y, int tamanho, Color cor)
{
    Color contorno = { 0, 0, 0, cor.a };
    DrawText(texto, x - 2, y, tamanho, contorno);
    DrawText(texto, x + 2, y, tamanho, contorno);
    DrawText(texto, x, y - 2, tamanho, contorno);
    DrawText(texto, x, y + 2, tamanho, contorno);
    DrawText(texto, x, y, tamanho, cor);
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

void DesenharPainelJogador(const Player *player, int onda, int totalOndas, int inimigosRestantes)
{
    float uiX = 10.0f;
    float uiY = 0.0f;

    char bufferOnda[32];
    char bufferInimigos[32];

    snprintf(bufferOnda, sizeof(bufferOnda), "ONDA %d/%d", onda, totalOndas);
    snprintf(bufferInimigos, sizeof(bufferInimigos), "INIMIGOS: %d", inimigosRestantes);

    DesenharPainel(uiX, uiY, 230.0f, 100.0f);

    DrawText(bufferOnda, (int)(uiX + 30), (int)(uiY + 14), 20, WHITE);
    DrawText(bufferInimigos, (int)(uiX + 30), (int)(uiY + 40), 20, WHITE);

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

void DesenharPainelEquipamento(const Player *player, const PilhaPocoes *inventario, Texture2D pocoes)
{
    float uiX = 10.0f;
    float uiY = 106.0f;

    DesenharPainel(uiX, uiY, 230.0f, 112.0f);

    float progresso = ProgressoEsquiva(player);
    bool pronta = progresso >= 1.0f;

    DrawText("ESQUIVA", (int)(uiX + 18), (int)(uiY + 15), 16, WHITE);
    DrawText("SHIFT", (int)(uiX + 170), (int)(uiY + 17), 12, (Color){ 150, 190, 230, 255 });
    DrawRectangle((int)(uiX + 18) - 1, (int)(uiY + 35) - 1, 194 + 2, 10 + 2, BLACK);
    DrawRectangle((int)(uiX + 18), (int)(uiY + 35), 194, 10, (Color){ 20, 40, 70, 255 });
    DrawRectangle((int)(uiX + 18), (int)(uiY + 35), (int)(194 * progresso), 10,
                  pronta ? (Color){ 110, 210, 255, 255 } : (Color){ 60, 110, 170, 255 });

    DrawText("POCOES", (int)(uiX + 18), (int)(uiY + 55), 16, WHITE);
    DrawText("Q", (int)(uiX + 196), (int)(uiY + 57), 14, (Color){ 150, 190, 230, 255 });

    int topo = PilhaPocoesTamanho(inventario);
    for (int i = 0; i < CAPACIDADE_POCOES; i++) {
        float sx = uiX + 18 + i * 39.0f;
        float sy = uiY + 76.0f;
        Rectangle casa = { sx, sy, 34.0f, 28.0f };
        bool ocupada = i < topo;
        bool nopo = ocupada && i == topo - 1;

        DrawRectangleRec(casa, (Color){ 10, 30, 56, 255 });
        DrawRectangleLinesEx(casa, nopo ? 2.0f : 1.0f, nopo ? COR_DESTAQUE : (Color){ 60, 100, 150, 255 });

        if (ocupada) {
            Rectangle origem = { (float)(inventario->itens[i].tipo * 64), 0.0f, 64.0f, (float)pocoes.height };
            Rectangle destino = { sx - 4.0f, sy - 6.0f, 42.0f, 42.0f };
            DrawTexturePro(pocoes, origem, destino, (Vector2){ 0, 0 }, 0.0f, WHITE);
        }
    }
}

void DesenharBarraChefe(const Inimigo *chefe)
{
    float largura = 420.0f;
    float x = (LARGURA_JANELA - largura) / 2.0f;
    float y = 62.0f;

    float porcentagem = (float)chefe->vida / (float)chefe->vidaMaxima;
    if (porcentagem < 0.0f) porcentagem = 0.0f;

    DesenharTextoCentralizado("CHEFE ORC", (int)y - 24, 20, (Color){ 255, 190, 110, 255 });
    DrawRectangle((int)x - 3, (int)y - 3, (int)largura + 6, 22, BLACK);
    DrawRectangleLines((int)x - 3, (int)y - 3, (int)largura + 6, 22, COR_BORDA);
    DrawRectangle((int)x, (int)y, (int)largura, 16, (Color){ 80, 18, 18, 255 });
    DrawRectangle((int)x, (int)y, (int)(largura * porcentagem), 16, (Color){ 215, 50, 40, 255 });
    DrawRectangle((int)x, (int)y, (int)(largura * porcentagem), 5, (Color){ 245, 110, 90, 255 });
}

void DesenharAvisoOnda(int proximaOnda, bool comChefe)
{
    char texto[32];
    snprintf(texto, sizeof(texto), "ONDA %d", proximaOnda);

    DesenharTextoCentralizado(texto, ALTURA_JANELA / 2 - 230, 56, WHITE);

    if (comChefe) {
        DesenharTextoCentralizado("CUIDADO: UM CHEFE SE APROXIMA!", ALTURA_JANELA / 2 - 165, 26, (Color){ 255, 90, 70, 255 });
    } else {
        DesenharTextoCentralizado("PREPARE-SE!", ALTURA_JANELA / 2 - 165, 24, COR_DESTAQUE);
    }
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
    DesenharTextoCentralizado("ENTER: JOGAR DE NOVO", ALTURA_JANELA / 2 + 40, 22, COR_DESTAQUE);
    DesenharTextoCentralizado("ESC: MENU PRINCIPAL", ALTURA_JANELA / 2 + 72, 20, (Color){ 170, 200, 230, 255 });
}

void DesenharEscurecer(int alfa)
{
    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, (Color){ 0, 0, 0, (unsigned char)alfa });
}

void DesenharTituloJogo(int y)
{
    DesenharTextoContorno("ARENA SURVIVAL", (LARGURA_JANELA - MeasureText("ARENA SURVIVAL", 64)) / 2, y, 64, (Color){ 255, 214, 110, 255 });
    DesenharTextoCentralizado("Estrutura de Dados I", y + 78, 22, (Color){ 170, 200, 230, 255 });
}

void DesenharMenuOpcoes(const char *titulo, const char *subtitulo, const char *const *opcoes, int quantidade, int selecionada, int y)
{
    if (titulo != NULL) {
        DesenharTextoContorno(titulo, (LARGURA_JANELA - MeasureText(titulo, 48)) / 2, y, 48, WHITE);
        y += 70;
    }
    if (subtitulo != NULL) {
        DesenharTextoCentralizado(subtitulo, y - 14, 20, (Color){ 170, 200, 230, 255 });
        y += 24;
    }

    for (int i = 0; i < quantidade; i++) {
        int larguraCaixa = 320;
        int xCaixa = (LARGURA_JANELA - larguraCaixa) / 2;
        int yCaixa = y + i * 62;
        bool ativa = (i == selecionada);

        DesenharPainel((float)xCaixa, (float)yCaixa, (float)larguraCaixa, 52.0f);
        if (ativa) {
            DrawRectangleRoundedLines((Rectangle){ (float)xCaixa, (float)yCaixa, (float)larguraCaixa, 52.0f }, 0.2f, 9, COR_DESTAQUE);
            DrawText(">", xCaixa + 18, yCaixa + 14, 26, COR_DESTAQUE);
        }

        int larguraTexto = MeasureText(opcoes[i], 26);
        DrawText(opcoes[i], xCaixa + (larguraCaixa - larguraTexto) / 2, yCaixa + 13, 26, ativa ? COR_DESTAQUE : WHITE);
    }
}

void DesenharTelaControles(void)
{
    DesenharEscurecer(200);

    float largura = 640.0f;
    float altura = 470.0f;
    float x = (LARGURA_JANELA - largura) / 2.0f;
    float y = 130.0f;

    DesenharPainel(x, y, largura, altura);
    DesenharTextoCentralizado("CONTROLES", (int)y + 26, 36, COR_DESTAQUE);

    const char *teclas[] = { "WASD / SETAS", "ESPACO", "SHIFT", "Q", "ESC ou P", "ENTER" };
    const char *acoes[] = {
        "Mover o anao",
        "Atacar com a espada",
        "Esquivar (invulneravel)",
        "Usar a ultima pocao coletada",
        "Pausar o jogo",
        "Confirmar nos menus"
    };

    for (int i = 0; i < 6; i++) {
        int linhaY = (int)y + 90 + i * 40;
        DrawText(teclas[i], (int)x + 50, linhaY, 22, (Color){ 120, 200, 255, 255 });
        DrawText(acoes[i], (int)x + 270, linhaY, 22, WHITE);
    }

    DesenharTextoCentralizado("Fuja do circulo vermelho dos orcs e da linha de mira dos arqueiros", (int)y + 340, 18, (Color){ 255, 150, 120, 255 });
    DesenharTextoCentralizado("Pocoes: ate 5 na mochila (pilha), a ultima coletada e a primeira usada", (int)y + 366, 18, (Color){ 170, 200, 230, 255 });
    DesenharTextoCentralizado("ENTER ou ESC: voltar", (int)y + 420, 20, COR_DESTAQUE);
}
