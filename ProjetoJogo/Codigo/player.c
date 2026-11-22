/* ============================================================
 *  player.c
 *  Implementação do módulo Player (movimento, limites e animação).
 *
 *  O sprite sheet "player_walk.png" é uma grade de 5 colunas x 2 linhas.
 *  Os frames são numerados da esquerda p/ direita, de cima p/ baixo:
 *      0 1 2 3 4
 *      5 6 7 8 9
 * ============================================================ */
#include "player.h"
#include "config.h"      // LARGURA_JANELA e ALTURA_JANELA
#include "raymath.h"     // Vector2Normalize

#define FRAMES_COLUNAS  5      // quantos frames existem na horizontal
#define FRAMES_LINHAS   2      // quantos frames existem na vertical
#define TOTAL_FRAMES    10     // FRAMES_COLUNAS * FRAMES_LINHAS
#define FPS_ANIMACAO    12.0f  // frames trocados por segundo (maior = pernas mais rápidas)
#define FRAME_PARADO    0      // frame mostrado quando o jogador não está andando
#define ALTURA_PLAYER   104.0f // altura com que o player aparece na tela

void IniciarPlayer(Player *player)
{
    // Carrega o sprite sheet de caminhada (o arquivo fica em assets/)
    player->sprite = LoadTexture("assets/player_walk.png");

    // Descobre o tamanho de um frame dividindo a imagem pela quantidade de frames.
    player->larguraFrame = (float)player->sprite.width  / FRAMES_COLUNAS;
    player->alturaFrame  = (float)player->sprite.height / FRAMES_LINHAS;

    // CORREÇÃO: a largura na tela agora é calculada a partir da proporção REAL
    // do frame. Antes ela era fixa (85x104), o que deixava o anão esticado,
    // porque cada frame do sprite sheet tem proporção diferente dessa.
    // Se a textura não carregou (altura 0), usa uma proporção padrão.
    float proporcao = (player->alturaFrame > 0.0f)
                    ? player->larguraFrame / player->alturaFrame
                    : 0.72f;
    player->tamanho = (Vector2){ ALTURA_PLAYER * proporcao, ALTURA_PLAYER };

    // Começa no centro da janela
    player->posicao = (Vector2){
        LARGURA_JANELA / 2.0f - player->tamanho.x / 2.0f,
        ALTURA_JANELA  / 2.0f - player->tamanho.y / 2.0f
    };

    player->velocidade = 250.0f; // pixels por segundo

    // Estado inicial da animação: parado, olhando para a direita, no frame 0
    player->frameAtual     = FRAME_PARADO;
    player->tempoFrame     = 0.0f;
    player->andando        = false;
    player->olhandoDireita = true;
}

void AtualizarPlayer(Player *player, const Arena *arena)
{
    // deltaTime = segundos que passaram desde o frame anterior do JOGO.
    // Usamos tanto para mover quanto para animar, assim nada depende do FPS.
    float deltaTime = GetFrameTime();

    Vector2 direcao = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    direcao.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  direcao.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  direcao.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direcao.x += 1.0f;

    // Se a direção é diferente de (0,0), o jogador está tentando andar.
    // Guardamos isso ANTES de normalizar, só para a animação saber.
    player->andando = (direcao.x != 0.0f || direcao.y != 0.0f);

    // Só troca o lado para o qual o anão olha quando há movimento HORIZONTAL.
    // Assim, ao andar só para cima/baixo ele mantém o último lado.
    if (direcao.x > 0.0f) player->olhandoDireita = true;
    if (direcao.x < 0.0f) player->olhandoDireita = false;

    if (player->andando) {
        direcao = Vector2Normalize(direcao); // diagonal não pode ser mais rápida
    }

    player->posicao.x += direcao.x * player->velocidade * deltaTime;
    player->posicao.y += direcao.y * player->velocidade * deltaTime;

    /* ---- Limites da arena ---- */
    if (player->posicao.x < arena->limites.x)
        player->posicao.x = arena->limites.x;

    if (player->posicao.y < arena->limites.y)
        player->posicao.y = arena->limites.y;

    if (player->posicao.x + player->tamanho.x >
        arena->limites.x + arena->limites.width)
    {
        player->posicao.x =
            arena->limites.x + arena->limites.width - player->tamanho.x;
    }

    if (player->posicao.y + player->tamanho.y >
        arena->limites.y + arena->limites.height)
    {
        player->posicao.y =
            arena->limites.y + arena->limites.height - player->tamanho.y;
    }

    /* ---- Animação ---- */
    if (player->andando) {
        // Quanto tempo cada frame fica na tela: 1 / 12 ≈ 0,083 s
        float duracaoFrame = 1.0f / FPS_ANIMACAO;

        player->tempoFrame += deltaTime;

        // "while" (e não "if") para não perder frames caso o jogo dê uma travada
        while (player->tempoFrame >= duracaoFrame) {
            player->tempoFrame -= duracaoFrame;

            // % TOTAL_FRAMES faz o contador voltar a 0 depois do frame 9 (ciclo infinito)
            player->frameAtual = (player->frameAtual + 1) % TOTAL_FRAMES;
        }
    } else {
        // Parou de andar: volta para a pose parada e zera o cronômetro,
        // assim a próxima caminhada sempre começa do começo do ciclo.
        player->frameAtual = FRAME_PARADO;
        player->tempoFrame = 0.0f;
    }
}

void DesenharPlayer(const Player *player)
{
    // Converte o número do frame (0 a 9) em coluna e linha na grade:
    //   coluna = resto da divisão por 5  |  linha = divisão inteira por 5
    // Ex.: frame 7 -> coluna 2, linha 1
    int coluna = player->frameAtual % FRAMES_COLUNAS;
    int linha  = player->frameAtual / FRAMES_COLUNAS;

    // "origem" = o retângulo DENTRO do sprite sheet que será recortado
    Rectangle origem = {
        coluna * player->larguraFrame,
        linha  * player->alturaFrame,
        player->larguraFrame,
        player->alturaFrame
    };

    // Truque do raylib: largura NEGATIVA na origem espelha a imagem na horizontal.
    // O sprite original olha para a direita, então espelhamos ao ir para a esquerda.
    if (!player->olhandoDireita) {
        origem.width = -origem.width;
    }

    // "destino" = onde e com que tamanho o recorte aparece na tela
    Rectangle destino = { player->posicao.x, player->posicao.y, player->tamanho.x, player->tamanho.y };

    DrawTexturePro(player->sprite, origem, destino, (Vector2){ 0, 0 }, 0.0f, WHITE);
}

void FinalizarPlayer(Player *player)
{
    UnloadTexture(player->sprite);
}
