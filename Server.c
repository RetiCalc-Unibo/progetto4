#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <ctype.h>

int main(int argc, char * argv[]){

	int i, j, listenfd, connfd, udpfd, fd_file, nready, maxfdpl;
	char zero=0, buff[DIM_BUFF], nome_file[20], nome_dir[20];
	fd_set rset;
	int len, nread, nwrite, num , ris, port;
	struct sockaddr_in cliaddr, servaddr;
	int port;
	const int on = 1;


	// Controllo Argomenti

	// DI DEFAULT UTILIZZO LA STESSA PORTA

	// se 1 -> Porte Default -> UDP = 1050, TCP = 1051
	// se 2 -> Assegno la stessa porta sia a UDP che a TCP 
	// se 3 -> Controllo che siano numeriche e che siano valori validi compresi tra 1024 e 65535
	// else -> Errore e printf
	if(argc == 1){
		for(j = 0; j < strlen(argv[i]); j++){
			if(!isdigit(argv[i][j])){
				printf("Server: Ports must be numeric.\n");
				exit(2);
			}
		}

		port = atoi(argc[1]);
		if(port[i - 1] < 1024 || port[i - 1] > 65535){
				printf("Server: Ports must be between 1024-65535.\n");
				exit(3);
		}
	} /*else if(argc == 2){
		for(j = 0; j < strlen(argv[1]); j++){
				if(!isdigit(argv[i][j])){
					printf("Server: Ports must be numeric.\n");
					exit(1);
				}
			}
		port[0] = atoi(argv[1]);
		port[1] = atoi(argv[1]);
		if(port[0] < 1024 || port[0] > 65535){
				printf("Server: Ports must be between 1024-65535.\n");
				exit(1);
		}
	} else if(argc == 2 ) {
		for(i = 1; i < 3; i ++){
			for(j = 0; j < strlen(argv[i]); j++){
				if(!isdigit(argv[i][j])){
					printf("Server: Ports must be numeric.\n");
					exit(2);
				}
			}
			port[i - 1] = atoi(argv[i]);
			if(port[i - 1] < 1024 || port[i - 1] > 65535){
				printf("Server: Ports must be between 1024-65535.\n");
				exit(3);
			}
		}
	} */ else {
		printf("Server: Usage -> Server [port]\n");
		exit(4);
	}


	//Inizializzo indirizzo server

	memset((char *)&serveraddr, 0,sizeof(serveraddr));
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	serveraddr.sin_port = htons(port);


	//Creazione socket TCP listen

	if ((listedfd = socket(AF_INET, SOCK_STEAM, 0)) < 0){
		perror("Apertura socket TCP"); 
		exit(5);
	}

	//Set option socket TCP

	//SO_REUSE --> reuse address; option_value --> on = 1
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSE, &on, sizeof(on)) < 0){
		perror("Set opzioni socker TCP");
		exit(6);
	}

	if(bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0){
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
	if(setsockopt(udpfd, SOL_SOCKET, SO_REUSE, &on, sizeof(on)) < 0){
		perror("Set opzioni socker UDP");
		exit(9);
	}

	if(bind(udpfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0){
		perror("Bind socket UDP");
		exit(10);
	}

	//Gestione richieste
	// NB! --> EINTR corrisponde a interruzione da richieste
	signal(SIGCHLD, handler);
	FD_ZERO($rset); //set FD mask

	maxfdpl = max(listenfd, udpfd) + 1; //determino fd più alto

	for(;;){
		//Preparazione maschera --> writefds, exceptfds e timeout settati null
		// 							si considera solo readfds
		FD_SET(listenfd, &rset);
		FD_SET(udpfd, &rset);

		if((nready = select(maxfdpl, &rset, NULL, NULL, NULL)) < 0){
			if(errno == EINTR) continue;
			else{
				perror("Select"); 
				exit(11);
			}
		}

		//Richieste UDP in sequenziale
		if(FD_ISSET(udpfd, &rset)){


		}

		//Richieste TCP in parallelo
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
			if(fork() == 0){
				close(listenfd);
				printf("PID %i: richiesta ricevuta\n", getpid());
			}


		}
	}




}