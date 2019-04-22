/*
**   Celso Antonio Uliana Junior,
**   April 2019
*/
#include "cache.h"


// Variavel passada no segundo argumento do listen(). 
// Serve como quantidade maxima de requisições de conexão.
#define MAX_PENDING 10
#define MAX_LINE 512  
#define TAMANHO_BUFFER 8192  
#define TAMANHO_MAX 8192  

/*
###############################################################################
Seção da struct do buffer do pacote RIO.
###############################################################################
*/
#define RIO_BUFSIZE 8192
typedef struct {
    int rio_fd;                /* descriptor for this internal buf */
    int rio_cnt;               /* unread bytes in internal buf */
    char *rio_bufptr;          /* next unread byte in internal buf */
    char rio_buf[RIO_BUFSIZE]; /* internal buffer */
} rio_t;

/*
###############################################################################
Seção de declaração de assinaturas de métodos.
###############################################################################
*/

/*  Encapsula toda a parte de criar socket, bindar a uma porta
    passada como argumento e escuta a porta passivamente.
*/
int encapsulaListen(int port);

/*  Encapsula toda a parte de criar socket, e conexão da parte
    Cliente.
*/
int encapsulaCliente(int port, char * host);

/*  Encapsula toda a parte de criar socket, e conexão da parte
    Cliente, porém thread safe para o método gethostbyname()
*/
int encapsulaClienteTs(int port, char * host);

/*  Inicialização da thread, função passada para pthread_create(),
    Onde acontece o pthread_detach para que requisições 
    Simultaneas funcionem.
*/
void *threadInit(void * cliente);


/*  Metodo que faz o parsing da requisicao, e busca no cache,
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

/*  Função de ler dados encapsulada */
ssize_t rio_readn(int fd, void * usrbuf, size_t n);

/*  Função de ler dados buffered */
ssize_t rio_readlineb(rio_t *rp, char *usrbuf, size_t maxlen);

/*  Wrapper de associação de conexão com o pointer rp */
void rio_readinitb(rio_t *rp, int fd);

/*  Wrapper da função read() Unix. */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n);

/*
###############################################################################
Seção de declaração de variaveis globais, como mutex e o cache.
###############################################################################
*/

/*  Mutex */
static sem_t mutexCache;
static sem_t mutexHost;

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

int encapsulaCliente(int port, char * host){

    int cliente;
    struct hostent * sharedp;
    struct sockaddr_in serveraddr;

    if ((cliente = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    return -1; /* check errno for cause of error */

    /*  Abre o IP do server e a porta. */
    if ((sharedp = gethostbyname(host)) == NULL)
	    return -2; /* check h_errno for cause of error */


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

int encapsulaClienteTs(int port, char * host){

    int cliente;
    struct hostent * sharedp;
    struct sockaddr_in serveraddr;

    if ((cliente = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    return -1; /* check errno for cause of error */

    /*  Entrada da sessão critica */
    sem_wait(&mutexHost);

    /*  Abre o IP do server e a porta. */
    if ((sharedp = gethostbyname(host)) == NULL)
	    return -2; /* check h_errno for cause of error */

    sem_post(&mutexHost);
    /*  Saida da sessão critica  */

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

    //  Deatach, separa a thread atual da thread main, sendo assim, não é
    //  Necessário o uso de Pthread_join().
    pthread_detach(pthread_self());

    //printf("connexão cliente num %d\n", conexao_cliente);

    parseAndCache(conexaoCliente);

    //recv(conexao_cliente, buf, sizeof(buf), 0);
    //fputs(buf, stdout);

    close(conexaoCliente);

    return NULL;
}

void rio_readinitb(rio_t *rp, int fd){
    rp->rio_fd = fd;  
    rp->rio_cnt = 0;  
    rp->rio_bufptr = rp->rio_buf;
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

ssize_t rio_readlineb(rio_t *rp, char *usrbuf, size_t maxlen) {
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) { 
	if ((rc = rio_read(rp, &c, 1)) == 1) {
	    *bufp++ = c;
	    if (c == '\n')
		break;
	} else if (rc == 0) {
	    if (n == 1)
		return 0; /* EOF, no data read */
	    else
		break;    /* EOF, some data was read */
	} else
	    return -1;	  /* error */
    }
    *bufp = 0;
    return n;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n){
    int cnt;

    while (rp->rio_cnt <= 0) {  /* refill if buf is empty */
	rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, 
			   sizeof(rp->rio_buf));
	if (rp->rio_cnt < 0) {
	    if (errno != EINTR) /* interrupted by sig handler return */
		return -1;
	}
	else if (rp->rio_cnt == 0)  /* EOF */
	    return 0;
	else 
	    rp->rio_bufptr = rp->rio_buf; /* reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;          
    if (rp->rio_cnt < n)   
	cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}

void parseAndCache(int conexao){

    char buf[TAMANHO_BUFFER];
    char * hostname;
    char * aux;
    char * caminho = (char *) malloc(MAX_LINE * sizeof(char));
    char * metodo = (char *) malloc(MAX_LINE * sizeof(char));
    char * nome = (char *) malloc(MAX_LINE * sizeof(char));
    char * restoDaRequisicao = (char *) malloc(MAX_LINE * sizeof(char));
    char * requisicao = (char *) malloc(TAMANHO_MAX * sizeof(char));

    /*  Declaracao dos ponteiros para utilização do pacote RIO */
    rio_t rioServidor;
	rio_t rioCliente;

    /*
    if(recv(conexao, buf, sizeof(buf), 0) < 0){
        printf("Erro de leitura\n");
    }
    */

    /*
	rio_readinitb(&rioCliente, conexao);

    while(rio_readn(conexao, buf, TAMANHO_BUFFER) > 0){
        printf("%s\n", buf);
    }
    */

    
    while(recv(conexao, buf, sizeof(buf), 0) < 0){
        printf("%s\n", buf);
    }
    

    /* Lê a primeira linha o formato de dados descrito de uma string. */
    sscanf(buf, "%s %s %s", metodo, nome, restoDaRequisicao);

    // printf("%s", buf);
    
    strcat(requisicao, buf);
	
   
    //printf("REQUISIÇÃO = \n%s\n", requisicao);

    if(strcasecmp(metodo, "GET") == 0){
        //printf("metodo = %s\n nome = %s\n resto = %s\n" , metodo, nome, restoDaRequisicao);


         // Muda o HTTP 1.1 para HTTP 1.0
        char * req;
        if(req = strstr(requisicao, "HTTP/1.1"))
            strncpy(req, "HTTP/1.0", strlen("HTTP/1.0"));

        strcat(requisicao, "\r\n");

        
        portaCliente = 80;   

        // Parsing.

        char * aux;
        strcpy(caminho, "/");

        hostname = strstr(nome, "/");

        //if(hostname != NULL)
            //printf("hostname = %s\n", hostname); 

        // Ajusta o hostname para ser apropriado na hora de abrir
        // a conexão.
        aux = index(hostname,':');
        if(aux == NULL){
            hostname = (char *) strtok(hostname, "/");
        }
        else{
            hostname = (char *) strtok(hostname, ":");
        }

        // Entrando na sessão critica.
        sem_wait(&mutexCache);
        it = mapa.find(nome);

        //printf("nome = %s\n hostname = %s\n", nome, hostname);

        if(it == mapa.end()){
            printf("Cache miss\n");
            noListaCache * novo = (noListaCache *) malloc(sizeof(noListaCache));
            novo -> dados.url = nome;
            novo -> direita = NULL;
            novo -> esquerda = NULL;

            int conexaoServer = encapsulaClienteTs(portaCliente, hostname);
            req = strstr(requisicao, "GET");

            rio_writen(conexaoServer, req, strlen(req));

            free(requisicao);

            //printf("conexão servidor = %d\n", conexao_server);

            int tam = 0, n;

            while((n = rio_readn(conexaoServer, buf, TAMANHO_BUFFER)) > 0 ) {
                // Alocação do tamanho do body do site.
                novo -> dados.body = (char *) realloc(novo -> dados.body, n + n * sizeof(char));

                // Copia do body do site.
                memcpy(novo -> dados.body + tam, buf, n);

                // Tamanho que irá ocupar em memoria.
                tam += n;
                rio_writen(conexao, buf, n);
                bzero(buf, TAMANHO_BUFFER);
            }

            novo -> dados.tamanhoDados = tam;

            cache -> tamanhoAtual += tam;

            printf("cache tamanho atual %d\n", cache -> tamanhoAtual);

            adicionaNoCache(novo, cache);
            mapa.insert(make_pair(nome, novo));

            close(conexaoServer);
        }

        
        else{
            printf("Cache hit\n");
            rio_writen(conexao, mapa[nome] -> dados.body, mapa[nome] -> dados.tamanhoDados);
            //printf("%s\n", mapa[nome] -> dados.url);
            //removeNoCache(mapa[nome], cache);
            //mapa.erase(it);
        }

        printCache(cache);
        sem_post(&mutexCache);
        // Saindo da sessão critica

        /*
        rio_writen(conexaoServer, (char *) "GET ", strlen("GET "));
        rio_writen(conexaoServer, nome, strlen(nome));
        rio_writen(conexaoServer, (char *) " HTTP/1.0\r\n\r\n", strlen(" HTTP/1.0\r\n\r\n"));
        */

        

    }
    
    else
        printf("Metodo não GET\n");

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

    /* Inicia o semaforo */
    sem_init(&mutexCache, 0, 1);
    sem_init(&mutexHost, 0, 1);

    if (argc != 3) {
		fprintf(stderr, "usage: %s <porta> <cache size in MB>\n", argv[0]);
		exit(1);
	}

    portaProxy = atoi(argv[1]);
    cacheSize = atoi(argv[2]);

    s = encapsulaListen(portaProxy);
    cache = criaCache(cacheSize);

    printf("tamanho do cache = %d\n", cache -> tamanhoLimite);


    /* wait for connection, then receive and print text */
    while(1) {
        
        if ((new_s = accept(s, (struct sockaddr *)&sin, &len)) < 0) {
            perror("simplex-talk: accept");
            exit(1);
        }
        

        else{
            //printf("new_s = %d\n", new_s);
            if (pthread_create(&tid, NULL, threadInit, &new_s) != 0) {
                perror("pthread_create() error");
                exit(1);
            }
        }
    }
}