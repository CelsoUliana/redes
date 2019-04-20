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
    cache -> tamanho = tamanhoCache << 20; 

}

int adicionaNoCache(noListaCache * noCache, cacheFinal * cache){
    
    if(noCache == NULL)
        return 0;

    if(cache -> cabeca == NULL){
        cache -> cabeca = noCache;
    }
    
    else{
        noListaCache * aux = cache -> cabeca;

        while(aux -> direita != NULL){
            printf("alo\n");
            aux = aux -> direita;
        }

        noCache -> direita = aux -> direita;
        aux -> direita = noCache;
        noCache -> esquerda = aux;

    }

    return 1;
}

int removeNoCache(noListaCache* noCache, cacheFinal* cache){
    noListaCache* aux;



    return 1;
}

noListaCache* achaNoCache(char *url, cacheFinal* cache){
    return NULL;
}

void printCache(cacheFinal * cache){
    noListaCache * aux = cache -> cabeca;

    printf("Começo da lista:\n");

    while(aux != NULL){
        printf("url = %s ->  ", aux -> dados.url);
        aux = aux -> direita;
    }
}