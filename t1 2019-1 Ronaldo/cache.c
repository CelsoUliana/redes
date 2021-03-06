/*
**   Celso Antonio Uliana Junior,
**   April 2019
*/
#include "cache.h"

cacheFinal * criaCache(int tamanhoCache){
    cacheFinal * cache = (cacheFinal *) malloc (sizeof(cacheFinal));

    /*  Erro ao criar o cache.   */
    if(cache == NULL){
        perror("Erro ao criar o cache");
        exit(1);
    }

    //cache -> tamanhoLimite = 1000;
    /*  Converte de MB para o tamanho verdadeiro */
    cache -> tamanhoLimite = tamanhoCache << 20;
    cache -> tamanhoAtual = 0;
    cache -> cauda = cache -> cabeca = NULL;
}

int adicionaNoCache(noListaCache * noCache, cacheFinal * cache){
    
    if(noCache == NULL || cache == NULL)
        return 0;

    if(cache -> cabeca == NULL && cache -> cauda == NULL){
        cache -> cabeca = noCache;
        cache -> cauda = noCache;
    }
    
    else{
        noCache -> direita = cache -> cabeca;
        cache -> cabeca = noCache;
    }

    return 1;
}

int AdicionaNoExistenteNaCabeca(noListaCache * noCache, cacheFinal * cache){

    if(noCache -> direita != NULL)
        noCache -> direita -> esquerda = noCache -> esquerda;

    else
        cache -> cauda = noCache -> esquerda;
    
    if(noCache -> esquerda != NULL)
        noCache -> esquerda -> direita = noCache -> direita;

    else
        cache -> cabeca = noCache -> direita;

    noCache -> esquerda = NULL;
    noCache -> direita = cache -> cabeca;
    cache -> cabeca = noCache;
}

int removeNoCache(noListaCache * noCache, cacheFinal * cache){

    if(noCache == NULL)
        return 0;

    if(noCache -> direita != NULL)
        noCache -> direita -> esquerda = noCache -> esquerda;
    
    else
        cache -> cauda = noCache -> esquerda;

    if(noCache -> esquerda != NULL)
        noCache -> esquerda -> direita = noCache -> direita;

    else
        cache -> cabeca = noCache -> direita;

    
    free(noCache);

    noCache = NULL;

    return 1;
}

void printCache(cacheFinal * cache){
    noListaCache * aux = cache -> cabeca;

    if(cache -> cabeca != NULL && cache -> cauda != NULL){
        printf("cabeca = %s\ncauda = %s\n", cache -> cabeca -> dados.url, cache -> cauda -> dados.url);
    }

    while(aux != NULL){
        printf("%s ->  \t", aux -> dados.url);
        aux = aux -> direita;
    }

    printf("\n");
}