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

// Thread lock global para uso no trabalho.
extern pthread_rwlock_t lock;

// Objeto de dados da estrutura de dados da cache.
struct SDados {
    char * body;
    char * url;
    long long int tamanhoDados;
};

typedef struct SDados Dados;

// Nó da lista duplamente encadeada que contem uma da
// estruturas de dados da cache.
struct SNoListaCache {

    // Organização da Nó da lista.
    Dados dados;
    Dados * direita;
    Dados * esquerda;
};

typedef struct SNoListaCache NoListaCache;

// Estrutura final de dados do cache.
struct SCacheFinal {
    map<char *, NoListaCache> enderecos;
    // Talvez
    // Map<char [], NoListaCache> enderecos543'wsd    // ou
    
    NoListaCache cabeca;
    long long int tamanho;
};

typedef struct SCacheFinal CacheFinal;

/*
###############################################################################
Seção de funcões para funcionalidade do cache.
###############################################################################
*/

/*  Inicializa um cache com o tamanho desejado. */
CacheFinal * criaCache(int tamanhoCache);

/*  Adiciona um novo nó no cache. */
int adicionaNoCache(NoListaCache noCache, CacheFinal * cache);

/*  Remove um nó no cache. */
int removeNoCache(NoListaCache * noCache, CacheFinal * cache);

/*  Acha um nó na lista. */
NoListaCache * achaNoCache(char *url, CacheFinal * cache);


#endif
