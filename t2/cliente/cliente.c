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
#define PORT_NO 15050 
#define NET_BUF_SIZE 32 
#define sendrecvflag 0 
#define nofile "File Not Found!" 

/*   Método de limpar buffer   */
void LimpaBuffer(char * b){ 
	int i; 
	for (i = 0; i < NET_BUF_SIZE; i++) 
		b[i] = '\0'; 
} 

/*   Método de enviar arquivo   */
int enviaArquivo(FILE * fp, char * buf, int s){ 
	int i, len; 
	if (fp == NULL) { 
		strcpy(buf, nofile); 
		len = strlen(nofile); 
		buf[len] = EOF; 
		return 1; 
	} 

	char ch;
	for (i = 0; i < s; i++) { 
		ch = fgetc(fp); 
		buf[i] = ch; 
		if (ch == EOF) 
			return 1; 
	} 
	return 0; 
} 

/*   Main   */
int main(int argc, char **argv){ 
    
	int sockfd, nBytes; 
	struct sockaddr_in addr_con; 
	int addrlen = sizeof(addr_con); 
	addr_con.sin_family = AF_INET; 
	addr_con.sin_port = htons(PORT_NO); 
	addr_con.sin_addr.s_addr = INADDR_ANY; 
	char net_buf[NET_BUF_SIZE]; 
	FILE* fp; 



    if (argc != 9){
		fprintf(stderr, "uso: %s <fn> <sip> <sport> <wnd> <rto> <mss> <dupack> <lp>\n", argv[0]);
		exit(1);
	}

    // portaProxy = atoi(argv[1]);
    // cacheSize = atoi(argv[2]);



	/*   socket() de UDP   */ 
	sockfd = socket(AF_INET, SOCK_DGRAM, IP_PROTOCOL); 

	if (sockfd < 0) 
		printf("\nfile descriptor not received!!\n"); 
	else
		printf("\nfile descriptor %d received\n", sockfd); 

	/*   bind()  */
	if (bind(sockfd, (struct sockaddr*)&addr_con, sizeof(addr_con)) == 0) 
		printf("\nSuccessfully binded!\n"); 
	else
		printf("\nBinding Failed!\n"); 

	while (1) { 
		printf("\nWaiting for file name...\n"); 

		/*   Rebece o nome do file   */
		LimpaBuffer(net_buf); 

        nBytes = recvfrom(sockfd, net_buf, 
						NET_BUF_SIZE, sendrecvflag, 
						(struct sockaddr*)&addr_con, &addrlen); 

        fp = fopen(net_buf, "r"); 


		printf("\nFile Name Received: %s\n", net_buf); 
		if (fp == NULL) 
			printf("\nFile open failed!\n"); 
		else
			printf("\nFile Successfully opened!\n"); 

		while (1) { 

			/*   Processamento de arquivo   */
			if (enviaArquivo(fp, net_buf, NET_BUF_SIZE)) { 
				sendto(sockfd, net_buf, NET_BUF_SIZE, 
					sendrecvflag, 
					(struct sockaddr*)&addr_con, addrlen); 
				break; 
			} 

			/*   Envio de arquivo    */
			sendto(sockfd, net_buf, NET_BUF_SIZE, 
				sendrecvflag, 
				(struct sockaddr*)&addr_con, addrlen); 
			LimpaBuffer(net_buf); 
		} 
		if (fp != NULL) 
			fclose(fp); 
	} 
	return 0; 
} 
