/*
**   Celso Antonio Uliana Junior,
**   April 2019
*/
#include "cache.h"


/*  
    Variavel passada no segundo argumento do listen(). 
    Serve como quantidade maxima de requisições de conexão.
*/
#define MAX_PENDING 10
#define MAX_LINE 512  
#define TAMANHO_BUFFER 8192  
#define TAMANHO_MAX 8192  


/*
###############################################################################
Seção de Wrappers de funções de memoria.
###############################################################################
*/
void *Malloc(size_t size);

void *Realloc(void *ptr, size_t size);

/*
###############################################################################
Seção de declaração de assinaturas de métodos.
###############################################################################
*/

/*  
    Encapsula toda a parte de criar socket, bindar a uma porta
    passada como argumento e escuta a porta passivamente.
*/
int encapsulaListen(int port);

/*  
    Encapsula toda a parte de criar socket, e conexão da parte
    Cliente, porém thread safe para o método gethostbyname()
*/
int encapsulaClienteTs(int port, char * host);

/*  
    Inicialização da thread, função passada para pthread_create(),
    Onde acontece o pthread_detach para que requisições 
    Simultaneas funcionem.
*/
void *threadInit(void * cliente);

/*  
    Metodo que faz o parsing da requisicao, e busca no cache,
    Caso ache, retorna para o cliente, caso não, adiciona ao
    Cache.
*/
void parseAndCache(int conexao);

/*
###############################################################################
Seção de declaração de métodos do pacote RIO.
###############################################################################
*/

/*  Função de escrever dados encapsulada */
ssize_t rio_writen(int fd, void * usrbuf, size_t n);

/*  Função de escrever dados encapsulada */
ssize_t rio_readn(int fd, char * usrbuf, size_t n);

/*
###############################################################################
Seção de declaração de variaveis globais, como mutex e o cache.
###############################################################################
*/

/*  Mutex */
static sem_t mutexHost;
static sem_t mutexCache;

/*  Cache */
cacheFinal * cache;

/*  Portas */
int portaProxy;
int portaCliente;

/*  Mapa de endereços do cache */
map<string, noListaCache *> mapa;

/*  Iterador do mapa de endereços */
map<string, noListaCache *>::iterator it;

/*
###############################################################################
Seção de Wrappers de funções de memoria.
###############################################################################
*/
void *Malloc(size_t size) 
{
    void *p;

    if ((p  = malloc(size)) == NULL)
	    printf("Malloc error\n");
    return p;
}

void *Realloc(void *ptr, size_t size) 
{
    void *p;

    if ((p  = realloc(ptr, size)) == NULL)
	    printf("Realloc error\n");
    return p;
}

/*
###############################################################################
Seção de Implementação dos métodos declarados acima.
###############################################################################
*/

int encapsulaListen(int port){

    struct sockaddr_in sin;
    int s;

    /*  Constroi estrutura de dados de endereço */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);


    /*  Abre um socket passivo */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }

    /*  Binda o socket a uma porta */
    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("simplex-talk: bind");
        exit(1);
    }
    
    /*  Escuta na porta */
    if(listen(s, MAX_PENDING) < 0){
        perror("simplex-talk: listen");
        exit(1);
    }

    return s;
}

int encapsulaClienteTs(int port, char * host){

    int cliente, flag;
    struct hostent * sharedp;
    struct sockaddr_in serveraddr;

    if ((cliente = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    return -1; /* check errno for cause of error */

    /*  Entrada da sessão critica */
    sem_wait(&mutexHost);

    /*  Abre o IP do server e a porta. */
    if ((sharedp = gethostbyname(host)) == NULL)
	    flag = -2; /* check h_errno for cause of error */

    sem_post(&mutexHost);
    /*  Saida da sessão critica  */

    if(flag == -2){
        return flag;
    }

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)sharedp->h_addr_list[0], 
	  (char *)&serveraddr.sin_addr.s_addr, sharedp->h_length);
    serveraddr.sin_port = htons(port);

    /*  Abre a conexão com o servidor */
    if (connect(cliente, (sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
	    return -1;

    return cliente;

}

void * threadInit(void * cliente){

    int conexaoCliente = *((int *) cliente);

    /*  
        Deatach, separa a thread atual da thread main, sendo assim, não é
        Necessário o uso de Pthread_join().
    */
    pthread_detach(pthread_self());
    //free(cliente);

    parseAndCache(conexaoCliente);

    close(conexaoCliente);

    return NULL;
}

ssize_t rio_writen(int fd, char * usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten;
    char * bufp =  usrbuf;

    while (nleft > 0) {
	if ((nwritten = write(fd, bufp, nleft)) <= 0) {
	    if (errno == EINTR)  /* interrupted by sig handler return */
		nwritten = 0;    /* and call write() again */
	    else
		return -1;       /* errorno set by write() */
	}
	nleft -= nwritten;
	bufp += nwritten;
    }
    return n;
}

ssize_t rio_readn(int fd, char * usrbuf, size_t n) {
    size_t nleft = n;
    ssize_t nread;
    char * bufp = usrbuf;

    while (nleft > 0) {
	if ((nread = read(fd, bufp, nleft)) < 0) {
	    if (errno == EINTR) /* interrupted by sig handler return */
		nread = 0;      /* and call read() again */
	    else
		return -1;      /* errno set by read() */ 
	} 
	else if (nread == 0)
	    break;              /* EOF */
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}

void parseAndCache(int conexao){

    char buf[TAMANHO_BUFFER];
    char * hostname;
    char * nome = (char *) Malloc(TAMANHO_MAX * sizeof(char));
    char * metodo = (char *) Malloc(TAMANHO_MAX * sizeof(char));
    char * caminho = (char *) Malloc(TAMANHO_MAX * sizeof(char));
    char * requisicao = (char *) Malloc(TAMANHO_MAX * sizeof(char));


    while(recv(conexao, buf, sizeof(buf), 0) < 0){
        //printf("%s\n", buf);
    }
    

    /* Lê a primeira linha o formato de dados descrito de uma string. */
    sscanf(buf, "%s %s ", metodo, nome);
    
    strcat(requisicao, buf);

    if(strcasecmp(metodo, "GET") == 0){

        /* Muda o HTTP 1.1 para HTTP 1.0    */
        char * req;
        if(req = strstr(requisicao, "HTTP/1.1"))
            strncpy(req, "HTTP/1.0", strlen("HTTP/1.0"));

		if((req = strstr(buf, "Proxy-Connection: ")))
			strcpy(buf, "Proxy-Connection: close\r\n");

		else if((req = strstr(buf, "Connection:")))
			strcpy(buf, "Connection: close\r\n");

        strcat(requisicao, buf);
        strcat(requisicao, "\r\n");

        portaCliente = 80;   

        char * aux;
        strcpy(caminho, "/");

        hostname = strstr(nome, "/");

        /* 
            Ajusta o hostname para ser apropriado na hora de abrir
            a conexão.
        */
        aux = index(hostname,':');
        if(aux == NULL){
            hostname = (char *) strtok(hostname, "/");
        }
        else{
            hostname = (char *) strtok(hostname, ":");
        }

        sem_wait(&mutexCache);
        it = mapa.find(nome);
        sem_post(&mutexCache);

        if(it == mapa.end()){

            printf("Cache miss\n");
            printf("Url : %s\n", nome);
            noListaCache * novo = (noListaCache *) Malloc(sizeof(noListaCache));
            novo -> dados.url = nome;
            novo -> dados.body = NULL;
            novo -> direita = NULL;
            novo -> esquerda = NULL;

            int conexaoServer = encapsulaClienteTs(portaCliente, hostname);
            req = strstr(requisicao, "GET");

            //printf("%s", req);

            rio_writen(conexaoServer, req, strlen(req));

            req = NULL;

            int tam = 0, n;

            while((n = rio_readn(conexaoServer, buf, TAMANHO_BUFFER)) > 0 ) {

                /*  Alocação do tamanho do body do site. */
                novo -> dados.body = (char *) Realloc(novo -> dados.body, n + n * sizeof(char));

                //printf("vou copiar os dados para o cliente\n");

                /*  Copia do body do site.   */
                memcpy(novo -> dados.body + tam, buf, n);

                /*  Tamanho que irá ocupar em memoria.   */
                tam += n;

                /*  Escrita real para o cliente */
                rio_writen(conexao, buf, n);

                /*  Limpa buffer */
                bzero(buf, TAMANHO_BUFFER);
            }

            /*  Acesso a sessão critica(cache)  */
            sem_wait(&mutexCache);
            novo -> dados.tamanhoDados = tam;
            cache -> tamanhoAtual += tam;

            printf("Tamanho atual do cache =  %d ------ Tamanho Limite do cache = %d\n",
             cache -> tamanhoAtual, cache -> tamanhoLimite, cache -> tamanhoLimite);

            /*  
                Abre espaço necessario em cache, pois a ordem da lista encadeada
                simboliza a ordem de uso, quando um nó é usado, ele é removido
                e reinserido no começo da lista. Sendo assim para liberar espaço
                Só é necessário ir removendo os nós caudas. 
            */
            
            while(cache -> tamanhoAtual > cache -> tamanhoLimite){

                /*  Acha a cauda pelo mapa */
                it = mapa.find(cache -> cauda -> dados.url);

                /*  Atualiza o novo valor */
                cache -> tamanhoAtual -= cache -> cauda -> dados.tamanhoDados;

                /*  Remove do cache, e em seguida, do cache */
                removeNoCache(cache -> cauda, cache);

                if(it != mapa.end())
                    mapa.erase(it);
            }

            if(!adicionaNoCache(novo, cache)){
                printf("Falha na inserção no cache.\n");
            }

            /*  Adiciona no mapa */
            mapa.insert(make_pair(nome, novo));
            
            sem_post(&mutexCache);
            //printCache(cache);
            close(conexaoServer);
        }

        
        else{
            /*  Acesso a sessão critica     */
            sem_wait(&mutexCache);

            printf("Tamanho atual do cache =  %d ------ Tamanho Limite do cache = %d\n",
             cache -> tamanhoAtual, cache -> tamanhoLimite);
            printf("Cache hit\n");
            //printCache(cache);

            it = mapa.find(nome);

            if(it != mapa.end()){
                printf("Url : %s\n", mapa[nome] -> dados.url);
                rio_writen(conexao, mapa[nome] -> dados.body, mapa[nome] -> dados.tamanhoDados);
            }

            /*  
                Remove o nó da sua posição atual e insere ele novamente
                no começo da lista, para simbolizar que foi acessado.
                Custo remoção O(1) e inserção O(1).
            */

            noListaCache * aux = mapa[nome];

            AdicionaNoExistenteNaCabeca(aux, cache);   

            sem_post(&mutexCache);
        }    
    }

    else
        printf("Metodo não GET - Proxy só implementa metódos GET.\n");
    
    free(metodo);
    free(requisicao);

    
    //printCache(cache);
}

/*
###############################################################################
Seção da main.
###############################################################################
*/

int main(int argc, char **argv){
    struct sockaddr_in sin;
    int len, s, new_s, cacheSize;
    pthread_t tid;

    /*  Inicia o semaforo */
    sem_init(&mutexHost, 0, 1);
    sem_init(&mutexCache, 0, 1);


    if (argc != 3) {
		fprintf(stderr, "usage: %s <porta> <cache size in MB>\n", argv[0]);
		exit(1);
	}

    portaProxy = atoi(argv[1]);
    cacheSize = atoi(argv[2]);

    s = encapsulaListen(portaProxy);
    cache = criaCache(cacheSize);

    printf("tamanho do cache = %d\n", cache -> tamanhoLimite);

    /*  wait for connection, then receive and print text */
    while(1) {
        
        if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
            perror("simplex-talk: accept");
            free(cache);
            exit(1);
        }
        
        else{
            
            if (pthread_create(&tid, NULL, threadInit, &new_s) != 0) {
                perror("pthread_create() error");
                free(cache);
                exit(1);
            }
        }
    }
}