#include "inimigo.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define PI_F                 3.14159265f
#define DURACAO_FLASH        0.15f
#define COMPRIMENTO_PASSO    22.0f
#define QUIQUE_ANDAR         12.0f
#define BALANCO_ANDAR        8.0f
#define INCLINACAO_FRENTE    3.5f
#define SUAVIZACAO_ANDAR     10.0f
#define DURACAO_ATORDOAMENTO 0.25f
#define DURACAO_TEXTO_DANO   0.7f
#define DURACAO_MORTE        0.9f
#define LARGURA_CORPO_SKIN   0.55f
#define PASSO_POR_CICLO      0.75f
#define FPS_PARADO           9.0f
#define FORCA_RECUO          420.0f
#define FORCA_RECUO_MORTE    620.0f
#define DESACELERACAO_RECUO  10.0f
#define ACHATAMENTO_CHAO     0.55f
#define ALTURA_MIRA_ALVO     52.0f
#define TRAVA_MIRA           0.7f
#define RECUPERACAO_APOS_ATORDOAR 0.3f

typedef struct {
    float altura;
    int vidaBase;
    int vidaNivel;
    float velocidadeBase;
    float velocidadeNivel;
    float alcance;
    float preparo;
    float ataque;
    float recuperacao;
    int danoBase;
    int danoNivel;
    int pontos;
    int chanceDrop;
    bool superArmadura;
    float raioCorpo;
    float raioSeparacao;
    float distanciaIdeal;
} PerfilInimigo;

static const PerfilInimigo PERFIS[] = {
    [INIMIGO_ORC] = {
        .altura = 110.0f, .vidaBase = 30, .vidaNivel = 10, .velocidadeBase = 55.0f, .velocidadeNivel = 15.0f,
        .alcance = 72.0f, .preparo = 0.5f, .ataque = 0.2f, .recuperacao = 0.8f,
        .danoBase = 6, .danoNivel = 2, .pontos = 100, .chanceDrop = 22,
        .superArmadura = false, .raioCorpo = 0.0f, .raioSeparacao = 25.0f, .distanciaIdeal = 0.0f
    },
    [INIMIGO_ARQUEIRO] = {
        .altura = 100.0f, .vidaBase = 18, .vidaNivel = 6, .velocidadeBase = 80.0f, .velocidadeNivel = 8.0f,
        .alcance = 0.0f, .preparo = 0.8f, .ataque = 0.15f, .recuperacao = 1.5f,
        .danoBase = 6, .danoNivel = 2, .pontos = 150, .chanceDrop = 32,
        .superArmadura = false, .raioCorpo = 0.0f, .raioSeparacao = 25.0f, .distanciaIdeal = 300.0f
    },
    [INIMIGO_CHEFE] = {
        .altura = 190.0f, .vidaBase = 240, .vidaNivel = 30, .velocidadeBase = 40.0f, .velocidadeNivel = 5.0f,
        .alcance = 140.0f, .preparo = 1.0f, .ataque = 0.25f, .recuperacao = 1.3f,
        .danoBase = 20, .danoNivel = 3, .pontos = 1000, .chanceDrop = 100,
        .superArmadura = true, .raioCorpo = 50.0f, .raioSeparacao = 45.0f, .distanciaIdeal = 0.0f
    }
};

static const PerfilInimigo *Perfil(const Inimigo *inimigo)
{
    return &PERFIS[inimigo->tipo];
}

SpritesInimigos CarregarSpritesInimigos(void)
{
    SpritesInimigos s;
    s.orc      = LoadTexture("assets/orc.png");
    s.arqueiro = LoadTexture("assets/orc_arqueiro.png");
    s.chefe    = LoadTexture("assets/orc_chefe.png");
    s.arco     = LoadTexture("assets/arco.png");
    s.flecha   = LoadTexture("assets/flecha.png");
    s.skinOrc    = CarregarSkinOrc(1);
    s.skinOrcAlt = CarregarSkinOrc(2);
    s.skinChefe  = CarregarSkinOrc(3);
    return s;
}

void DescarregarSpritesInimigos(SpritesInimigos sprites)
{
    UnloadTexture(sprites.orc);
    UnloadTexture(sprites.arqueiro);
    UnloadTexture(sprites.chefe);
    UnloadTexture(sprites.arco);
    UnloadTexture(sprites.flecha);
    DescarregarSkinOrc(&sprites.skinOrc);
    DescarregarSkinOrc(&sprites.skinOrcAlt);
    DescarregarSkinOrc(&sprites.skinChefe);
}

void IniciarInimigo(Inimigo *inimigo, TipoInimigo tipo, const SpritesInimigos *sprites, Vector2 posicao, int nivel)
{
    const PerfilInimigo *perfil = &PERFIS[tipo];

    inimigo->tipo = tipo;
    inimigo->estadoIA = IA_PERSEGUINDO;
    inimigo->tempoEstado = 0.0f;
    inimigo->anguloMira = 0.0f;
    inimigo->posicao = posicao;
    inimigo->nivel = nivel;
    inimigo->arco = sprites->arco;
    memset(&inimigo->skin, 0, sizeof(inimigo->skin));
    inimigo->faseQuadro = 0.0f;
    if (tipo == INIMIGO_ORC) {
        bool usarAlt = sprites->skinOrcAlt.valida && (!sprites->skinOrc.valida || GetRandomValue(0, 1) == 1);
        if (usarAlt) inimigo->skin = sprites->skinOrcAlt;
        else if (sprites->skinOrc.valida) inimigo->skin = sprites->skinOrc;
    }
    if (tipo == INIMIGO_CHEFE && sprites->skinChefe.valida) inimigo->skin = sprites->skinChefe;

    switch (tipo) {
        case INIMIGO_ARQUEIRO: inimigo->sprite = sprites->arqueiro; break;
        case INIMIGO_CHEFE:    inimigo->sprite = sprites->chefe;    break;
        default:               inimigo->sprite = sprites->orc;      break;
    }

    inimigo->vidaMaxima = perfil->vidaBase + perfil->vidaNivel * nivel;
    inimigo->vida = inimigo->vidaMaxima;
    inimigo->velocidade = perfil->velocidadeBase + perfil->velocidadeNivel * nivel;
    inimigo->tempoDano = 0.0f;
    inimigo->andando = false;
    inimigo->intensidadeAndar = 0.0f;
    inimigo->tempoAnimacao = (float)GetRandomValue(0, 628) / 100.0f;
    inimigo->fasePasso = (float)GetRandomValue(0, 314) / 100.0f;
    inimigo->recuo = (Vector2){ 0.0f, 0.0f };
    inimigo->tempoAtordoado = 0.0f;
    inimigo->ultimoDano = 0;
    inimigo->tempoTextoDano = 0.0f;
    inimigo->tempoMorte = 0.0f;
    inimigo->ladoQueda = 1;

    float proporcao = (inimigo->sprite.height > 0) ? (float)inimigo->sprite.width / (float)inimigo->sprite.height : 0.7f;
    inimigo->tamanho = (Vector2){ perfil->altura * proporcao, perfil->altura };
    if (inimigo->skin.valida) inimigo->tamanho.x = perfil->altura * LARGURA_CORPO_SKIN;

    float centroX = posicao.x + inimigo->tamanho.x / 2.0f;
    inimigo->olhandoDireita = (centroX < GetScreenWidth() / 2.0f);
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

Vector2 PeDoInimigo(const Inimigo *inimigo)
{
    return (Vector2){
        inimigo->posicao.x + inimigo->tamanho.x / 2.0f,
        inimigo->posicao.y + inimigo->tamanho.y - 6.0f
    };
}

static Vector2 PeitoDoInimigo(const Inimigo *inimigo)
{
    return (Vector2){
        inimigo->posicao.x + inimigo->tamanho.x / 2.0f,
        inimigo->posicao.y + inimigo->tamanho.y * 0.5f
    };
}

Vector2 PontaDoArco(const Inimigo *inimigo)
{
    Vector2 peito = PeitoDoInimigo(inimigo);
    return (Vector2){
        peito.x + cosf(inimigo->anguloMira) * 34.0f,
        peito.y + sinf(inimigo->anguloMira) * 34.0f
    };
}

bool GolpeAtingeAlvo(const Inimigo *inimigo, Vector2 peAlvo)
{
    float alcance = Perfil(inimigo)->alcance;
    if (alcance <= 0.0f) return false;

    Vector2 pe = PeDoInimigo(inimigo);
    float nx = (peAlvo.x - pe.x) / alcance;
    float ny = (peAlvo.y - pe.y) / (alcance * ACHATAMENTO_CHAO);
    return nx * nx + ny * ny <= 1.0f;
}

static float DistanciaEliptica(const Inimigo *inimigo, Vector2 peAlvo)
{
    float alcance = Perfil(inimigo)->alcance;
    Vector2 pe = PeDoInimigo(inimigo);
    float nx = (peAlvo.x - pe.x) / alcance;
    float ny = (peAlvo.y - pe.y) / (alcance * ACHATAMENTO_CHAO);
    return sqrtf(nx * nx + ny * ny);
}

static void AplicarRecuo(Inimigo *inimigo, float dt, Rectangle limites)
{
    if (inimigo->recuo.x == 0.0f && inimigo->recuo.y == 0.0f) return;

    inimigo->posicao.x += inimigo->recuo.x * dt;
    inimigo->posicao.y += inimigo->recuo.y * dt;

    float atrito = expf(-DESACELERACAO_RECUO * dt);
    inimigo->recuo.x *= atrito;
    inimigo->recuo.y *= atrito;

    if (fabsf(inimigo->recuo.x) < 4.0f && fabsf(inimigo->recuo.y) < 4.0f) {
        inimigo->recuo = (Vector2){ 0.0f, 0.0f };
    }

    LimitarNaArena(inimigo, limites);
}

static void AndarPara(Inimigo *inimigo, float dirX, float dirY, float velocidade, float dt, Rectangle limites)
{
    inimigo->posicao.x += dirX * velocidade * dt;
    inimigo->posicao.y += dirY * velocidade * dt;
    inimigo->andando = true;
    inimigo->fasePasso += PI_F * (velocidade / COMPRIMENTO_PASSO) * dt;
    inimigo->faseQuadro += velocidade * dt / (inimigo->tamanho.y * PASSO_POR_CICLO);
    LimitarNaArena(inimigo, limites);
}

static void MirarNoAlvo(Inimigo *inimigo, Vector2 peAlvo)
{
    Vector2 peito = PeitoDoInimigo(inimigo);
    inimigo->anguloMira = atan2f((peAlvo.y - ALTURA_MIRA_ALVO) - peito.y, peAlvo.x - peito.x);
}

static void MudarEstado(Inimigo *inimigo, EstadoIA estado)
{
    inimigo->estadoIA = estado;
    inimigo->tempoEstado = 0.0f;
}

static void ComportamentoPerseguir(Inimigo *inimigo, Vector2 peAlvo, float dt, Rectangle limites)
{
    const PerfilInimigo *perfil = Perfil(inimigo);
    Vector2 pe = PeDoInimigo(inimigo);
    float dx = peAlvo.x - pe.x;
    float dy = peAlvo.y - pe.y;
    float distancia = sqrtf(dx * dx + dy * dy);
    float nx = (distancia > 0.001f) ? dx / distancia : 0.0f;
    float ny = (distancia > 0.001f) ? dy / distancia : 0.0f;

    if (perfil->alcance > 0.0f) {
        if (DistanciaEliptica(inimigo, peAlvo) > 0.8f) {
            AndarPara(inimigo, nx, ny, inimigo->velocidade, dt, limites);
        } else {
            MudarEstado(inimigo, IA_PREPARANDO);
        }
        return;
    }

    if (distancia < perfil->distanciaIdeal - 80.0f) {
        AndarPara(inimigo, -nx, -ny, inimigo->velocidade, dt, limites);
    } else if (distancia > perfil->distanciaIdeal + 60.0f) {
        AndarPara(inimigo, nx, ny, inimigo->velocidade, dt, limites);
    }

    if (distancia > 120.0f && distancia < 560.0f && !inimigo->andando) {
        MudarEstado(inimigo, IA_PREPARANDO);
    }
}

EventoInimigo AtualizarIAInimigo(Inimigo *inimigo, Vector2 peAlvo, float dt, Rectangle limites)
{
    const PerfilInimigo *perfil = Perfil(inimigo);

    AplicarRecuo(inimigo, dt, limites);
    inimigo->andando = false;

    if (inimigo->vida <= 0) return EVENTO_NENHUM;

    if (inimigo->tempoAtordoado > 0.0f) {
        if (inimigo->estadoIA != IA_PERSEGUINDO && !perfil->superArmadura) {
            MudarEstado(inimigo, IA_RECUPERANDO);
            inimigo->tempoEstado = perfil->recuperacao - RECUPERACAO_APOS_ATORDOAR;
        }
        return EVENTO_NENHUM;
    }

    Vector2 pe = PeDoInimigo(inimigo);
    float dx = peAlvo.x - pe.x;
    if (inimigo->estadoIA != IA_ATACANDO) {
        if (dx > 1.0f)  inimigo->olhandoDireita = true;
        if (dx < -1.0f) inimigo->olhandoDireita = false;
    }

    if (inimigo->tipo == INIMIGO_ARQUEIRO && inimigo->estadoIA != IA_ATACANDO) {
        bool travada = inimigo->estadoIA == IA_PREPARANDO && inimigo->tempoEstado / perfil->preparo >= TRAVA_MIRA;
        if (!travada) MirarNoAlvo(inimigo, peAlvo);
    }

    EventoInimigo evento = EVENTO_NENHUM;

    switch (inimigo->estadoIA) {
        case IA_PERSEGUINDO:
            ComportamentoPerseguir(inimigo, peAlvo, dt, limites);
            break;

        case IA_PREPARANDO:
            inimigo->tempoEstado += dt;
            if (inimigo->tempoEstado >= perfil->preparo) {
                MudarEstado(inimigo, IA_ATACANDO);
                evento = (inimigo->tipo == INIMIGO_ARQUEIRO) ? EVENTO_DISPARO : EVENTO_GOLPE_CORPO;
            }
            break;

        case IA_ATACANDO:
            inimigo->tempoEstado += dt;
            if (inimigo->tempoEstado >= perfil->ataque) MudarEstado(inimigo, IA_RECUPERANDO);
            break;

        case IA_RECUPERANDO:
            inimigo->tempoEstado += dt;
            if (inimigo->tempoEstado >= perfil->recuperacao) MudarEstado(inimigo, IA_PERSEGUINDO);
            break;
    }

    return evento;
}

void AfastarInimigos(Inimigo *a, Inimigo *b, Rectangle limites)
{
    if (a->vida <= 0 || b->vida <= 0) return;

    float minima = Perfil(a)->raioSeparacao + Perfil(b)->raioSeparacao;

    float dx = (b->posicao.x + b->tamanho.x / 2.0f) - (a->posicao.x + a->tamanho.x / 2.0f);
    float dy = (b->posicao.y + b->tamanho.y / 2.0f) - (a->posicao.y + a->tamanho.y / 2.0f);
    float distancia = sqrtf(dx * dx + dy * dy);

    if (distancia >= minima) return;

    if (distancia < 0.001f) {
        dx = 1.0f;
        dy = 0.0f;
        distancia = 1.0f;
    }

    float empurrao = (minima - distancia) / 2.0f;
    float nx = dx / distancia;
    float ny = dy / distancia;

    a->posicao.x -= nx * empurrao;
    a->posicao.y -= ny * empurrao;
    b->posicao.x += nx * empurrao;
    b->posicao.y += ny * empurrao;

    LimitarNaArena(a, limites);
    LimitarNaArena(b, limites);
}

int DanoCorpoInimigo(const Inimigo *inimigo)
{
    return Perfil(inimigo)->danoBase + Perfil(inimigo)->danoNivel * inimigo->nivel;
}

int DanoFlechaInimigo(const Inimigo *inimigo)
{
    return DanoCorpoInimigo(inimigo);
}

int PontosInimigo(const Inimigo *inimigo)
{
    return Perfil(inimigo)->pontos;
}

int ChanceDropInimigo(const Inimigo *inimigo)
{
    return Perfil(inimigo)->chanceDrop;
}

bool InimigoPertoDe(const Inimigo *inimigo, Vector2 alvo, float alcance)
{
    if (inimigo->vida <= 0) return false;

    float centroX = inimigo->posicao.x + inimigo->tamanho.x / 2.0f;
    float centroY = inimigo->posicao.y + inimigo->tamanho.y / 2.0f;

    float dx = centroX - alvo.x;
    float dy = centroY - alvo.y;

    return sqrtf(dx * dx + dy * dy) <= alcance + Perfil(inimigo)->raioCorpo;
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
        return true;
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
    bool armadura = Perfil(inimigo)->superArmadura;

    float forca = morreu ? FORCA_RECUO_MORTE : FORCA_RECUO;
    if (armadura && !morreu) forca *= 0.2f;
    inimigo->recuo = (Vector2){ (dx / distancia) * forca, (dy / distancia) * forca };
    inimigo->ladoQueda = (dx >= 0.0f) ? 1 : -1;

    if (!morreu && !armadura) inimigo->tempoAtordoado = DURACAO_ATORDOAMENTO;

    return morreu;
}

static float ProgressoEstado(const Inimigo *inimigo, EstadoIA estado, float duracao)
{
    if (inimigo->estadoIA != estado || duracao <= 0.0f) return 0.0f;
    float p = inimigo->tempoEstado / duracao;
    return (p > 1.0f) ? 1.0f : p;
}

void DesenharTelegrafoInimigo(const Inimigo *inimigo)
{
    if (inimigo->vida <= 0) return;

    const PerfilInimigo *perfil = Perfil(inimigo);
    Vector2 pe = PeDoInimigo(inimigo);

    if (perfil->alcance > 0.0f) {
        float preparo = ProgressoEstado(inimigo, IA_PREPARANDO, perfil->preparo);
        float golpe = ProgressoEstado(inimigo, IA_ATACANDO, perfil->ataque);
        int rx = (int)perfil->alcance;
        int ry = (int)(perfil->alcance * ACHATAMENTO_CHAO);

        if (inimigo->estadoIA == IA_PREPARANDO) {
            DrawEllipse((int)pe.x, (int)pe.y, (float)rx, (float)ry, (Color){ 230, 40, 40, (unsigned char)(35 + 40 * preparo) });
            DrawEllipse((int)pe.x, (int)pe.y, rx * preparo, ry * preparo, (Color){ 240, 60, 40, (unsigned char)(70 + 90 * preparo) });
            DrawEllipseLines((int)pe.x, (int)pe.y, (float)rx, (float)ry, (Color){ 255, 90, 70, (unsigned char)(140 + 100 * preparo) });
        } else if (inimigo->estadoIA == IA_ATACANDO) {
            DrawEllipse((int)pe.x, (int)pe.y, (float)rx, (float)ry, (Color){ 255, 235, 200, (unsigned char)(200 * (1.0f - golpe)) });
            DrawEllipseLines((int)pe.x, (int)pe.y, (float)rx, (float)ry, (Color){ 255, 255, 255, (unsigned char)(255 * (1.0f - golpe)) });
        }
        return;
    }

    if (inimigo->tipo == INIMIGO_ARQUEIRO && inimigo->estadoIA == IA_PREPARANDO) {
        float p = ProgressoEstado(inimigo, IA_PREPARANDO, perfil->preparo);
        Vector2 inicio = PontaDoArco(inimigo);
        Vector2 fim = { inicio.x + cosf(inimigo->anguloMira) * 620.0f, inicio.y + sinf(inimigo->anguloMira) * 620.0f };
        bool travada = p >= TRAVA_MIRA;
        Color cor = travada ? (Color){ 255, 60, 50, 230 } : (Color){ 255, 200, 90, (unsigned char)(70 + 90 * p) };
        DrawLineEx(inicio, fim, travada ? 3.0f : 1.5f, cor);
    }
}

static Vector2 PontoNoCorpo(Vector2 pivo, float largura, float altura, bool direita, float fx, float fy, float inclinacao)
{
    float rx = ((direita ? fx : 1.0f - fx) - 0.5f) * largura;
    float ry = (fy - 1.0f) * altura;
    float a = inclinacao * (PI_F / 180.0f);
    return (Vector2){ pivo.x + rx * cosf(a) - ry * sinf(a), pivo.y + rx * sinf(a) + ry * cosf(a) };
}

static void DesenharPoeira(const Inimigo *inimigo, float intensidade)
{
    if (intensidade < 0.3f) return;

    float fase = fmodf(inimigo->fasePasso, PI_F) / PI_F;
    if (fase > 0.6f) return;

    float t = fase / 0.6f;
    float lado = (sinf(inimigo->fasePasso) >= 0.0f) ? 1.0f : -1.0f;
    float direcao = inimigo->olhandoDireita ? 1.0f : -1.0f;
    float escala = (inimigo->tipo == INIMIGO_CHEFE) ? 1.7f : 1.0f;

    float x = inimigo->posicao.x + inimigo->tamanho.x / 2.0f + lado * inimigo->tamanho.x * 0.16f - direcao * t * 14.0f;
    float y = inimigo->posicao.y + inimigo->tamanho.y - 5.0f - t * 6.0f;
    unsigned char alfa = (unsigned char)(110.0f * (1.0f - t) * intensidade);

    DrawCircle((int)x, (int)y, (3.0f + t * 9.0f) * escala, (Color){ 214, 200, 170, alfa });
    DrawCircle((int)(x - direcao * 6.0f), (int)(y + 2.0f), (2.0f + t * 6.0f) * escala, (Color){ 190, 176, 148, (unsigned char)(alfa * 0.7f) });
}

static void DesenharCorpoSkin(const Inimigo *inimigo, float preparo, float golpe, float recupera, float intensidade, bool morrendo)
{
    const SkinOrc *skin = &inimigo->skin;
    float direcao = inimigo->olhandoDireita ? 1.0f : -1.0f;
    AnimacaoOrc anim = ANIM_ATAQUE;
    int quadro = 0;
    bool procedural = false;
    float opacidade = 1.0f;
    float deslocX = 0.0f;
    float aquecimento = 0.0f;

    if (morrendo && SkinTemAnimacao(skin, ANIM_MORTE)) {
        float p = 1.0f - inimigo->tempoMorte / DURACAO_MORTE;
        if (p < 0.0f) p = 0.0f;
        anim = ANIM_MORTE;
        quadro = (int)(p * (float)skin->quadros[ANIM_MORTE]);
        if (p > 0.75f) opacidade = 1.0f - (p - 0.75f) / 0.25f;
    } else if (morrendo) {
        procedural = true;
        opacidade = inimigo->tempoMorte / DURACAO_MORTE;
    } else if (inimigo->tempoAtordoado > 0.0f && SkinTemAnimacao(skin, ANIM_DANO)) {
        float p = 1.0f - inimigo->tempoAtordoado / DURACAO_ATORDOAMENTO;
        anim = ANIM_DANO;
        quadro = (int)(p * (float)skin->quadros[ANIM_DANO]);
    } else if (inimigo->estadoIA == IA_PREPARANDO && SkinTemAnimacao(skin, ANIM_ATAQUE)) {
        quadro = (int)(preparo * 6.99f);
        deslocX = -direcao * 4.0f * preparo + sinf(inimigo->tempoAnimacao * 70.0f) * 1.2f * preparo;
        aquecimento = preparo;
    } else if (inimigo->estadoIA == IA_ATACANDO && SkinTemAnimacao(skin, ANIM_ATAQUE)) {
        quadro = 6 + (int)(golpe * 2.99f);
        deslocX = direcao * 10.0f;
    } else if (inimigo->estadoIA == IA_RECUPERANDO && SkinTemAnimacao(skin, ANIM_ATAQUE)) {
        quadro = 9;
    } else if (intensidade > 0.35f) {
        if (SkinTemAnimacao(skin, ANIM_ANDAR)) {
            anim = ANIM_ANDAR;
            quadro = (int)(inimigo->faseQuadro * (float)skin->quadros[ANIM_ANDAR]) % skin->quadros[ANIM_ANDAR];
        } else {
            procedural = true;
        }
    } else if (SkinTemAnimacao(skin, ANIM_PARADO)) {
        anim = ANIM_PARADO;
        quadro = (int)(inimigo->tempoAnimacao * FPS_PARADO) % skin->quadros[ANIM_PARADO];
    }

    if (!SkinTemAnimacao(skin, anim)) {
        anim = ANIM_ATAQUE;
        quadro = 0;
        if (!SkinTemAnimacao(skin, anim)) return;
    }

    float passo = fabsf(sinf(inimigo->fasePasso));
    float quique = 0.0f;
    float inclinacao = 0.0f;
    float escalaX = 1.0f;
    float escalaY = 1.0f;

    if (procedural && !morrendo) {
        quique = passo * QUIQUE_ANDAR * intensidade;
        inclinacao = sinf(inimigo->fasePasso) * BALANCO_ANDAR * intensidade + direcao * INCLINACAO_FRENTE * intensidade;
        float esticar = 0.12f * (passo - 0.5f) * intensidade;
        escalaX -= esticar;
        escalaY += esticar;
    }

    float pancada = inimigo->tempoDano / DURACAO_FLASH;
    if (pancada < 0.0f) pancada = 0.0f;
    if (pancada > 1.0f) pancada = 1.0f;
    escalaX *= 1.0f + 0.08f * pancada;
    escalaY *= 1.0f - 0.08f * pancada;

    float forcaRecuo = inimigo->recuo.x / FORCA_RECUO;
    if (forcaRecuo > 1.5f) forcaRecuo = 1.5f;
    if (forcaRecuo < -1.5f) forcaRecuo = -1.5f;
    if (!morrendo) inclinacao += forcaRecuo * 5.0f;

    float centroX = inimigo->posicao.x + inimigo->tamanho.x / 2.0f;
    float baseY = inimigo->posicao.y + inimigo->tamanho.y;

    if (!morrendo) DesenharPoeira(inimigo, intensidade);
    DrawEllipse((int)centroX, (int)(baseY - 4.0f), inimigo->tamanho.x * 0.5f * (1.0f - 0.2f * (quique / 20.0f)),
                inimigo->tamanho.x * 0.16f, (Color){ 0, 0, 0, (unsigned char)(70 * opacidade) });

    Color tinta = WHITE;
    if (inimigo->tempoDano > 0.0f) {
        tinta = (Color){ 255, 110, 110, 255 };
    } else if (aquecimento > 0.0f) {
        tinta = (Color){ 255, (unsigned char)(255 - 110 * aquecimento), (unsigned char)(255 - 150 * aquecimento), 255 };
    }
    tinta.a = (unsigned char)(255.0f * opacidade);

    Vector2 pe = { centroX + deslocX, baseY - quique };
    DesenharQuadroSkin(skin, anim, quadro, pe, inimigo->tamanho.y, inimigo->olhandoDireita, inclinacao, escalaX, escalaY, tinta);
}

void DesenharInimigo(const Inimigo *inimigo)
{
    const PerfilInimigo *perfil = Perfil(inimigo);
    bool morrendo = (inimigo->vida <= 0);
    bool invisivel = morrendo && inimigo->tempoMorte <= 0.0f;
    float direcao = inimigo->olhandoDireita ? 1.0f : -1.0f;

    float passo = fabsf(sinf(inimigo->fasePasso));
    float intensidade = inimigo->intensidadeAndar;

    float quique = passo * QUIQUE_ANDAR * intensidade;
    float inclinacao = sinf(inimigo->fasePasso) * BALANCO_ANDAR * intensidade + direcao * INCLINACAO_FRENTE * intensidade;
    float esticar = 0.12f * (passo - 0.5f) * intensidade;
    float respirar = 0.012f * sinf(inimigo->tempoAnimacao * 3.0f) * (1.0f - intensidade);

    float escalaY = 1.0f + esticar + respirar;
    float escalaX = 1.0f - esticar - respirar;
    float deslocX = 0.0f;
    float aquecimento = 0.0f;

    float preparo = ProgressoEstado(inimigo, IA_PREPARANDO, perfil->preparo);
    float golpe = ProgressoEstado(inimigo, IA_ATACANDO, perfil->ataque);
    float recupera = ProgressoEstado(inimigo, IA_RECUPERANDO, perfil->recuperacao);

    if (!morrendo) {
        float forca = (inimigo->tipo == INIMIGO_ARQUEIRO) ? 0.3f : 1.0f;
        float altura = (inimigo->tipo == INIMIGO_CHEFE) ? 34.0f : 0.0f;

        if (preparo > 0.0f) {
            inclinacao -= direcao * 16.0f * preparo * forca;
            escalaY += 0.07f * preparo * forca;
            escalaX -= 0.04f * preparo * forca;
            deslocX = -direcao * 8.0f * preparo * forca + sinf(inimigo->tempoAnimacao * 70.0f) * 1.6f * preparo * forca;
            quique += altura * preparo;
            aquecimento = preparo;
        } else if (golpe > 0.0f || inimigo->estadoIA == IA_ATACANDO) {
            inclinacao += direcao * 24.0f * forca;
            escalaX += 0.14f * forca;
            escalaY -= 0.1f * forca;
            deslocX = direcao * 16.0f * forca;
            quique += altura * (1.0f - golpe);
        } else if (inimigo->estadoIA == IA_RECUPERANDO) {
            float queda = 1.0f - recupera;
            inclinacao += direcao * 7.0f * queda * forca;
            escalaY -= 0.05f * queda * forca;
        }
    }

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
        deslocX = 0.0f;
        opacidade = 1.0f - p * p;
    }

    float largura = inimigo->tamanho.x * escalaX;
    float altura  = inimigo->tamanho.y * escalaY;
    Vector2 pivo = {
        inimigo->posicao.x + inimigo->tamanho.x / 2.0f + deslocX,
        inimigo->posicao.y + inimigo->tamanho.y - quique
    };

    if (inimigo->skin.valida) {
        if (!invisivel) DesenharCorpoSkin(inimigo, preparo, golpe, recupera, intensidade, morrendo);
    } else if (!invisivel) {
        if (!morrendo) DesenharPoeira(inimigo, intensidade);
        DrawEllipse((int)(inimigo->posicao.x + inimigo->tamanho.x / 2.0f), (int)(inimigo->posicao.y + inimigo->tamanho.y - 4.0f),
                    inimigo->tamanho.x * 0.34f * (1.0f - 0.25f * (quique / 34.0f)), inimigo->tamanho.x * 0.34f * 0.32f,
                    (Color){ 0, 0, 0, (unsigned char)(70 * opacidade) });

        Rectangle origem = { 0, 0, (float)inimigo->sprite.width, (float)inimigo->sprite.height };
        if (!inimigo->olhandoDireita) origem.width = -origem.width;

        Rectangle destino = { pivo.x, pivo.y, largura, altura };

        Color tinta = WHITE;
        if (inimigo->tempoDano > 0.0f) {
            tinta = (Color){ 255, 110, 110, 255 };
        } else if (aquecimento > 0.0f) {
            tinta = (Color){ 255, (unsigned char)(255 - 110 * aquecimento), (unsigned char)(255 - 150 * aquecimento), 255 };
        }
        tinta.a = (unsigned char)(255.0f * opacidade);

        DrawTexturePro(inimigo->sprite, origem, destino, (Vector2){ largura / 2.0f, altura }, inclinacao, tinta);
    }

    if (inimigo->tipo == INIMIGO_ARQUEIRO && !morrendo && inimigo->arco.id != 0) {
        Vector2 peito = PontoNoCorpo(pivo, largura, altura, inimigo->olhandoDireita, 0.5f, 0.5f, inclinacao);
        float puxar = (inimigo->estadoIA == IA_PREPARANDO) ? preparo : 0.0f;
        float angulo = inimigo->anguloMira + sinf(inimigo->fasePasso) * 0.07f * intensidade;
        float afasta = 20.0f - 5.0f * puxar;
        Vector2 mao = { peito.x + cosf(angulo) * afasta, peito.y + sinf(angulo) * afasta };

        Rectangle origemArco = { 0, 0, (float)inimigo->arco.width, (float)inimigo->arco.height };
        Rectangle destinoArco = { mao.x, mao.y, inimigo->arco.width * 0.85f, inimigo->arco.height * 0.85f };
        Vector2 pivoArco = { 10.0f * 0.85f, inimigo->arco.height * 0.85f / 2.0f };

        if (inimigo->estadoIA == IA_PREPARANDO || inimigo->estadoIA == IA_ATACANDO) {
            float recuoFlecha = 14.0f * puxar;
            Vector2 cabo = { mao.x - cosf(angulo) * recuoFlecha, mao.y - sinf(angulo) * recuoFlecha };
            Vector2 ponta = { cabo.x + cosf(angulo) * 44.0f, cabo.y + sinf(angulo) * 44.0f };
            if (inimigo->estadoIA == IA_PREPARANDO) DrawLineEx(cabo, ponta, 3.0f, (Color){ 150, 100, 55, 255 });
        }

        DrawTexturePro(inimigo->arco, origemArco, destinoArco, pivoArco, angulo * RAD2DEG, (Color){ 255, 255, 255, 255 });
    }

    if (!morrendo && inimigo->tipo != INIMIGO_CHEFE) {
        float larguraBarra = inimigo->tamanho.x;
        float alturaBarra  = 6.0f;
        float barraX = inimigo->posicao.x;
        float barraY = inimigo->posicao.y - 12.0f;

        float porcentagem = (float)inimigo->vida / (float)inimigo->vidaMaxima;
        if (porcentagem < 0.0f) porcentagem = 0.0f;

        DrawRectangle((int)barraX - 1, (int)barraY - 1, (int)larguraBarra + 2, (int)alturaBarra + 2, BLACK);
        DrawRectangle((int)barraX, (int)barraY, (int)larguraBarra, (int)alturaBarra, (Color){ 90, 20, 20, 255 });
        DrawRectangle((int)barraX, (int)barraY, (int)(larguraBarra * porcentagem), (int)alturaBarra, (Color){ 220, 40, 40, 255 });

        char textoNivel[24];
        snprintf(textoNivel, sizeof(textoNivel), "%s Nv %d", (inimigo->tipo == INIMIGO_ARQUEIRO) ? "Arq" : "Orc", inimigo->nivel);
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
