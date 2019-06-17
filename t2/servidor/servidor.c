/*
**   Celso Antonio Uliana Junior,
**   June 2019
*/
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 

#define IP_PROTOCOL 0 
#define IP_ADDRESS "127.0.0.1" // localhost 
#define PORT_NO 15050 
#define NET_BUF_SIZE 32 
#define sendrecvflag 0 

/*   Método de limpar buffer   */
void LimpaBuffer(char * b){ 
	int i; 
	for (i = 0; i < NET_BUF_SIZE; i++) 
		b[i] = '\0'; 
} 

/*   Método de receber arquivo   */
int recebeArquivo(char * buf, int s){ 
	int i; 
	char ch; 
	for (i = 0; i < s; i++) { 
		ch = buf[i]; 
		if (ch == EOF) 
			return 1; 
		else
			printf("%c", ch); 
	} 
	return 0; 
} 

/*   Main    */
int main(int argc, char **argv){ 

	int sockfd, nBytes; 
	struct sockaddr_in addr_con; 
	int addrlen = sizeof(addr_con); 
	addr_con.sin_family = AF_INET; 
	addr_con.sin_port = htons(PORT_NO); 
	addr_con.sin_addr.s_addr = inet_addr(IP_ADDRESS); 
	char net_buf[NET_BUF_SIZE]; 
	FILE* fp; 

    if (argc != 5){
		fprintf(stderr, "uso: %s <fn> <sport> <wnd> <lp>\n", argv[0]);
		exit(1);
	}

    // portaProxy = atoi(argv[1]);
    // cacheSize = atoi(argv[2]);


	/*   socket()   */ 
	sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL); 

	if (sockfd < 0) 
		printf("\nfile descriptor not received!!\n"); 
	else
		printf("\nfile descriptor %d received\n", sockfd); 

	while (1) { 
		printf("\nPlease enter file name to receive:\n"); 
		scanf("%s", net_buf); 
		sendto(sockfd, net_buf, NET_BUF_SIZE, 
			sendrecvflag, (struct sockaddr*)&addr_con, 
			addrlen); 

		printf("\n---------Data Received---------\n"); 

		while (1) { 
			/*   Receber     */
			LimpaBuffer(net_buf); 
			nBytes = recvfrom(sockfd, net_buf, NET_BUF_SIZE, 
							sendrecvflag, (struct sockaddr*)&addr_con, 
							&addrlen); 

			/*   Processar   */
			if (recebeArquivo(net_buf, NET_BUF_SIZE)) { 
				break; 
			} 
		} 
        
		printf("\n-------------------------------\n"); 
	} 
	return 0; 
} 
