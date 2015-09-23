/* prog3_server.c - code for server program that runs a jeopardy like game */

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#define QLEN 6 /* size of request queue */
#define MAX_PARTICIPANT 8 /* number of max participants */
#define MAX_OBSERVERS 16 /* number of max observers */



/*Structure that holds all participant information*/
typedef struct participant {
    char user_name[17];            /*stores playeers use name*/
    int paricipant_sd;             /*stores players sd */
    int score;                     /*stores players score*/
    int lastrnd_Answered;          /*stores players last round answered */
    int validName;                 /*stores players name has been validated*/
    int correct;                   /*stores player answer corectly or not*/
    int wait;                      /*stores whether player should wait to be added*/
    int qAnswered;                 /*stores whether player answered the question*/
    char answer[1024];             /*stores players answers*/
    int midround;
} PAR;

/*Structure that holds all question information*/
struct question {
    char category [100];
    int value;
    char question [1024];
    char answer  [100];
};

PAR p_clients[8];               /* Array storing participant structures */
int o_clients[16];               /* Array storing observer sd numbers */
char correctA[136];              /*stores the user name of users who got answer  correct*/
char wrongA[136];                /*stores the user name of users who got answer wrong*/
struct question currentQuestion; /*stores the current question being asked*/
char board[1024];                /*stores the score board*/
int inround = 0;                 /*stores whether we are in a round or not*/
int roundNum = 0;                /*stores the round number*/
int numParticipants = 0;         /*stores the number of particiapnts*/
fd_set readSd;                   /*fd_set for partcipants and observers*/


char validateName (char uname [16], int nameSize);
void addPlayer();
void kickUsers();
char * leaderBoard();
void printScore();

/*
Checks the last round a player answered and if its equal to 3 rounds 
from current round then they are kicked from the game
*/
void kickUsers() {
    int i;
    int j;
    char kickedUsers[1024];

    for (i = 0; i < MAX_PARTICIPANT; i++){
        if ((roundNum - p_clients[i].lastrnd_Answered) == 3 && p_clients[i].validName == 1){
            close(p_clients[i].paricipant_sd);
            FD_CLR(p_clients[i].paricipant_sd, &readSd);
            p_clients[i].paricipant_sd = 0;
            p_clients[i].validName = 0;
            numParticipants--;
            if(numParticipants == 0){
                roundNum = 0;
            }
            p_clients[i].lastrnd_Answered = 0;
            sprintf(kickedUsers, "User %s was kicked.\n", p_clients[i].user_name);
            memset(&p_clients[i].user_name[0], 0, strlen(p_clients[i].user_name));
            for (j = 0; j < MAX_OBSERVERS; j++){
                if(o_clients[j] != 0){
                    send(o_clients[j], kickedUsers, strlen(kickedUsers), 0);
                }
            }
        }
    }
}

/*
checks if player connected during a round and had to wait
sets wait feild back to 0 and sends messages to observer 
that user hsas been added
*/

void addPlayer(){
    int i;
    int j;
    char temp3[1024];

    for(i = 0; i < MAX_PARTICIPANT; i++){
        if(p_clients[i].wait == 1){
            p_clients[i].wait = 0;
            sprintf(temp3, "User %s has joined.\n", p_clients[i].user_name);
            for (j = 0; j < MAX_OBSERVERS; j++){
                send(o_clients[j],temp3,strlen(temp3), 0);
            }
        }
    }
}

/*
Takes in the players name and name size
Validates the name makign sure it is unique and of proper size
Returns: V = valid name
         I = invalid name
*/
char validateName (char uname[17], int nameSize) {
    int i;
    int j;
    if(nameSize < 17 ){
        int p = nameSize - 1;
        while(p != 16){
            uname[p] = ' ';
            p++;
        }
        uname[16] = '\0';
    }
    if(nameSize == 17){
        uname[16] = '\0';
    }
    if (nameSize > 17) {
        return('I');
    }
    for(i = 0; i < MAX_PARTICIPANT; i++) {
        if (strcasecmp (uname, p_clients[i].user_name) == 0){
            return('I');
        }
    }
    if (i == MAX_PARTICIPANT){
        return('V');
    }
}

/*
Creates and returns the leader board

*/
char * leaderBoard() {
    int i;
    int j;
    char tempScore[8];
    for (i = 0; i <= 27; i++) {
        if (i == 27) {
            strcat(board, "\n");
        }
        else if (i < 27){
            strcat(board, "-");
        }
    }
    strcat(board, "|     Player     | Score |\n");
    for (i = 0; i <= 27; i++) {
        if (i == 27) {
            strcat(board, "\n");
        }
        else if (i < 27){
            strcat(board, "-");
        }
    }
    for (i = 0; i < MAX_PARTICIPANT; i++) {
        if(p_clients[i].validName == 1 && p_clients[i].wait != 1){
            sprintf(tempScore, "%d", p_clients[i].score);
            if ((j = strlen(tempScore)) != 7){
                for (j; j < 7; j++){
                    tempScore[j] = ' ';
                }
            }
            tempScore[7] = '\0';
            strcat(board, "|");
            strcat(board, p_clients[i].user_name);
            strcat(board, "|");
            strcat(board, tempScore);
            strcat(board, "|\n");
        }
    }
    return board;
} 


/*
Sets inround = 0
Sends out players who got the answer correct
Sends out players who got the answer incorrect
kicks users out of game if ther are any who need to be kicked
Sends out the correct answer
Send out the leaderboard
*/
void printScore(){
    int i = 0;
    char correctSend [] = "These players got the answer correct:\n";
    char wrongSend[] = "These players got the answer wrong:\n";
    char answer[] = "The correct answer is:\n";
    char *lBoard;
    inround = 0;

    for(i = 0; i < MAX_PARTICIPANT; i++){
        if (p_clients[i].validName == 1){
            if(p_clients[i].correct == 1 && p_clients[i].qAnswered == 1){
                p_clients[i].qAnswered = 0;
                char temp1 [strlen(p_clients[i].user_name) ];
                sprintf(temp1,"%s\n", p_clients[i].user_name);
                strcat(correctA, temp1);
            } 
            else if (p_clients[i].correct == 0 && p_clients[i].qAnswered == 1){
                p_clients[i].qAnswered = 0;
                char temp2 [strlen(p_clients[i].user_name) ];
                sprintf(temp2, "%s\n", p_clients[i].user_name);
                strcat(wrongA, temp2);
            }
        }
    }
    for(i = 0; i<MAX_OBSERVERS; i++){
        if(o_clients[i] != 0){
            send(o_clients[i], correctSend, strlen(correctSend), 0);
            send(o_clients[i], correctA, strlen(correctA), 0);
        }    
    }
    memset(&correctA[0], 0, strlen(correctA));
    for(i = 0; i<MAX_OBSERVERS; i++){
        if(o_clients[i] != 0){
            send(o_clients[i], wrongSend, strlen(wrongSend), 0);
            send(o_clients[i], wrongA, strlen(wrongA), 0);
        }    
    }
    memset(&wrongA[0], 0, strlen(wrongA));
    
    kickUsers();

    for(i = 0; i<MAX_OBSERVERS; i++){
        if(o_clients[i] != 0){
            send(o_clients[i], answer, strlen(answer), 0);
            send(o_clients[i], currentQuestion.answer, strlen(currentQuestion.answer), 0);
        }    
    }

    lBoard = leaderBoard();
    for(i = 0; i<MAX_OBSERVERS; i++){
        if(o_clients[i] != 0){
            send(o_clients[i], lBoard, strlen(lBoard), 0);
        }    
    }
    memset(&board[0], 0, strlen(board));
} 



  


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
    char pbuf[1024];                /* buffer for message the partcipant sends */
    int i;                          /* int to iterate through arrays */
    int j;                          /* int to iterate through arrays */
    int ready_sd;                   /* int stores number of ready sds returned by select */
    int newP_sock;                  /* socket descriptors for participants */
    int newO_sock;                  /* socket descriptors for observers*/
    int m;                          /* int to iterate through arrays */
    int bytesRead;                  /* int stores number bytes read returned by read */
    char vName;                     /* stores the value returned by validateName functino */
    int z;                          /* stores size of return_char */
    int numObservers = 0;           /* stores the number of observers */
    char return_char;               /* char that store "F" when server is full */
    char qbuf[1024];                /* buffer used while parsing questions from stdin */
    struct question *q_array = malloc(202754*sizeof(*q_array)); /*array storing all the question structs*/
    char *p;                        /* pointer use when parsing questions from stdin */
    int k = 0;                      /* in used to iterate through arrays */
    int questionNum = 0;            /* stores the question number */
    struct timeval timeout;         /* struct timeval used with select */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    char pbufU[1024];               /* buffer used to update players answer by removing spaces */


    if( argc != 3 ) {
        fprintf(stderr,"Error: Wrong number of arguments\n");
        fprintf(stderr,"usage:\n");
        fprintf(stderr,"./server server_port-partcipants server_port-observers\n");
        exit(EXIT_FAILURE);
    }

    while(fgets(qbuf, sizeof(qbuf), stdin) != NULL) {   /*read through stdin and create q_array full of question structs*/
        strtok(qbuf, "\t");
        strcpy(q_array[k].category, qbuf);
        p = (char*)strtok(NULL, "\t");
        p = p + 1;
        q_array[k].value = atoi(p);
        p = (char*)strtok(NULL, "\t");
        strcpy(q_array[k].question, p);
        p = (char*)strtok(NULL, "\t");
        strcpy(q_array[k].answer, p);
        k++;
    }

    for(i = 0; i<MAX_OBSERVERS; i++){         /*Initialize observer and partcipant client arrays to 0*/
        o_clients[i] = 0;
    }       
	for(i = 0; i<MAX_PARTICIPANT; i++){
        p_clients[i].paricipant_sd = 0;
        p_clients[i].validName = 0;
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

        if(inround == 0 && roundNum != 0){              /*sleep then addplayers who tried connecting during a round*/
            sleep(5);
            addPlayer();
        }




        for(i = 0; i<MAX_PARTICIPANT; i++){                              /*Update fd set readSd with most recent participant and observer sds*/                     
            if(p_clients[i].paricipant_sd != 0 && p_clients[i].wait != 1){
                FD_SET(p_clients[i].paricipant_sd, &readSd);
            }
        } 

        for(i = 0; i<MAX_OBSERVERS; i++){
            if(o_clients[i] != 0){
                FD_SET(o_clients[i], &readSd);
            }
        } 

        if(roundNum != 0){
            while(ready_sd = select(29, &readSd, NULL, NULL, &timeout) > 0){
                if((FD_ISSET(sd, &readSd) != 0)){
                    if ( (newP_sock =accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {  
                        fprintf(stderr, "Error: Accept failed\n");
                        exit(EXIT_FAILURE);
                    }
                }

                for(i = 0; i<MAX_PARTICIPANT; i++){                            /*store sd*/
                    if(p_clients[i].paricipant_sd == 0){
                        p_clients[i].paricipant_sd = newP_sock;
                        FD_SET(p_clients[i].paricipant_sd, &readSd);
                        break;
                    }
                }

                for(i = 0; i < MAX_PARTICIPANT; i++){
                    if((FD_ISSET(p_clients[i].paricipant_sd, &readSd) != 0)){
                        if((bytesRead = read(p_clients[i].paricipant_sd, pbuf, sizeof(pbuf))) != 0 && p_clients[i].validName == 1){
                            continue;
                        }
                        else{
                            close(p_clients[i].paricipant_sd);
                            FD_CLR(p_clients[i].paricipant_sd, &readSd);
                            p_clients[i].paricipant_sd = 0;
                            p_clients[i].validName = 0;
                            numParticipants--;
                            if(numParticipants == 0){
                                roundNum = 0;
                            }
                        }                       
                    }              
                }
            }
            FD_ZERO(&readSd);
            FD_SET(sd2, &readSd);
            FD_SET(sd, &readSd);
       } 


        for(i = 0; i<MAX_PARTICIPANT; i++){                              /*Update fd set readSd with most recent participant and observer sds*/                     
            if(p_clients[i].paricipant_sd != 0 && p_clients[i].wait != 1){
                FD_SET(p_clients[i].paricipant_sd, &readSd);
            }
        } 

        for(i = 0; i<MAX_OBSERVERS; i++){
            if(o_clients[i] != 0){
                FD_SET(o_clients[i], &readSd);
            }
        } 



        if(inround == 0){
            if(numObservers >= 1 && numParticipants >=1){
                currentQuestion = q_array[questionNum];
                for(j = 0; j < MAX_PARTICIPANT; j++){
                    if(p_clients[j].validName == 1){
                        memset(&p_clients[j].answer[0], 0, strlen(p_clients[j].answer));        /* clear all patricipant buffers */
                    }
                    if(p_clients[j].midround == 0){
                        p_clients[j].midround = 0;
                    }    
                }
                questionNum++;
                roundNum++;
                inround = 1;
                for(j = 0; j < MAX_OBSERVERS; j++){             /*start round by sending question to all observers*/
                    if(o_clients[j] != 0){
                        char sendQuestion [1024];
                        sprintf(sendQuestion, "The category is: %s\nThe question is: %s\nThe value is: %d\n", currentQuestion.category, currentQuestion.question, currentQuestion.value);
                        send(o_clients[j], sendQuestion, strlen(sendQuestion), 0);
                    }             
                }
                sleep(15);
            }
         }   

        while(ready_sd = select(29, &readSd, NULL, NULL, &timeout) > 0){
            if(ready_sd < 0 && errno !=  EINTR){
                fprintf(stderr, "Error: select failed\n");
                exit(EXIT_FAILURE);
            }
    	
            /*Handles activity on listening partcipant socket connection*/
            if((FD_ISSET(sd, &readSd) != 0)){
        	    if ( (newP_sock =accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {  
                    fprintf(stderr, "Error: Accept failed\n");
                    exit(EXIT_FAILURE);
                }

                for(i = 0; i<MAX_PARTICIPANT; i++){                            /*store sd*/
                    if(p_clients[i].paricipant_sd == 0){
                        p_clients[i].paricipant_sd = newP_sock;
                        break;
                    }

                }
                if(inround == 1){
                    p_clients[i].wait = 1;
                    FD_SET(p_clients[i].paricipant_sd, &readSd);
                }
                if(i == 8){
                    return_char = 'F';
                    z = sprintf(buf, "%c", return_char);
                    send(newP_sock,buf,z,0);                                    /*Max participant users reached send 'F' to participant*/
                }
            }

            /*Handles activity on listening observer socket connection*/
    	    if((FD_ISSET(sd2, &readSd) != 0)){
        	    if ( (newO_sock =accept(sd2, (struct sockaddr *)&cad, &alen)) < 0) {
                    fprintf(stderr, "Error: Accept failed\n");
                    exit(EXIT_FAILURE);
                }
    	
                for(i = 0; i<MAX_OBSERVERS; i++){                     /*store obeserver sd*/
                    if(o_clients[i] == 0){
                        o_clients[i] = newO_sock;
                        break;
                    }
                }
                if(i == 16){                                /*Max observer clients reached close socket*/
                    close(newO_sock);
                }
                else{
                    numObservers++;
                }                                       
            }
    
                /*not a new connection so it must be activity on a partcipant or observer*/ 
        

    	       /*loop through all partcipant clients to try and find a socket thats active */ 
            for(i = 0; i < MAX_PARTICIPANT; i++){
                if((FD_ISSET(p_clients[i].paricipant_sd, &readSd) != 0)){
                    if((bytesRead = read(p_clients[i].paricipant_sd, pbuf, sizeof(pbuf))) == 0){         /*alert was because that socket closed*/
                        close(p_clients[i].paricipant_sd);
                        FD_CLR(p_clients[i].paricipant_sd, &readSd);
                        p_clients[i].paricipant_sd = 0;
                        p_clients[i].validName = 0;
                        numParticipants--;
                        if(numParticipants == 0){
                            roundNum = 0;
                        }
                    }
                    if(p_clients[i].validName == 0){
                        vName = validateName(pbuf, bytesRead); /*Validate players name and add to pclients array*/
                        z = sprintf(buf, "%c", vName);
                        send(p_clients[i].paricipant_sd, buf, z, 0); 
                        if (vName == 'V'){
                            numParticipants++;
                            bytesRead--;
                            for(m = 0; m <bytesRead; m++){
                                p_clients[i].user_name[m] = pbuf[m];
                            }
                            p_clients[i].score = 2000;
                            p_clients[i].validName = 1;
                            p_clients[i].qAnswered = 0;
                            p_clients[i].lastrnd_Answered = roundNum;
                            
                            if ((j = strlen(p_clients[i].user_name)) < 16){
                                for (j; j < 16; j++){
                                    p_clients[i].user_name[j] = ' ';
                                }
                                p_clients[i].user_name[16] = '\0';
                            }
                        }
                    }
                    if(p_clients[i].validName == 1 && inround == 1 && p_clients[i].qAnswered != 1 && p_clients[i].wait != 1 && p_clients[i].midround != 1){ /*handle answers to question inround*/
                        pbuf[bytesRead] = '\0';
                        j = 0;
                        k = 0;
                        while(pbuf[k] != '\0'){                     /*transfers players answer to players specified buffer*/
                            p_clients[i].answer[j] = pbuf[k];
                            k++;
                            j++;
                        }
                        p_clients[i].answer[j] = '\n';
                        j = 0;
                        k = 0;
                        int a = 0;
                        while(p_clients[i].answer[j] != '\n'){      /*remove white spaces from players answer*/
                            if (!isspace(p_clients[i].answer[j])){
                                pbufU[a] = p_clients[i].answer[j];
                                a++;
                            }
                            j++;
                        }
                        pbufU[a] = '\n';
                        pbufU[a + 1] = '\0';

                        char answerbuf[1024]; /*stores question answer with spaces removed*/
                        a = 0;
                    
                        while(currentQuestion.answer[k] != '\n'){  /*remove white spaces from the question answer*/
                            if (!isspace(currentQuestion.answer[k])){
                                answerbuf[a] = currentQuestion.answer[k];
                                a++;
                            }
                            k++;
                        }
                        answerbuf[a] = '\n';
                        answerbuf[a+1] = '\0';

                        if(strcasecmp(pbufU, answerbuf) == 0){      /*check players response with questions answer*/
                            p_clients[i].score = p_clients[i].score + currentQuestion.value; /*players correct update score and othe feilds*/
                            p_clients[i].correct = 1;
                            p_clients[i].qAnswered = 1;
                            p_clients[i].lastrnd_Answered = roundNum;
                        }
                        else{
                            p_clients[i].score = p_clients[i].score - (currentQuestion.value / 2); /*players incorrect update score and othe feilds*/
                            p_clients[i].qAnswered = 1;
                            p_clients[i].correct = 0;
                            p_clients[i].lastrnd_Answered = roundNum;
                        }
                    }   
                }    
            }
    
            /*Handle activity on a observer, which will only be when it closes*/
            for(i = 0; i < MAX_OBSERVERS; i++){
                if((FD_ISSET(o_clients[i], &readSd) != 0)){
                    if((bytesRead = read(o_clients[i], pbuf, 1024)) == 0){
                        close(o_clients[i]);
                        FD_CLR(o_clients[i], &readSd);
                        numObservers--;
                        o_clients[i] = 0;
                        sprintf(buf,"An observer has left\n");   
                        for(j = 0; j< MAX_OBSERVERS; j++){
                            if(o_clients[j] != 0){    
                                send(o_clients[j],buf,strlen(buf),0);
                            }             
                        }
                    }
                }
            }
        }
        
        if(numObservers>=1 && numParticipants>=1 && roundNum > 0){ /*print out scoreboard and do other procedures called within printScore*/
            printScore();
        }
	}
}
