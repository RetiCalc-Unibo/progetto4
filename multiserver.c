#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <ctype.h>

int main(int argc, char * argv[]){

	int i, j;
	int port[2];


	// Controllo Argomenti
	// se 1 -> Porte Default -> UDP = 1050, TCP = 1051
	// se 3 -> Controllo che siano numeriche e che siano valori validi compresi tra 1024 e 65535
	// else -> Errore e printf
	if(argc == 1){
		port[0] = 1050; // Port UDP
		port[1] = 1051; // Port TCP
	} else if(argc == 3 ) {
		for(i = 1; i < 3; i ++){
			for(j = 0; j < strlen(argv[i]); j++){
				if(!isdigit(argv[i][j])){
					printf("Server: Ports must be numeric.\n");
					exit(1);
				}
			}
			port[i - 1] = atoi(argv[i]);
			if(port[i - 1] < 1024 || port[i - 1] > 65535){
				printf("Server: Ports must be between 1024-65535.\n");
				exit(1);
			}
		}
	} else {
		printf("Server: Usage -> Server [portUDP] [port[TCP]\n");
		exit(1);
	}



}