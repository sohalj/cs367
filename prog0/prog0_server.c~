/* prog0_server.c - code for server side of guessing program that uses TCP */

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define QLEN 6 /* size of request queue */


/*------------------------------------------------------------------------
* Program: server
*
* JP Sohal, Dana Sanders
*
* Purpose: allocate a socket and then repeatedly execute the following:
*  1. wait for the next connection from a client
*  2. fork process to handle guess_game interaction
*  3. recieve guess from client
*  4. send back response, which is determined by the guess
*  5. if guess is correct break loop and close socket else go back to 3
*
* Syntax: server [ port ] answer
*
* port - protocol port number to use
* answer- the number that the client must guess 	
*
* Note: The port argument is optional. If no port is specified,
* the server uses the default given by PROTOPORT.
*
*------------------------------------------------------------------------
*/

void guess_game(int sd2, char *buf, char *answer, int prev_guess, int converted_guess);

/* Guess Game Helper
 * This is where the server compares the guess from the client
 * with the answer and sends back information regarding position.
 */
void guess_game(int sd2, char *buf, char *answer, int prev_guess,  int converted_guess){

	char return_char; /* The returned character sent back to client */
	int guesslen; /* Length of guess from client */
	int int_guess; /* converted_guess converted back to host byte order format */
	int int_answer; /* Integer value of answer held by server */

	int_answer = atoi(answer);
	
	while(1){
		guesslen = recv(sd2, &converted_guess, sizeof(converted_guess), 0);
		buf[guesslen] = '0';
		int_guess = ntohl(converted_guess);
		
		/* You win! */
		if (int_answer == int_guess){
			return_char = '0';
			sprintf(buf, "%c", return_char);
			send(sd2, buf, strlen(buf), 0);
			close(sd2);
			break;
		}	
		
		/* Warmer! */
		if (abs((int_answer - int_guess)) < abs(prev_guess - int_answer)){
			return_char = '1';
			sprintf(buf, "%c", return_char);
			send(sd2, buf, strlen(buf), 0);
			prev_guess = int_guess;
		}
		
		/* Colder! */
		else if (abs((int_answer - int_guess)) > abs(prev_guess - int_answer)){
			return_char = '2';
			sprintf(buf, "%c", return_char);
			send(sd2, buf, strlen(buf), 0);
			prev_guess = int_guess;
		}
		
		/* Same! */
		else if (abs((int_answer - int_guess)) == abs(prev_guess - int_answer)){
			return_char = '3';
			sprintf(buf, "%c", return_char);
			send(sd2, buf, strlen(buf), 0);
			prev_guess = int_guess;
		}		

	}
}

int main(int argc, char **argv) {
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold server's address */
	struct sockaddr_in cad; /* structure to hold client's address */
	int sd, sd2; /* socket descriptors */
	int port; /* protocol port number */
	int alen; /* length of address */
	char buf[1000]; /* buffer for string the server sends */
	pid_t cpid; /* child pid from fork */
	char *answer; /* answer given from command line */
	int prev_guess; /* client's previous guess -- initalized to 0 on first run */
	int converted_guess; /* 32 unsigned network byte conversion of guess from client */
	
	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port answer\n");
		exit(EXIT_FAILURE);
	}

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */
	sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

	port = atoi(argv[1]); /* convert argument to binary */
	answer = argv[2];
	if (port > 0) { /* test for illegal value */
		sad.sin_port = htons((u_short)port);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Bind a local address to the socket */
	if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	/* Specify size of request queue */
	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}

	/* Main server loop - accept and handle requests */
	while (1) {
		alen = sizeof(cad);
		if ( (sd2=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
			fprintf(stderr, "Error: Accept failed\n");
			exit(EXIT_FAILURE);
		}
		
		cpid = fork();
		if (cpid < 0) {
     		fprintf(stderr, "Error: Fork failed\n");
     		return -1;
   		}
   		
   		/* We are the child, run the guessing game */
   		if (cpid == 0){
   			close(sd);
   			prev_guess = 0;
   			guess_game(sd2, buf, answer, prev_guess, converted_guess);
   			return 0;
   		}
   		
   		/* We are the parent */
   		else
 			close(sd2);
	}
}

