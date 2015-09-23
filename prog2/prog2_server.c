/* prog2_server.c - code for server program that runs a chat */

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#define QLEN 6 /* size of request queue */


/*------------------------------------------------------------------------
* Program: server
*
* Purpose: allocate a socket for partcipant client and observer client
           and then repeatedly execute the following:
* (1) wait for activity on one of the sd in the fd set using select
* (2) determine which sd is active
* (3) based on the sd that is active, handle the activity
*       Types of Activity:
*       1. activity on listening participant socket
*           -new connection
*       2. activity on listening observer socket
*           -new connection
*       3. activity on a partcipant sd
*           - sending a message
*           - quiting
*       4. activtiy on a observer sd
*           -quiting
* (4) go back to step (1)
*
* Syntax: prog2_server [ port1 ] [ port2 ]
*
* port1 - protocol port number to use with participant clients
* port2 - protocol port number to use with observer clients
* 
* 
*
*------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
	struct protoent *ptrp;          /* pointer to a protocol table entry */
    struct sockaddr_in sad0;        /* structure to hold server's address for observers*/
    struct sockaddr_in sad;         /* structure to hold server's address for partcipants */
    struct sockaddr_in cad;         /* structure to hold client's address */
    int sd, sd2, sd3, sd4;          /* socket descriptors */
    int p_port;                     /* protocol port number for participants */
    int o_port;                     /* protocol port number for oberservers*/
    int alen;                       /* length of address */
    char buf[1024];                 /* buffer for string the server sends */
    char rbuf[1029];                /* buffer for string the server sends with appended user id */
    char pbuf[1024];                /* buffer for message the partcipant sends */
    fd_set readSd;                  /* fd_set for partcipants and observers */
    int i;                          /* int to iterate through arrays */
    int j;                          /* int to iterate through arrays */
    int o_clients[64];              /* Array storing observer sd numbers */
    int p_clients[64];              /* Array storing participant sd numbers */
    int ready_sd;                   /* int stores number of ready sds returned by select */
    int newP_sock;                  /* socket descriptors for participants */
    int newO_sock;                  /* socket descriptors for observers*/
    int m;                          /* int to iterate through arrays */
    int bytesRead;                  /* int stores number bytes read returned by read */
    char user[3];                   /* char array stores participants user number */
    int userNum;                    /* int stores number of bytes of the user number */


    if( argc != 3 ) {
            fprintf(stderr,"Error: Wrong number of arguments\n");
            fprintf(stderr,"usage:\n");
            fprintf(stderr,"./server server_port-partcipants server_port-observers\n");
            exit(EXIT_FAILURE);
    }
    
    
    for(i = 0; i<=63; i++){         /*Initialize observer and partcipant client arrays to 0*/
        o_clients[i] = 0;
    }       
	for(i = 0; i<=63; i++){
        p_clients[i] = 0;
    }       
    
    
    memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
    memset((char *)&sad0,0,sizeof(sad0)); /* clear sockaddr structure */
    sad.sin_family = AF_INET; /* set family to Internet */
    sad0.sin_family = AF_INET; /* set family to Internet */
    sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */
    sad0.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */
    p_port = atoi(argv[1]); /* convert argument to binary */
    o_port = atoi(argv[2]); /* convert argument to binary */
    
    if (p_port > 0) { /* test for illegal value */
        sad.sin_port = htons((u_short)p_port);
    } 
    else { /* print error message and exit */
        fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
        exit(EXIT_FAILURE);
    }

    if (o_port > 0) { /* test for illegal value */
       sad0.sin_port = htons((u_short)o_port);
    } 
    else { /* print error message and exit */
        fprintf(stderr,"Error: Bad port number %s\n",argv[2]);
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

    sd2 = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd2 < 0) {
        fprintf(stderr, "Error: Socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    /* Bind a local address to the socket */
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) { // socket for partcipant
        fprintf(stderr,"Error: Bind failed\n");
        exit(EXIT_FAILURE);
    }
    
    if (bind(sd2, (struct sockaddr *)&sad0, sizeof(sad0)) < 0) { // socket for oberserver
        fprintf(stderr,"Error: Bind failed0\n");
        exit(EXIT_FAILURE);
    }

    /* Specify size of request queue */
    if (listen(sd, QLEN) < 0) {
        fprintf(stderr,"Error: Listen failed\n");
        exit(EXIT_FAILURE);
    }
  
    if (listen(sd2, QLEN) < 0) {
        fprintf(stderr,"Error: Listen failed\n");
        exit(EXIT_FAILURE);
    }


        /* Main server loop - accept and handle requests */
	while (1) {
        FD_ZERO(&readSd);
    	FD_SET(sd2, &readSd);
    	FD_SET(sd, &readSd);
		alen = sizeof(cad);
        
        for(i = 0; i<=63; i++){                              /*Update fd set readSd with most recent participant and observer sds*/                     
            if(p_clients[i] != 0){
                FD_SET(p_clients[i], &readSd);
            }
        }  

        for(i = 0; i<=63; i++){
            if(o_clients[i] != 0){
                FD_SET(o_clients[i], &readSd);
            }
        } 


    	ready_sd = select(133, &readSd, NULL, NULL, NULL);

        if(ready_sd < 0){
            fprintf(stderr, "Error: select failed\n");
            exit(EXIT_FAILURE);
        }
    	
        /*Handles activity on listening partcipant socket connection*/
        if((FD_ISSET(sd, &readSd) != 0)){
        	if ( (newP_sock =accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {  
            	fprintf(stderr, "Error: Accept failed\n");
                exit(EXIT_FAILURE);
            }

            for(i = 0; i<=63; i++){                         /*Assign user number and store sd*/
                if(p_clients[i] == 0){
                    p_clients[i] = newP_sock;
                    break;
                }
            }

            if(i == 64){                                    /*Max participant users reached close socket*/
                close(newP_sock);
            }   
            else{                                           /*send observers message new participant client has joined*/
                sprintf(buf,"User %d has joined\n",i);  
                for(j = 0; j<=63; j++){
                    if(o_clients[j] != 0){    
                        send(o_clients[j],buf,strlen(buf),0);
                    }       
                }
            }
        }

        /*Handles activity on listening observer socket connection*/
    	if((FD_ISSET(sd2, &readSd) != 0)){
        	if ( (newO_sock =accept(sd2, (struct sockaddr *)&cad, &alen)) < 0) {
            	fprintf(stderr, "Error: Accept failed\n");
                exit(EXIT_FAILURE);
            }
    	
            for(i = 0; i<=63; i++){                     /*store obeserver sd*/
                if(o_clients[i] == 0){
                    o_clients[i] = newO_sock;
                    break;
                }
            }
            if(i == 64){                                /*Max observer clients reached close socket*/
                close(newO_sock);
            }                                       
            else{
                sprintf(buf,"A new observer has joined\n");   /*send observers message new observer client has joined*/
                for(j = 0; j<=63; j++){
                    if(o_clients[j] != 0){    
                        send(o_clients[j],buf,strlen(buf),0);
                    }          
                }
            }
        }
    
        /*not a new connection so it must be activity on a partcipant or observer*/ 
        

    	/*loop through all partcipant clients to try and find a socket thats active */ 
        for(i = 0; i<= 63; i++){
            if((FD_ISSET(p_clients[i], &readSd) != 0)){

                if((bytesRead = read(p_clients[i], pbuf, sizeof(pbuf))) == 0){ /*alert was because that socket closed*/
                    close(p_clients[i]);
                    FD_CLR(p_clients[i], &readSd);
                    p_clients[i] = 0;
                    sprintf(buf,"User %d has left\n",i);        /*send observers message user i left*/
                    for(j = 0; j<=63; j++){    
                        if(o_clients[j] != 0){     
                            send(o_clients[j],buf,strlen(buf),0);
                        }             
                    }
                }
                else{                                              /*alert was becasue participant has sent a message*/
                    for(m = 0; m <= bytesRead-1; m++){
                            rbuf[m + 4] = pbuf[m];
                    }
  
                    userNum = sprintf(user, "%d", i);              /*prepend user id to message */
                    if(userNum == 2){
                        rbuf[0] = user[0];
                        rbuf[1] = user[1];
                        rbuf[2] = ':';
                        rbuf[3] = ' ';
                    }
                    else{
                        rbuf[0] = ' ' ;
                        rbuf[1] = user[0];
                        rbuf[2] = ':';
                        rbuf[3] = ' ';
                    }
                    if(bytesRead == 1024){                      /*add newLine to message then send to observers*/
                        rbuf[1028] = '\n'; 
                        for(j = 0; j<=63; j++){
                            if(o_clients[j] != 0){
                                send(o_clients[j],rbuf,bytesRead + 5,0);
                            }       
                        }
                    }
                    else{                                       /*send message to observers*/
                        for(j = 0; j<=63; j++){
                            if(o_clients[j] != 0){
                            send(o_clients[j],rbuf,bytesRead + 4,0);
                            }       
                        }
                    }                
                }
            }    
        }


        /*Activity on observer clients (client closed)*/
        for(i = 0; i <= 63; i++){
            if((FD_ISSET(o_clients[i], &readSd) != 0)){
                if((bytesRead = read(o_clients[i], pbuf, 1024)) == 0){
                    close(o_clients[i]);
                    FD_CLR(o_clients[i], &readSd);
                    o_clients[i] = 0;
                    sprintf(buf,"A observer has left\n");   
                    for(j = 0; j<=63; j++){
                        if(o_clients[j] != 0){    
                            send(o_clients[j],buf,strlen(buf),0);
                        }             
                    }
                }
            }
        }
	}
}