#ifndef FILA_ONDAS_H
#define FILA_ONDAS_H

#include <stdbool.h>

typedef struct {
    int quantidade;
    int nivel;
} Onda;

typedef struct NoOnda {
    Onda onda;
    struct NoOnda *proximo;
} NoOnda;

typedef struct {
    NoOnda *inicio;
    NoOnda *fim;
    int tamanho;
} FilaOndas;

void FilaIniciar(FilaOndas *fila);
bool FilaEnfileirar(FilaOndas *fila, Onda onda);
bool FilaDesenfileirar(FilaOndas *fila, Onda *saida);
bool FilaVazia(const FilaOndas *fila);
void FilaLimpar(FilaOndas *fila);

#endif
