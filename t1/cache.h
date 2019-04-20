/*
**   Celso Antonio Uliana Junior,
**   April 2019
*/
#ifndef _CACHE_H
#define _CACHE_H
#include <map>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std; 

/*
###############################################################################
Seção de declaração da estrutura de dados referente ao cache.
###############################################################################
*/

// Objeto de dados da estrutura de dados da cache.
struct SDados {
    char * body;
    char * url;
    long long int tamanhoDados;
};

typedef struct SDados dados;

// Nó da lista duplamente encadeada que contem uma da
// estruturas de dados da cache.
struct SNoListaCache {

    // Organização da Nó da lista.
    dados dados;
    struct SNoListaCache * direita;
    struct SNoListaCache * esquerda;
};

typedef struct SNoListaCache noListaCache;

// Estrutura final de dados do cache.
struct SCacheFinal {   
    noListaCache * cabeca;
    long long int tamanho;
};

typedef struct SCacheFinal cacheFinal;

/*
###############################################################################
Seção de funcões para funcionalidade do cache.
###############################################################################
*/

/*  Inicializa um cache com o tamanho desejado. */
cacheFinal * criaCache(int tamanhoCache);

/*  Adiciona um novo nó no cache. */
int adicionaNoCache(noListaCache * noCache, cacheFinal * cache);

/*  Remove um nó no cache. */
int removeNoCache(noListaCache * noCache, cacheFinal * cache);

/*  Acha um nó na lista. */
noListaCache * achaNoCache(char *url, cacheFinal * cache);

/* Printa a lista encadeada de nós */
void printCache(cacheFinal * cache);

#endif