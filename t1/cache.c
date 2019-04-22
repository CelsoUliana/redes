/*
**   Celso Antonio Uliana Junior,
**   April 2019
*/
#include "cache.h"

cacheFinal * criaCache(int tamanhoCache){
    cacheFinal * cache = (cacheFinal *) malloc (sizeof(cacheFinal));

    // Erro ao criar o cache.
    if(cache == NULL){
        perror("Erro ao criar o cache");
        exit(1);
    }

    // Converte de MB para o tamanho verdadeiro 
    
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
        noCache -> esquerda = cache -> cabeca -> esquerda;
        cache -> cabeca = noCache;

        /*
        noListaCache * aux = cache -> cabeca;

        while(aux -> direita != NULL){
            aux = aux -> direita;
        }

        noCache -> direita = aux -> direita;
        aux -> direita = noCache;
        noCache -> esquerda = aux;
        */
    }

    return 1;
}

int removeNoCache(noListaCache * noCache, cacheFinal * cache){

    noListaCache * aux = noCache;

    if(aux == NULL)
        return 0;


    if(aux -> direita != NULL)
        aux -> direita -> esquerda = aux -> esquerda;
    
    else
        cache -> cauda = aux -> esquerda;

    if(aux -> esquerda != NULL)
        aux -> esquerda -> direita = aux -> direita;

    else
        cache -> cabeca = aux -> direita;

    free(aux);

    return 1;
}

void printCache(cacheFinal * cache){
    noListaCache * aux = cache -> cabeca;

    //printf("Começo da lista:\n");

    if(cache -> cabeca != NULL && cache -> cauda != NULL){
        printf("cabeca = %s\ncauda = %s\n", cache -> cabeca -> dados.url, cache -> cauda -> dados.url);
    }

    while(aux != NULL){
        printf("%s ->  \t", aux -> dados.url);
        aux = aux -> direita;
    }

    printf("\n");
}