#ifndef INIMIGO_H
#define INIMIGO_H

#include "raylib.h"
#include <stdbool.h>
#include "skin_orc.h"

typedef enum {
    INIMIGO_ORC,
    INIMIGO_ARQUEIRO,
    INIMIGO_CHEFE
} TipoInimigo;

typedef enum {
    IA_PERSEGUINDO,
    IA_PREPARANDO,
    IA_ATACANDO,
    IA_RECUPERANDO
} EstadoIA;

typedef enum {
    EVENTO_NENHUM,
    EVENTO_GOLPE_CORPO,
    EVENTO_DISPARO
} EventoInimigo;

typedef struct {
    Texture2D orc;
    Texture2D arqueiro;
    Texture2D chefe;
    Texture2D arco;
    Texture2D flecha;
    SkinOrc skinOrc;
    SkinOrc skinOrcAlt;
    SkinOrc skinChefe;
} SpritesInimigos;

typedef struct {
    TipoInimigo tipo;
    EstadoIA estadoIA;
    float tempoEstado;
    float anguloMira;
    Vector2 posicao;
    Vector2 tamanho;
    int vida;
    int vidaMaxima;
    int nivel;
    Texture2D sprite;
    Texture2D arco;
    SkinOrc skin;
    float faseQuadro;
    bool olhandoDireita;
    float tempoDano;
    float velocidade;
    bool andando;
    float fasePasso;
    float intensidadeAndar;
    float tempoAnimacao;
    Vector2 recuo;
    float tempoAtordoado;
    int ultimoDano;
    float tempoTextoDano;
    float tempoMorte;
    int ladoQueda;
} Inimigo;

SpritesInimigos CarregarSpritesInimigos(void);
void DescarregarSpritesInimigos(SpritesInimigos sprites);

void IniciarInimigo(Inimigo *inimigo, TipoInimigo tipo, const SpritesInimigos *sprites, Vector2 posicao, int nivel);
void AtualizarInimigo(Inimigo *inimigo, float deltaTime);
EventoInimigo AtualizarIAInimigo(Inimigo *inimigo, Vector2 peAlvo, float deltaTime, Rectangle limites);
void AfastarInimigos(Inimigo *a, Inimigo *b, Rectangle limites);

Vector2 PeDoInimigo(const Inimigo *inimigo);
Vector2 PontaDoArco(const Inimigo *inimigo);
bool GolpeAtingeAlvo(const Inimigo *inimigo, Vector2 peAlvo);
int DanoCorpoInimigo(const Inimigo *inimigo);
int DanoFlechaInimigo(const Inimigo *inimigo);
int PontosInimigo(const Inimigo *inimigo);
int ChanceDropInimigo(const Inimigo *inimigo);

bool InimigoPertoDe(const Inimigo *inimigo, Vector2 alvo, float alcance);
bool DanoInimigo(Inimigo *inimigo, int dano);
bool GolpearInimigo(Inimigo *inimigo, int dano, Vector2 origem);

void DesenharTelegrafoInimigo(const Inimigo *inimigo);
void DesenharInimigo(const Inimigo *inimigo);

#endif
