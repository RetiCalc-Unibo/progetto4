#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/select.h>
#include <math.h>

#define DIM_BUFF 4096
#define MAX_LENGTH 256
//Struct richiesta UDP
typedef struct {
	char fileName[MAX_LENGTH];
    char word[MAX_LENGTH];
} Request;

void handler(int signo){

}
int main(int argc, char * argv[]){
	Request request;
	int i, j, listenfd, connfd, udpfd, fd_file, nready, maxfdp1, udp_repl, fd_fileUDP_out, fd_fileUDP_in;
	char zero=0, buff[DIM_BUFF], nome_file[20], nome_dir[20], file_dest_UDP[256];
	fd_set rset;
	int len, nread, nwrite, num , ris, port; 
	struct sockaddr_in cliaddr, servaddr;
	const int on = 1;


	// Controllo Argomenti

	// DI DEFAULT UTILIZZO LA STESSA PORTA
	// else -> Errore e printf
	if(argc == 1){
		for(j = 0; j < strlen(argv[i]); j++){
			if(!isdigit(argv[i][j])){
				printf("Server: Ports must be numeric.\n");
				exit(2);
			}
		}
		port = atoi(argv[1]);
		if(port < 1024 || port> 65535){
				printf("Server: Ports must be between 1024-65535.\n");
				exit(3);
		}
	} else {
		printf("Server: Usage -> Server [port]\n");
		exit(4);
	}

	//Inizializzo indirizzo server
	memset((char *)&servaddr, 0,sizeof(servaddr));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(port);


	//Creazione socket TCP listen
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Apertura socket TCP"); 
		exit(5);
	}

	//Set option socket TCP
	//SO_REUSE --> reuse address; option_value --> on = 1
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
		perror("Set opzioni socker TCP");
		exit(6);
	}
	if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		perror("Bind socket TCP");
		exit(7);
	}

	//Creazione socket UDP 
	if ((udpfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("Apertura socket UDP");
		exit(8);
	}

	//Set option socket UDP
	//SO_REUSE --> reuse address; option_value --> on = 1
	if(setsockopt(udpfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
		perror("Set opzioni socker UDP");
		exit(9);
	}
	if(bind(udpfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		perror("Bind socket UDP");
		exit(10);
	}

	//Gestione richieste
	// NB! --> EINTR corrisponde a interruzione da richieste
	signal(SIGCHLD, handler);
	FD_ZERO(&rset); //set FD mask
	maxfdp1 = listenfd + 1; //determino fd più alto, range 
	for(;;){
		//Preparazione maschera --> writefds, exceptfds e timeout settati null
		// 							si considera solo readfds
		FD_SET(listenfd, &rset);
		FD_SET(udpfd, &rset);

		if((nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0){
			if(errno == EINTR) continue;
			else{
				perror("Select"); 
				exit(11);
			}
		}
		//Richieste UDP in sequenziale
		if(FD_ISSET(udpfd, &rset)){
			len = sizeof(struct sockaddr_in);
			if (recvfrom(udpfd, &request, sizeof(Request), 0, (struct sockaddr*)&cliaddr, &len) < 0) {
				perror("Recvfrom error ");
				continue;
			}
			len = strlen(request.word);
			//Leggo e riscrivo file in locale, poi rename 
			if((fd_fileUDP_in = open(request.fileName, O_RDONLY)) < 0){
				perror("Opening file input UDP");
				continue;
			}
			//File appoggio
			strcpy(file_dest_UDP, request.word);
			strcat(file_dest_UDP, ".tmp ");
			if((fd_fileUDP_out = open(file_dest_UDP, O_WRONLY | O_CREAT, 0644)) < 0){
				perror("Opening file output UDP");
				continue;
			}
			while ((nread = read(fd_fileUDP_in, &buff, DIM_BUFF)) > 0) {
				//Scrittura del file senza occorrenze parola
				for(i = 0; i < nread; i++){
					if(len == 1 || buff[i] != request.word[0])
						write(fd_fileUDP_out, &(buff[i]), sizeof(char));
					else {
						if(i + len < DIM_BUFF){
							j = 1;
							while(j < len && buff[i + j] == request.word[j])
								j++;
							if(j == len)
								i += len;
							else if(j == 1)
								write(fd_fileUDP_out, &(buff[i]), sizeof(char));
						}
						
					} //end word check
				}
			}

		}//if UDP

		//Richieste TCP in concorrente multiprocesso
		if(FD_ISSET(listenfd, &rset)){
			len = sizeof(struct sockaddr_in);
			//Accettazione connessione e creazione figlio
			if((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len)) < 0){
				if(errno = EINTR) continue;
				else {
					perror("Accept");
					exit(12);
				}
			}
			//Creazione figlio
			if(fork() == 0){
				close(listenfd);
				printf("PID %i: richiesta ricevuta\n", getpid());


				close(connfd);
				exit(0);
			} //figlio

			//Padre chiude la socket (not listen)
			close(connfd);

		}//if tcp

	}//for request
}//main