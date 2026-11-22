/* ============================================================
 *  inimigo.c
 *  Implementação do módulo Inimigo (Orc).
 * ============================================================ */
#include "inimigo.h"
#include <stdio.h>
#include <math.h>

#define ALTURA_ORC       110.0f   // altura com que o orc aparece na tela
#define VIDA_BASE        30       // vida de um orc = VIDA_BASE + VIDA_POR_NIVEL * nivel
#define VIDA_POR_NIVEL   10
#define DURACAO_FLASH    0.15f    // segundos que o orc fica avermelhado ao levar dano
#define VELOCIDADE_BASE  55.0f
#define VELOCIDADE_NIVEL 15.0f
#define DISTANCIA_PARADA 25.0f
#define DISTANCIA_ENTRE_ORCS 50.0f
#define DANO_CONTATO_BASE  4
#define DANO_CONTATO_NIVEL 2
#define PI_F                 3.14159265f
#define COMPRIMENTO_PASSO    30.0f
#define QUIQUE_ANDAR         9.0f
#define BALANCO_ANDAR        6.0f
#define SUAVIZACAO_ANDAR     10.0f
#define DURACAO_ATORDOAMENTO 0.25f
#define DURACAO_TEXTO_DANO   0.7f
#define DURACAO_MORTE        0.4f
#define FORCA_RECUO          420.0f
#define FORCA_RECUO_MORTE    620.0f
#define DESACELERACAO_RECUO  10.0f

Texture2D CarregarSpriteOrc(void)
{
    // Coloque a imagem do orc em assets/orc.png
    return LoadTexture("assets/orc.png");
}

void IniciarInimigo(Inimigo *inimigo, Texture2D sprite, Vector2 posicao, int nivel)
{
    inimigo->sprite  = sprite;
    inimigo->posicao = posicao;
    inimigo->nivel   = nivel;

    inimigo->vidaMaxima = VIDA_BASE + VIDA_POR_NIVEL * nivel;
    inimigo->vida       = inimigo->vidaMaxima;
    inimigo->tempoDano  = 0.0f;
    inimigo->velocidade = VELOCIDADE_BASE + VELOCIDADE_NIVEL * nivel;

    inimigo->andando          = false;
    inimigo->intensidadeAndar = 0.0f;
    inimigo->tempoAnimacao    = (float)GetRandomValue(0, 628) / 100.0f;
    inimigo->fasePasso        = (float)GetRandomValue(0, 314) / 100.0f;
    inimigo->recuo            = (Vector2){ 0.0f, 0.0f };
    inimigo->tempoAtordoado   = 0.0f;
    inimigo->ultimoDano       = 0;
    inimigo->tempoTextoDano   = 0.0f;
    inimigo->tempoMorte       = 0.0f;
    inimigo->ladoQueda        = 1;

    // Mantém a proporção da imagem original (evita o orc "esticado").
    // Se a textura não carregou (altura 0), usa uma proporção padrão.
    float proporcao = (sprite.height > 0) ? (float)sprite.width / (float)sprite.height : 0.7f;
    inimigo->tamanho = (Vector2){ ALTURA_ORC * proporcao, ALTURA_ORC };

    // Vira o orc para o centro da arena. O sprite original olha para a direita.
    float centroOrcX = posicao.x + inimigo->tamanho.x / 2.0f;
    inimigo->olhandoDireita = (centroOrcX < GetScreenWidth() / 2.0f);
}

void AtualizarInimigo(Inimigo *inimigo, float deltaTime)
{
    if (inimigo->tempoDano > 0.0f) {
        inimigo->tempoDano -= deltaTime;
        if (inimigo->tempoDano < 0.0f) inimigo->tempoDano = 0.0f;
    }

    if (inimigo->tempoAtordoado > 0.0f) {
        inimigo->tempoAtordoado -= deltaTime;
        if (inimigo->tempoAtordoado < 0.0f) inimigo->tempoAtordoado = 0.0f;
    }

    if (inimigo->tempoTextoDano > 0.0f) {
        inimigo->tempoTextoDano -= deltaTime;
        if (inimigo->tempoTextoDano < 0.0f) inimigo->tempoTextoDano = 0.0f;
    }

    if (inimigo->tempoMorte > 0.0f) {
        inimigo->tempoMorte -= deltaTime;
        if (inimigo->tempoMorte < 0.0f) inimigo->tempoMorte = 0.0f;
    }

    inimigo->tempoAnimacao += deltaTime;

    float alvo = inimigo->andando ? 1.0f : 0.0f;
    float suavizacao = SUAVIZACAO_ANDAR * deltaTime;
    if (suavizacao > 1.0f) suavizacao = 1.0f;
    inimigo->intensidadeAndar += (alvo - inimigo->intensidadeAndar) * suavizacao;
}

static void LimitarNaArena(Inimigo *inimigo, Rectangle limites)
{
    if (inimigo->posicao.x < limites.x)
        inimigo->posicao.x = limites.x;
    if (inimigo->posicao.y < limites.y)
        inimigo->posicao.y = limites.y;
    if (inimigo->posicao.x + inimigo->tamanho.x > limites.x + limites.width)
        inimigo->posicao.x = limites.x + limites.width - inimigo->tamanho.x;
    if (inimigo->posicao.y + inimigo->tamanho.y > limites.y + limites.height)
        inimigo->posicao.y = limites.y + limites.height - inimigo->tamanho.y;
}

void MoverInimigo(Inimigo *inimigo, Vector2 alvo, float deltaTime, Rectangle limites)
{
    if (inimigo->recuo.x != 0.0f || inimigo->recuo.y != 0.0f) {
        inimigo->posicao.x += inimigo->recuo.x * deltaTime;
        inimigo->posicao.y += inimigo->recuo.y * deltaTime;

        float atrito = expf(-DESACELERACAO_RECUO * deltaTime);
        inimigo->recuo.x *= atrito;
        inimigo->recuo.y *= atrito;

        if (fabsf(inimigo->recuo.x) < 4.0f && fabsf(inimigo->recuo.y) < 4.0f) {
            inimigo->recuo = (Vector2){ 0.0f, 0.0f };
        }

        LimitarNaArena(inimigo, limites);
    }

    inimigo->andando = false;

    if (inimigo->vida <= 0 || inimigo->tempoAtordoado > 0.0f) return;

    float centroX = inimigo->posicao.x + inimigo->tamanho.x / 2.0f;
    float centroY = inimigo->posicao.y + inimigo->tamanho.y / 2.0f;

    float dx = alvo.x - centroX;
    float dy = alvo.y - centroY;
    float distancia = sqrtf(dx * dx + dy * dy);

    if (dx > 1.0f)  inimigo->olhandoDireita = true;
    if (dx < -1.0f) inimigo->olhandoDireita = false;

    if (distancia <= DISTANCIA_PARADA) return;

    inimigo->posicao.x += (dx / distancia) * inimigo->velocidade * deltaTime;
    inimigo->posicao.y += (dy / distancia) * inimigo->velocidade * deltaTime;

    inimigo->andando = true;
    inimigo->fasePasso += PI_F * (inimigo->velocidade / COMPRIMENTO_PASSO) * deltaTime;

    LimitarNaArena(inimigo, limites);
}

void AfastarInimigos(Inimigo *a, Inimigo *b, Rectangle limites)
{
    if (a->vida <= 0 || b->vida <= 0) return;

    float dx = (b->posicao.x + b->tamanho.x / 2.0f) - (a->posicao.x + a->tamanho.x / 2.0f);
    float dy = (b->posicao.y + b->tamanho.y / 2.0f) - (a->posicao.y + a->tamanho.y / 2.0f);
    float distancia = sqrtf(dx * dx + dy * dy);

    if (distancia >= DISTANCIA_ENTRE_ORCS) return;

    if (distancia < 0.001f) {
        dx = 1.0f;
        dy = 0.0f;
        distancia = 1.0f;
    }

    float empurrao = (DISTANCIA_ENTRE_ORCS - distancia) / 2.0f;
    float nx = dx / distancia;
    float ny = dy / distancia;

    a->posicao.x -= nx * empurrao;
    a->posicao.y -= ny * empurrao;
    b->posicao.x += nx * empurrao;
    b->posicao.y += ny * empurrao;

    LimitarNaArena(a, limites);
    LimitarNaArena(b, limites);
}

int DanoContatoInimigo(const Inimigo *inimigo)
{
    return DANO_CONTATO_BASE + DANO_CONTATO_NIVEL * inimigo->nivel;
}

bool InimigoPertoDe(const Inimigo *inimigo, Vector2 alvo, float alcance)
{
    if (inimigo->vida <= 0) return false;   // morto não leva golpe

    // Centro do orc
    float centroX = inimigo->posicao.x + inimigo->tamanho.x / 2.0f;
    float centroY = inimigo->posicao.y + inimigo->tamanho.y / 2.0f;

    float dx = centroX - alvo.x;
    float dy = centroY - alvo.y;

    return sqrtf(dx * dx + dy * dy) <= alcance;
}

bool DanoInimigo(Inimigo *inimigo, int dano)
{
    if (inimigo->vida <= 0) return false;

    inimigo->vida -= dano;
    inimigo->tempoDano = DURACAO_FLASH;
    inimigo->ultimoDano = dano;
    inimigo->tempoTextoDano = DURACAO_TEXTO_DANO;

    if (inimigo->vida <= 0) {
        inimigo->vida = 0;
        inimigo->tempoMorte = DURACAO_MORTE;
        return true;    // este golpe matou o orc
    }
    return false;
}

bool GolpearInimigo(Inimigo *inimigo, int dano, Vector2 origem)
{
    if (inimigo->vida <= 0) return false;

    float dx = (inimigo->posicao.x + inimigo->tamanho.x / 2.0f) - origem.x;
    float dy = (inimigo->posicao.y + inimigo->tamanho.y / 2.0f) - origem.y;
    float distancia = sqrtf(dx * dx + dy * dy);

    if (distancia < 0.001f) {
        dx = 1.0f;
        dy = 0.0f;
        distancia = 1.0f;
    }

    bool morreu = DanoInimigo(inimigo, dano);

    float forca = morreu ? FORCA_RECUO_MORTE : FORCA_RECUO;
    inimigo->recuo = (Vector2){ (dx / distancia) * forca, (dy / distancia) * forca };
    inimigo->ladoQueda = (dx >= 0.0f) ? 1 : -1;

    if (!morreu) inimigo->tempoAtordoado = DURACAO_ATORDOAMENTO;

    return morreu;
}

void DesenharInimigo(const Inimigo *inimigo)
{
    bool morrendo = (inimigo->vida <= 0);
    bool invisivel = morrendo && inimigo->tempoMorte <= 0.0f;

    float passo = fabsf(sinf(inimigo->fasePasso));
    float intensidade = inimigo->intensidadeAndar;

    float quique = passo * QUIQUE_ANDAR * intensidade;
    float inclinacao = sinf(inimigo->fasePasso) * BALANCO_ANDAR * intensidade;
    float esticar = 0.12f * (passo - 0.5f) * intensidade;
    float respirar = 0.012f * sinf(inimigo->tempoAnimacao * 3.0f) * (1.0f - intensidade);

    float escalaY = 1.0f + esticar + respirar;
    float escalaX = 1.0f - esticar - respirar;

    float pancada = inimigo->tempoDano / DURACAO_FLASH;
    if (pancada < 0.0f) pancada = 0.0f;
    if (pancada > 1.0f) pancada = 1.0f;

    escalaX *= 1.0f + 0.22f * pancada;
    escalaY *= 1.0f - 0.22f * pancada;

    float forcaRecuo = inimigo->recuo.x / FORCA_RECUO;
    if (forcaRecuo > 1.5f) forcaRecuo = 1.5f;
    if (forcaRecuo < -1.5f) forcaRecuo = -1.5f;
    inclinacao += forcaRecuo * 10.0f;

    float opacidade = 1.0f;

    if (morrendo) {
        float p = 1.0f - inimigo->tempoMorte / DURACAO_MORTE;
        inclinacao = (float)inimigo->ladoQueda * 85.0f * p * p;
        escalaX = 1.0f - 0.15f * p;
        escalaY = 1.0f - 0.15f * p;
        quique = 0.0f;
        opacidade = 1.0f - p * p;
    }

    if (!invisivel) {
        Rectangle origem = {
            0, 0,
            (float)inimigo->sprite.width,
            (float)inimigo->sprite.height
        };

        if (!inimigo->olhandoDireita) {
            origem.width = -origem.width;
        }

        float largura = inimigo->tamanho.x * escalaX;
        float altura  = inimigo->tamanho.y * escalaY;

        Rectangle destino = {
            inimigo->posicao.x + inimigo->tamanho.x / 2.0f,
            inimigo->posicao.y + inimigo->tamanho.y - quique,
            largura, altura
        };

        Color tinta = (inimigo->tempoDano > 0.0f) ? (Color){ 255, 110, 110, 255 } : WHITE;
        tinta.a = (unsigned char)(255.0f * opacidade);

        DrawTexturePro(inimigo->sprite, origem, destino, (Vector2){ largura / 2.0f, altura }, inclinacao, tinta);
    }

    if (!morrendo) {
        float larguraBarra = inimigo->tamanho.x;
        float alturaBarra  = 6.0f;
        float barraX = inimigo->posicao.x;
        float barraY = inimigo->posicao.y - 12.0f;

        float porcentagem = (float)inimigo->vida / (float)inimigo->vidaMaxima;
        if (porcentagem < 0.0f) porcentagem = 0.0f;

        DrawRectangle((int)barraX - 1, (int)barraY - 1, (int)larguraBarra + 2, (int)alturaBarra + 2, BLACK);
        DrawRectangle((int)barraX, (int)barraY, (int)larguraBarra, (int)alturaBarra, (Color){ 90, 20, 20, 255 });
        DrawRectangle((int)barraX, (int)barraY, (int)(larguraBarra * porcentagem), (int)alturaBarra, (Color){ 220, 40, 40, 255 });

        char textoNivel[16];
        snprintf(textoNivel, sizeof(textoNivel), "Nv %d", inimigo->nivel);
        DrawText(textoNivel, (int)barraX, (int)barraY - 16, 14, WHITE);
    }

    if (inimigo->tempoTextoDano > 0.0f) {
        float t = inimigo->tempoTextoDano / DURACAO_TEXTO_DANO;
        float aparecer = (t > 0.3f) ? 1.0f : t / 0.3f;
        unsigned char alfa = (unsigned char)(255.0f * aparecer);

        char textoDano[16];
        snprintf(textoDano, sizeof(textoDano), "-%d", inimigo->ultimoDano);

        int larguraTexto = MeasureText(textoDano, 22);
        int textoX = (int)(inimigo->posicao.x + inimigo->tamanho.x / 2.0f) - larguraTexto / 2;
        int textoY = (int)(inimigo->posicao.y - 46.0f - (1.0f - t) * 30.0f);

        Color contorno = { 0, 0, 0, alfa };
        DrawText(textoDano, textoX - 2, textoY, 22, contorno);
        DrawText(textoDano, textoX + 2, textoY, 22, contorno);
        DrawText(textoDano, textoX, textoY - 2, 22, contorno);
        DrawText(textoDano, textoX, textoY + 2, 22, contorno);
        DrawText(textoDano, textoX, textoY, 22, (Color){ 255, 214, 64, alfa });
    }
}

void DescarregarSpriteOrc(Texture2D sprite)
{
    UnloadTexture(sprite);
}