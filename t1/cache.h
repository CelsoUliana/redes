/*
**   Celso Antonio Uliana Junior,
**   April 2019
*/
#ifndef _CACHE_H
#define _CACHE_H

/*
###############################################################################
Seção de declaração da estrutura de dados referente ao cache.
###############################################################################
*/

// Thread lock 
pthread_rwlock_t lock;

// Objeto de dados da estrutura de dados da cache.
struct Dados {
    char * url;
};

// Nó da lista duplamente encadeada que contem uma da
// estruturas de dados da cache.
struct NoListaCache {

    // Organização da Nó da lista.
    Dados dados;
    Dados * direita;
    Dados * esquerda;
};

// Estrutura final de dados do cache.
struct CacheFinal {
    NoListaCache cabeca;
    NoListaCache cauda;
    int tamanhoCache;
};

/*
###############################################################################
Seção de funcões para funcionalidade do cache.
###############################################################################
*/

// Adiciona um novo nó no cache.
int adicionaNoCache(NoListaCache noCache, CacheFinal * cache);

// Remove um nó no cache.
int removeNoCache(NoListaCache * noCache, CacheFinal * cache);

// Acha um nó na lista.
NoListaCache * achaNoCache(char *url, CacheFinal * cache);


#endif