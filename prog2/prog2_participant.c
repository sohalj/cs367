/* prog2_participant.c - code for patrcipant client for program 2*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*------------------------------------------------------------------------
* Program: participant client
*
* Purpose: allocate a socket, connect to a server, and read from standard 
		   input.
*
* Syntax: prog2_partcipant [ host [port] ]
*
* host - name of a computer on which server is executing
* port - protocol port number server is using
*
*
*------------------------------------------------------------------------
*/
main( int argc, char **argv) {
	struct hostent *ptrh; 		/* pointer to a host table entry */
	struct protoent *ptrp; 		/* pointer to a protocol table entry */
	struct sockaddr_in sad; 	/* structure to hold an IP address */
	int sd; 					/* socket descriptor */
	int port; 					/* protocol port number */
	char *host; 				/* pointer to host name */
	int bytesRead; 				/* number of characters read */
	char buf[1024]; 			/* buffer for data read from stdin, from partcipant */
	


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

	/* Repeatedly read data from standard in and send to server. */
	while(1){
		bytesRead = read(0,buf,sizeof(buf));
		send(sd, buf, bytesRead, 0);
	}

	exit(EXIT_SUCCESS);
}
