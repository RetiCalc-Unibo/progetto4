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
#include <dirent.h>

#define DIM_BUFF 256
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
	int i, j, tcpfd, connfd, udpfd, fd_file, nready, maxfdp1, udp_repl, len_word;
	int fd_fileUDP_out, fd_fileUDP_in;
	char zero=0, buff[DIM_BUFF], nome_file[20], nome_dir[20], file_dest_UDP[256];
	char dirName[MAX_LENGTH];
	fd_set rset;
	int len, nread, nwrite, num , ris, port, check_word;; 
	struct sockaddr_in cliaddr, servaddr;
	struct hostent *clienthost;
	const int on = 1;

	DIR * dir;
	struct dirent * ent;
	int finish;

	// Controllo Argomenti

	// DI DEFAULT UTILIZZO LA STESSA PORTA
	// else -> Errore e printf
	if(argc == 1){
		port = 1050;
	}
	else if(argc == 2){
		for(j = 0; j < strlen(argv[1]); j++){
			if(!isdigit(argv[1][j])){
				printf("Server: Port must be numeric.\n");
				exit(2);
			}
		}
		port = atoi(argv[1]);
		if(port < 1024 || port> 65535){
				printf("Server: Port must be between 1024-65535.\n");
				exit(3);
		}
	} else {
		printf("Server: Usage -> Server [port]\n");
		exit(4);
	}
	printf("Server: Inizializzato\n");

	//Inizializzo indirizzo server
	memset((char *)&servaddr, 0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(port);


	//Creazione socket TCP listen
	if ((tcpfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Apertura socket TCP"); 
		exit(5);
	}

	//Set option socket TCP
	//SO_REUSE --> reuse address; option_value --> on = 1
	if(setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0){
		perror("Set opzioni socker TCP");
		exit(6);
	}
	if(bind(tcpfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
		perror("Bind socket TCP");
		exit(7);
	}

	if(listen(tcpfd, 5) < 0){
		perror("Listen socket TCP");
		exit(7);
	}
	printf("Server: bind socket TCP ok\n");
	//Creazione socket UDP 
	if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
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
	printf("Server: bind socket UDP ok\n");

	//Gestione richieste
	// NB! --> EINTR corrisponde a interruzione da richieste
	signal(SIGCHLD, handler);
	FD_ZERO(&rset); //set FD mask
	maxfdp1 = tcpfd + 1; //determino fd più alto, range 
	for(;;){
		//Preparazione maschera --> writefds, exceptfds e timeout settati null
		// 							si considera solo readfds
		FD_SET(tcpfd, &rset);
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
			printf("Server: select UDP\n");
			ris=0;
			len = sizeof(struct sockaddr_in);
			//Ricezione dati
			if (recvfrom(udpfd, &request, sizeof(Request), 0, (struct sockaddr*)&cliaddr, &len) < 0) {
				perror("Recvfrom error ");
				continue;
			}
			clienthost = gethostbyaddr((char *)&cliaddr.sin_addr, sizeof(cliaddr.sin_addr), AF_INET);
			if (clienthost == NULL) 
				printf("Server: Client host information not found\n");
			else {
				printf("Server: Operazione richiesta da: %s %i\n", clienthost->h_name, (unsigned)ntohs(cliaddr.sin_port));
			}			
			//Leggo e riscrivo file in locale, poi rename 
			if((fd_fileUDP_in = open(request.fileName, O_RDONLY)) < 0){
				perror("Opening file input UDP");
				ris=-1;
				continue;
			}
			//File appoggio
			strcpy(file_dest_UDP, request.word);
			strcat(file_dest_UDP, ".tmp ");
			if((fd_fileUDP_out = open(file_dest_UDP, O_WRONLY | O_CREAT, 0644)) < 0){
				perror("Opening file output UDP");
				ris=-1;
				continue;
			}
			printf("Server: Richiesta eliminazione parola %s dal file %s\n", 
				request.word, request.fileName);
			len_word = strlen(request.word);
			while ((nread = read(fd_fileUDP_in, &buff, DIM_BUFF)) > 0) {
				//Scrittura del file senza occorrenze parola
				check_word = 0;
				for(i = 0; i < nread; i++){
					j = 0;
					if(!check_word)
						write(fd_fileUDP_out, &(buff[i]), sizeof(char));
					else if( (i + len_word) < nread 
							 && (buff[i+len_word] == ' ' || buff[i+len_word] == '\n') ){
							while(j < len_word && buff[i + j] == request.word[j])
								j++;
							if(j == len_word-1) i +=len_word;
					}
					if(buff[i] == ' ' || buff[i] == '\n')
						check_word = 0;
					else check_word = 1;
				}
			}
			//Invio risposta
			if(sendto(udpfd, &ris, sizeof(int), 0, (struct sockaddr*)&cliaddr, len) < 0) {
				perror("Sendto error "); 
				exit(1);
			}
		}//if UDP

		//Richieste TCP in concorrente multiprocesso
		if(FD_ISSET(tcpfd, &rset)){
			len = sizeof(struct sockaddr_in);
			//Accettazione connessione e creazione figlio
			if((connfd = accept(tcpfd, (struct sockaddr *)&cliaddr, &len)) < 0){
				if(errno = EINTR) continue;
				else {
					perror("Accept");
					exit(12);
				}
			}
			//Creazione figlio
			if(fork() == 0){
				close(tcpfd);
				printf("Server: PID %i: richiesta ricevuta\n", getpid());
				while(1){
					finish = 0;
					if(read(connfd, &dirName, MAX_LENGTH) < 0){
						perror("dirname");
						exit(12);
					}
					printf("Server: Leggo dalla cartella %s\n", dirName);
					if((dir = opendir(dirName)) != NULL){
						if(write(connfd, "1", sizeof(char)) < 0){
							perror("write control accept");
							exit(13);
						}
						while((ent = readdir(dir)) != NULL){
							if(ent->d_name[0] == '.'){
								if(ent->d_name[1] == '.')
									if(ent->d_name[2] == '\0'){
										continue;
									}
								if(ent->d_name[1] == '\0'){
									continue;
								}
							}				
							printf("\t%s\n", ent->d_name);
							write(connfd, ent->d_name, MAX_LENGTH);
							write(connfd, &finish, sizeof(int));
						}
						closedir(dir);

					} else {
						if(write(connfd, "0", sizeof(int)) < 0){
							perror("write control refuse");
							exit(13);
						}
						perror("diropen");
						exit(13);
					}
					finish = 1;
					write(connfd, &finish, sizeof(int));
				}
				close(connfd);
				exit(0);
			} //figlio

			//Padre chiude la socket (not listen)
			close(connfd);

		}//if tcp

	}//for request
}//main