/* ============================================================
 *  inimigo.h
 *  INTERFACE PÚBLICA do módulo Inimigo (Orc).
 * ============================================================ */
#ifndef INIMIGO_H
#define INIMIGO_H

#include <stdbool.h>
#include "raylib.h"

typedef struct {
    Vector2 posicao;        // canto superior esquerdo do orc na tela
    Vector2 tamanho;        // tamanho com que o orc é DESENHADO
    int     vida;           // vida atual (0 ou menos = morto, não é mais desenhado)
    int     vidaMaxima;     // vida inicial (usada na barrinha de vida)
    int     nivel;          // nível do orc (1, 2, 3...) - quanto maior, mais vida
    Texture2D sprite;       // cópia da textura COMPARTILHADA (não descarregue por aqui!)
    bool    olhandoDireita; // true = sprite original; false = espelhado
    float   tempoDano;      // segundos restantes do "flash" vermelho ao levar dano
    float   velocidade;     // pixels por segundo ao perseguir o alvo

    bool    andando;
    float   fasePasso;
    float   intensidadeAndar;
    float   tempoAnimacao;

    Vector2 recuo;
    float   tempoAtordoado;
    int     ultimoDano;
    float   tempoTextoDano;
    float   tempoMorte;
    int     ladoQueda;
} Inimigo;

// Carrega a imagem do orc UMA vez (todos os orcs usam a mesma textura)
Texture2D CarregarSpriteOrc(void);

// Preenche a struct: posição, nível, vida, tamanho...
void IniciarInimigo(Inimigo *inimigo, Texture2D sprite, Vector2 posicao, int nivel);

// Atualiza timers do inimigo (flash de dano). Chame todo frame.
void AtualizarInimigo(Inimigo *inimigo, float deltaTime);

void MoverInimigo(Inimigo *inimigo, Vector2 alvo, float deltaTime, Rectangle limites);

void AfastarInimigos(Inimigo *a, Inimigo *b, Rectangle limites);

int DanoContatoInimigo(const Inimigo *inimigo);

// Retorna true se o inimigo está vivo e o CENTRO dele está a no máximo
// "alcance" pixels do ponto "alvo" (ex.: centro do player)
bool InimigoPertoDe(const Inimigo *inimigo, Vector2 alvo, float alcance);

// Tira "dano" de vida do inimigo. Retorna true se este golpe MATOU o orc.
bool DanoInimigo(Inimigo *inimigo, int dano);

bool GolpearInimigo(Inimigo *inimigo, int dano, Vector2 origem);

// Desenha o orc + barra de vida + nível (não desenha nada se estiver morto)
void DesenharInimigo(const Inimigo *inimigo);

// Libera a textura (chame UMA vez no fim do jogo)
void DescarregarSpriteOrc(Texture2D sprite);

#endif /* INIMIGO_H */