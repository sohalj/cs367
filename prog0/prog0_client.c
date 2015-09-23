/* prog0_client.c - code for client side of guessing game program that uses TCP */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*------------------------------------------------------------------------
* Program: client
*
* JP Sohal, Dana Sanders
*
* Purpose: 1. allocate a socket, connect to a server
*		   2. ask user for guess 
*		   3. send guess to server
*	       4. receive response from server
*	       5. print output, which is determined by response
*	       6. repeat 2-5 till user has won
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
main( int argc, char **argv) {
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */
	int sd; /* socket descriptor */
	int port; /* protocol port number */
	char *host; /* pointer to host name */
	int n = 1; /* number of characters sent  */
	char buf[1000]; /* buffer for data from the server */
	int responselen; /* response length from server */

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
	

	/* Repeatedly send and recieve data from socket and write to user's screen. */

	
	while (n > 0) {
		fprintf(stdout,"Enter guess: ");
		fgets(buf,1000-1,stdin);
		printf("\n");
		int intguess = atoi(buf);
		int converted_guess = htonl(intguess);
		n = send(sd,&converted_guess,sizeof(converted_guess),0);
		responselen = recv(sd, buf, sizeof(buf), 0);
		buf[responselen] = '\0';
		
		if (strcmp (buf, "0") == 0){
			fprintf(stdout, "You Win!\n");
			break;
		}
		
		if (strcmp (buf, "1") == 0)
			fprintf(stdout, "Warmer\n");
		
		if (strcmp (buf, "2") == 0)
			fprintf(stdout, "Colder\n");
		
		if (strcmp (buf, "3") == 0)
			fprintf(stdout, "Same\n");

	}

	close(sd);

	exit(EXIT_SUCCESS);
}

