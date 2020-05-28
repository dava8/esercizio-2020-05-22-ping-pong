#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include<stdio.h>
#include <stdlib.h>
#include <errno.h>

/*
Scrivere un programma che realizza un "ping-pong" tra due processi utilizzando una coppia di pipe,
una per ogni direzione.

Il contatore (di tipo int) viene incrementato ad ogni ping ed ad ogni pong e viene trasmesso attraverso la pipe.

Quanto il contatore raggiunge il valore MAX_VALUE il programma termina.

proc_padre manda a proc_figlio il valore 0 attraverso pipeA.
proc_figlio riceve il valore 0, lo incrementa (=1) e lo manda a proc_padre attraverso pipeB.
proc_padre riceve il valore 1, lo incrementa (=2) e lo manda a proc_figlio attraverso pipeA.
proc_figlio riceve il valore 2 .....

fino a MAX_VALUE, quando termina il programma.
(il primo processo che arriva a MAX_VALUE fa terminare il programma).


 */

#define MAX_VALUE 10000


int main() {

	int pipeA[2];
	int pipeB[2];
	char buf[sizeof(int)];

	if (pipe(pipeA) == -1) {
		perror("problema con pipe");

		exit(EXIT_FAILURE);
	}
	if (pipe(pipeB) == -1) {
		perror("problema con pipe");

		exit(EXIT_FAILURE);
	}


	switch (fork()) {
		case -1:
			perror("problema con fork");

			exit(EXIT_FAILURE);

		case 0: // processo FIGLIO: leggerà dalla PIPE

			close(pipeA[1]); // chiudiamo l'estremità di scrittura della pipe, non ci serve

			while (1) {

				int numRead = read(pipeA[0], buf, sizeof(int));

				if (numRead == 0)
					break; // EOF: la pipe è stata chiusa dal lato di scrittura

				//converto la stringa ricevuta il numero

				int x = atoi(buf);
				++x; //incremento di 1

				//se arrivo a MAX_VALUE chiudo la pipe dove child invia così che parent esca
				//dal ciclo

				if(x>=MAX_VALUE){

					close(pipeB[1]);


				}else{

					char str[sizeof(int) * 4 + 1];
					sprintf(str, "%d", x);


					int len = strlen(str);


					write(pipeB[1], str, len); // scriviamo sull'estremità di scrittura della pipe
				}


				if (numRead == -1) {
					perror("errore in read");
					exit(EXIT_FAILURE);
				}


				printf("CHILD : colpisco %d\n",x);



			}

			close(pipeA[0]); // chiudiamo l'estremità di lettura della pipe, ora il kernel libera tutta la struttura di pipe
			close(pipeB[1]);

			//printf("ciao\n");

			exit(EXIT_SUCCESS); // fine del processo figlio


		default:
			// processo PADRE: scriverà dentro la PIPE

			close(pipeA[0]); // chiudiamo l'estremità di lettura della pipe, non ci serve
			close(pipeB[1]);

			int start =0; //int di partenza

			char str[sizeof(int)];
			sprintf(str, "%d", start);

			printf("PARENT : batto %d\n",start);
			int len = strlen(str);

			write(pipeA[1], str, len); // scriviamo sull'estremità di scrittura della pipe

			while (1) {

				int numRead = read(pipeB[0], buf, sizeof(int));
				if (numRead == 0)
					break;

				//converto in numero e incremento
				int x = atoi(buf);
				++x;

				//se colpisco per l'ultima volta chiuso la pipe così che il child esca dal while
				if(x>=MAX_VALUE){

					close(pipeA[1]);

				}else{


					char str[sizeof(int) * 4 + 1];
					sprintf(str, "%d", x);

					int len = strlen(str);


					write(pipeA[1], str, len); // scriviamo sull'estremità di scrittura della pipe
				}


				if (numRead == -1) {
					perror("errore in read");
					exit(EXIT_FAILURE);
				}

				printf("PARENT : colpisco %d\n",x);


			}
fine:
			wait(NULL); // aspettiamo la terminazione del processo figlio

			exit(EXIT_SUCCESS);

	}
}
