/* prog1_client.c - code for client side of connect four game program that uses TCP */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
/*------------------------------------------------------------------------
* Program: client
*
* JP Sohal
*
* Purpose: 1. allocate a socket, connect to a server
*		   2. display game type
*		   3. if number of players is equal to 1 tell player to wait
*		   4. display game state
*	       5. tell active player it is their turn and ask for move 
*		   6. tell non active player to wait for their turn
*	       7. send active players move to server
*	       8. repeat 4-8 till one player has won or a tie as occured
*
* Syntax: client [ host [port] ]
*
* host - name of a computer on which server is executing
* port - protocol port number server is using
*
* Note: Both arguments are optional. If no host name is specified,
* the client uses "localhost"; if no protocol port is
* specified, the client uses the default given by PROTOPORT.
*
*------------------------------------------------------------------------
*/


void display_game(char *buf);


/*
Pre: takes in buffer containing the game state
Post: display/prints game state
*/
void display_game(char *buf){
	int j = 0;
	while(j<=41){
		if(((j % 7) == 0)  && (j != 0)){
			printf("\n");
		}
		printf("%c", buf[j]);
		printf(" ");
		j++;		
	}
	printf("\n");
	printf("-------------\n");
	printf("0 1 2 3 4 5 6\n");
}



main( int argc, char **argv) {
	struct hostent *ptrh; 		/* pointer to a host table entry */
	struct protoent *ptrp; 		/* pointer to a protocol table entry */
	struct sockaddr_in sad; 	/* structure to hold an IP address */
	int sd; 					/* socket descriptor */
	int port; 					/* protocol port number */
	char *host; 				/* pointer to host name */
	char buf[1000]; 			/* buffer for data from the server */
	int turn_response; 			/* stores size of turn response from server */
	int game_table;				/* stores size of gametable recived from server */
	int move;					/* stores size of move sent to server */
	char moveBuf[4];			/* stores move of active player */
	bool winner = false;		/* stores wheter win has occured */
	bool tie = false;			/* stores wheter tie has occured */
	int game_typelen; 			/* stores size of game type length from server */



	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]); /* convert to binary */
	if (port > 0) /* test for legal value */
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1]; /* if host argument specified */

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Connect the socket to the specified server. */
	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}
	
	game_typelen = recv(sd, buf, sizeof(buf), 0);
	buf[game_typelen] = '\0';

	if (strcmp (buf, "S") == 0){
		fprintf(stdout, "Game Type: Standard\n");
	}
		
	if (strcmp (buf, "P") == 0){
		fprintf(stdout, "Game Type: Popout\n");
	}
	
	if (strcmp (buf, "K") == 0){
		fprintf(stdout, "Game Type: Antistack\n");
	}
	


	/*main while loop that handles respones and sends moves to server */
	while(!winner && !tie){
		turn_response = recv(sd,buf, sizeof(buf), 0);
		buf[turn_response] = '\0';

		if (strcmp (buf, "2") == 0){
			printf("Waiting for a second player...\n");
		}
			
		if (strcmp (buf, "Y") == 0){
			game_table = recv(sd,buf, sizeof(buf), 0);
			buf[game_table] = '\0';
			display_game(buf);
			fprintf(stdout,"Please Make Your Move: ");
			fgets(moveBuf,1000-1,stdin);
			move = send(sd,moveBuf,sizeof(moveBuf),0);
		}
		
		if (strcmp (buf, "I") == 0){
			fprintf(stdout,"Invalid move. Please Make a New Move: ");
			fgets(moveBuf,1000-1,stdin);
			move = send(sd,moveBuf,sizeof(moveBuf),0);
		}
		
		if (strcmp (buf, "H") == 0){
			game_table = recv(sd,buf, sizeof(buf), 0);
			buf[game_table] = '\0';
			display_game(buf);
			printf("Please Wait For Your Turn\n");
		}	
	
		if (strcmp (buf, "W") == 0){
			printf("You Win\n");
			winner = true;
		}
		
		if (strcmp (buf, "L") == 0){
			printf("You Lose\n");
			winner = true;
		}	
		
		if (strcmp (buf, "T") == 0){
			printf("Tie\n");
			tie = true;
		}
			
	}
	
	close(sd);

	exit(EXIT_SUCCESS);
}

