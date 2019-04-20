/*
**   Celso Antonio Uliana Junior,
**   April 2019
*/
#include "cache.h"

CacheFinal * criaCache(int tamanhoCache){
    CacheFinal * cache = (CacheFinal *) malloc (sizeof(CacheFinal));

    // Erro ao criar o cache.
    if(cache == NULL){
        perror("Erro ao criar o cache");
        exit(1);
    }

    // Converte de MB para o tamanho verdadeiro 
    cache -> tamanho = tamanhoCache << 20; 

}

int adicionaNoCache(NoListaCache noCache, CacheFinal* cache){
    return 1;
}

int removeNoCache(NoListaCache* noCache, CacheFinal* cache){
    NoListaCache* aux;



    return 1;
}

NoListaCache* achaNoCache(char *url, CacheFinal* cache){
    return NULL;
}
