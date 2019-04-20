/*
**   Celso Antonio Uliana Junior,
**   April 2019
*/
#include "cache.h"


// Variavel passada no segundo argumento do listen(). 
// Serve como quantidade maxima de requisições de conexão.
#define MAX_PENDING 5 
#define MAX_LINE 8192  
#define TAMANHO_BUFFER 8192  
#define TAMANHO_MAX 8192  

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
int encapsulaCliente(int port, char *host);

int encapsulaClienteTs(int port, char *host);

/*  Inicialização da thread, função passada para pthread_create(),
    Onde acontece o pthread_detach para que requisições 
    Simultaneas funcionem.
*/
void *threadInit(void *cliente);

ssize_t rio_writen(int fd, void *usrbuf, size_t n);

ssize_t rio_readn(int fd, void *usrbuf, size_t n);

/*  Metodo que faz o parsing da requisicao, e busca no cache,
    Caso ache, retorna para o cliente, caso não, adiciona ao
    Cache.
*/
void parseAndCache(int conexao);

/*
###############################################################################
Seção de declaração de variaveis globais, como mutex e o cache.
###############################################################################
*/

/* Mutex */
static sem_t mutex;

/* Cache */
CacheFinal * cache;

/* Porta */
int port;

/*
###############################################################################
Seção de Implementação dos métodos declarados acima.
###############################################################################
*/

int encapsulaListen(int port){

    struct sockaddr_in sin;
    int s;

    /* Constroi estrutura de dados de endereço */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);


    /* Abre um socket passivo */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }

    /* Binda o socket a uma porta */
    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("simplex-talk: bind");
        exit(1);
    }
    
    /* Escuta na porta */
    if(listen(s, MAX_PENDING) < 0){
        perror("simplex-talk: listen");
        exit(1);
    }

    return s;
}

int encapsulaClienteTs(int port, char *host){

    int cliente;
    struct hostent *sharedp, *unsharedp;;
    struct sockaddr_in serveraddr;

    if ((cliente = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    return -1; /* check errno for cause of error */

    sem_wait(&mutex);
    /* Abre o IP do server e a porta. */
    if ((sharedp = gethostbyname(host)) == NULL)
	    return -2; /* check h_errno for cause of error */

    *unsharedp = *sharedp;
    sem_post(&mutex);

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)unsharedp->h_addr_list[0], 
	  (char *)&serveraddr.sin_addr.s_addr, unsharedp->h_length);
    serveraddr.sin_port = htons(port);

    /* Abre a conexão com o servidor */
    if (connect(cliente, (sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
	    return -1;

    return cliente;

}

int encapsulaCliente(int port, char *host){

    int cliente;
    struct hostent *sharedp;
    struct sockaddr_in serveraddr;

    if ((cliente = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    return -1; /* check errno for cause of error */

    /* Abre o IP do server e a porta. */
    if ((sharedp = gethostbyname(host)) == NULL)
	    return -2; /* check h_errno for cause of error */


    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)sharedp->h_addr_list[0], 
	  (char *)&serveraddr.sin_addr.s_addr, sharedp->h_length);
    serveraddr.sin_port = htons(port);

    /* Abre a conexão com o servidor */
    if (connect(cliente, (sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
	    return -1;

    return cliente;

}

ssize_t rio_writen(int fd, char *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp =  usrbuf;

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

ssize_t rio_readn(int fd, char *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

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

void * threadInit(void * cliente){
    int conexao_cliente = *((int *) cliente);
    int len;
    char buf[TAMANHO_BUFFER];

    // Deatach, separa a thread atual da thread main, sendo assim, não é
    // Necessário o uso de Pthread_join().
    pthread_detach(pthread_self());

    printf("connexão cliente num %d\n", conexao_cliente);


    parseAndCache(conexao_cliente);

    //recv(conexao_cliente, buf, sizeof(buf), 0);
    //fputs(buf, stdout);

    close(conexao_cliente);

    return NULL;
}

void parseAndCache(int conexao){
    char buf[TAMANHO_BUFFER];
    char * hostname;
    char * aux;
    char * metodo = (char *) malloc(MAX_LINE * sizeof(char));
    char * nome = (char *) malloc(MAX_LINE * sizeof(char));
    char * restoDaRequisicao = (char *) malloc(TAMANHO_MAX * sizeof(char));

    recv(conexao, buf, sizeof(buf), 0);


    /* Lê o formato de dados descrito de uma string. */
    sscanf(buf, "%s %s %s", metodo, nome, restoDaRequisicao);

    if(strcasecmp(metodo, "GET") == 0){
        printf("metodo = %s\n nome = %s\n resto = %s\n" , metodo, nome, restoDaRequisicao);

        hostname = strstr(nome, "www");

        if(hostname != NULL)
            printf("hostname = %s\n", hostname);



        // Ajusta o hostname para ser apropriado na hora de abrir
        // a conexão.
        char * aux = index(hostname,':');
        if(aux == NULL){
            hostname = (char *) strtok(hostname, "/");
        }
        else{
            hostname = (char *) strtok(hostname, ":");
        }

        int conexao_server = encapsulaCliente(80, hostname);

        rio_writen(conexao_server, (char *) "GET ", strlen("GET "));
        rio_writen(conexao_server, nome, strlen(nome));
        rio_writen(conexao_server, (char *) " HTTP/1.0\r\n\r\n", strlen(" HTTP/1.0\r\n\r\n"));


        printf("conexão servidor = %d\n", conexao_server);

        int tam = 0, n;

        while((n = rio_readn(conexao_server, buf, MAX_LINE)) > 0 ) {
            tam += n;
            rio_writen(conexao, buf, n);
            bzero(buf, MAX_LINE);
        }

        close(conexao_server);

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
    int len, s, new_s, cache_size;
    pthread_t tid;

    /* Inicia o semaforo */
    sem_init(&mutex, 0, 1);

    if (argc != 3) {
		fprintf(stderr, "usage: %s <port> <cache size in MB>\n", argv[0]);
		exit(1);
	}

    port = atoi(argv[1]);
    cache_size = atoi(argv[2]);
    
    signal(SIGPIPE, SIG_IGN);

    s = encapsulaListen(port);
    cache = criaCache(cache_size);


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