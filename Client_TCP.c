#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_LENGTH 256

int main (int argc, char *argv[]) {
	int sd, port, nread;
	char buff[MAX_LENGTH];
    char result;
    char dirName[MAX_LENGTH];
	struct hostent *host;
	struct sockaddr_in servaddr;

	// Controllo argomenti
	if (argc != 3) {
		printf("Client TCP: Error:%s serverAddress serverPort\n", argv[0]);
		exit(1);
	}

	// Inizializzazione indirizzo server
	memset((char *)&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	host = gethostbyname(argv[1]);

	nread = 0;
	while (argv[2][nread] != '\0') {
		if ((argv[2][nread] < '0') || (argv[2][nread] > '9')) {
			printf("Client TCP: Secondo argomento non intero\n");
			exit(2);
		}
		nread++;
	}

	port = atoi(argv[2]);

	// Verifica port e host
	if (port < 1024 || port > 65535) {
		printf("Client TCP: %s = porta scorretta...\n", argv[2]);
		exit(3);
	}
	if (host == NULL) {
		printf("Client TCP: %s not found in /etc/hosts\n", argv[1]);
		exit(4);
	} else {
		servaddr.sin_addr.s_addr = ((struct in_addr *)(host->h_addr)) -> s_addr;
		servaddr.sin_port = htons(port);
	}

	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Client TCP: Apertura socket");
		exit(5);
	}
	
	printf("\nClient TCP: Creata la socket sd=%d\n", sd);
	if (connect(sd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0) {
		perror("Connect"); 
		exit(6);
	}
	printf("Client TCP: Connessione effettuata\n\n");

	printf("Client TCP: Ciclo di richiesta di direttori fino a EOF\n\n");
	printf("Client TCP: Nome della directory, EOF per terminare: ");

	while (gets(dirName)) {
		// Invio del nome della directory
		write(sd, &dirName, sizeof(dirName));

		if (read(sd, &result, sizeof(char)) < 0) {
			perror("Read result");
			exit(7);
		} else if (result == '0') {
			printf("Client TCP: Problema di lettura della directory nel Server\n");
			printf("Client TCP: Nome della directory, EOF per terminare: ");
			continue;
		}

        // Ricezione e stampa dei nomi di file inviati dal server
        /* Ciclicamente leggo un nome file (256 byte) e un carattere c.
        *  Il carattere inviato dal server può valere '0' o '1':
        *   - se '0' significa che non ci sono altri nomi file da inviare
        *   - se '1' significa che il server non ha finito di trasmettere
        */
		printf("Client TCP: Ricevo e stampo i nomi dei file remoti\n---------------\n");
		
		while ((nread = read(sd, buff, MAX_LENGTH)) > 0) {
            if (buff[0] == '\0') {
            	break;
            }
            printf("\t%s\n", buff);
        }

		printf("---------------\nClient TCP: Trasferimento terminato\n\n");
		printf("Client TCP: Nome della directory, EOF per terminare: ");
	}
    
	close(sd);
	printf("\nClient TCP: termino...\n");
	exit(8);
}