/* prog1_server.c - code for server side of connect four game program that uses TCP */

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define QLEN 6 /* size of request queue */


/*------------------------------------------------------------------------
* Program: server
*
* JP Sohal
*
* Purpose: allocate a socket and then repeatedly execute the following:
*  1. wait for the next connection from a client
*  2. wait till number of players is equal to two
*  2. fork process to handle coneect_four_game interaction
*  3. recieve move location from client
*  4. update game_state based on move location from client
*  5. check if move has resulted in a win
*  6. check if move has resulted in a tie 
*  7. send back response, which is determined by the move location
*  8. if a win or tie has occured  break loop and close sockets else go back to 3
*
* Syntax: server [ port ] game_type
*
* port - protocol port number to use
* game_type - the version of connect four that is to be played(standard, popout, and antistack) 	
*
* Note: The port argument is optional. If no port is specified,
* the server uses the default given by PROTOPORT.
*
*------------------------------------------------------------------------
*/



void connect_four_game(int sd2, int sd3, char *buf, char game_state [], bool player1_turn, char *game_type);
char * update_game_state(char *moveBuf, char game_state [], int active_socket, char *buf, bool player1_turn, int move_loc, char *game_type);
bool check_for_winner(int move_loc, char game_state [], bool player1_turn, char *game_type);
bool check_for_tie(char game_state []);


/*
Pre: takes in updated game state
Post: returns bool based on whether game is a tir oe not
*/

bool check_for_tie(char game_state []){

	bool tie = false;	/*bool holds value of tie*/
	int i = 0;			/*used to iterate through game state*/
	
	while(game_state[i] != '0' && i != 42){
			i++;
	}
	if(i == 42){
		tie = true;
	}
	return tie;	
}


/*
Pre: takes in move_loc(col of move), updated game state, player1 turn and game type
Post: returns a bool based on wheter one of players has won or not 
*/
bool check_for_winner(int move_loc, char game_state [], bool player1_turn, char *game_type){

	char player_symbol;		/* stores active players player sumbol */
	bool winner = false;	/* bool hold whether win has happend */
	int i = 1;				/* used to iterate through game_state */
	int num_tokens = 1;		/* holds number of tokens that are aligned together */
	int k = 0;				/* used to iterate through game_state to get actual move location */
	int win_numtokens;		/* holds the number of tokens needed to win or lose based on game type */
	
	while(game_state[move_loc + k] == '0' && k != 42){
			k = k + 7;
	}
	move_loc = move_loc + k;
	
	if(player1_turn){
		player_symbol = '1';
	}
	else{
		player_symbol = '2';
	}		

	if(strcmp (game_type, "antistack") == 0){
		win_numtokens = 3;
	}	
	else{
		win_numtokens = 4;
	}	
	
	while(game_state[move_loc + i] == player_symbol && (move_loc + i)  % 7 != 0){			//checks row infront of token
		num_tokens++;
		i++;
	}
	i = 1;
    
    while(game_state[move_loc - i] == player_symbol && (move_loc - i)  % 7 != 0 && move_loc % 7 != 0){				//check row behind token
		num_tokens++;
		i++;
	}
	if(game_state[move_loc - i] == player_symbol && (move_loc - i)  % 7 == 0){
		num_tokens++;
	}
	
	if(num_tokens == win_numtokens){
		winner = true;
	}
	else{
		num_tokens = 1;
		i = 7;
	}
	while(game_state[move_loc + i] == player_symbol){												//checks col below token
		num_tokens++;
		i = i + 7;
	}
	if(num_tokens == win_numtokens){
		winner = true;
	}
	else{
		num_tokens = 1;
		i = 8;
	}
	while(game_state[move_loc + i] == player_symbol && (move_loc + i) % 7 != 0){				//checks diagonally below right of token
		num_tokens++;
		i = i + 8;
	}
	i = 8;
	while(game_state[move_loc - i] == player_symbol && ((move_loc - i) + 1) % 7 != 0){			//check diagonally above left of  token
		num_tokens++;
		i = i + 8;
	}
	if(num_tokens == win_numtokens){
		winner = true;
	}
	else{
		num_tokens = 1;
		i = 6;
	}	
	while(game_state[move_loc + i] == player_symbol && ((move_loc + i) + 1) % 7 != 0){			//checks diagonally below left of token
		num_tokens++;
		i = i + 6;
	}
	i = 6;
	while(game_state[move_loc - i] == player_symbol && (move_loc - i) % 7 != 0){				//check diagonally above right of token
		num_tokens++;
		i = i + 6;
	}
	if(num_tokens == win_numtokens){
		winner = true;
	}
	
	return winner;

}



/*
Pre: takes in moveBuf, current game state, active socket descriptor, buf, player1 bool, move location, gametype
Post: returns updated game sate
*/
char * update_game_state(char *moveBuf, char game_state [], int active_socket, char *buf, bool player1_turn, int move_loc, char *game_type){
	
	char invalid_move = 'I'; /* character sent to indicate invalid move */
	int move_len;			 /* stores legth of move reived from client */
	int k = 0;				 /* used to iterate through game board if move is 'A' */
	bool valid_move = true;  /* stores whether move is valid or not */
	int j = 35;				 /* used to iterate through game boad if move is 'P' */
	char player_symbol;		 /* stores active players player symbol */	
	
	if(player1_turn){
		player_symbol = '1';
	}
	else{
		player_symbol = '2';
	}	
	
	if((strcmp (game_type, "standard") == 0) || (strcmp (game_type, "antistack") == 0)){
		if((moveBuf[0] != 'A') || (move_loc > 6 || move_loc < 0) || game_state[move_loc] != '0' || moveBuf[2] != '\n') {
			sprintf(buf, "%c", invalid_move);
			send(active_socket, buf, strlen(buf), 0);
			move_len = recv(active_socket, moveBuf, sizeof(moveBuf), 0);							
			moveBuf[move_len] = '0';
			move_loc = moveBuf[1] - '0';
		    game_state = update_game_state(moveBuf, game_state, active_socket, buf, player1_turn, move_loc, game_type);
		    valid_move = false;
		}
		if(valid_move){
			while(game_state[move_loc + k] == '0' && k != 42){
				k = k + 7;
			}
			if(player1_turn){
				game_state[(k-7) + move_loc] = '1';
			}
			if(!player1_turn){
				game_state[(k-7) + move_loc] = '2';
			}
		}
	}
	
	if(strcmp (game_type, "popout") == 0){
		if((moveBuf[0] != 'A' && moveBuf[0] != 'P' ) || (move_loc > 6 || move_loc < 0) || moveBuf[2] != '\n') {
			sprintf(buf, "%c", invalid_move);
			send(active_socket, buf, strlen(buf), 0);
			move_len = recv(active_socket, moveBuf, sizeof(moveBuf), 0);							
			moveBuf[move_len] = '0';
			move_loc = moveBuf[1] - '0';
		    game_state = update_game_state(moveBuf, game_state, active_socket, buf, player1_turn, move_loc, game_type);
		    valid_move = false;
		}
		

		if((moveBuf[0] == 'P' && game_state[move_loc + 35] != player_symbol) || (moveBuf[0] == 'A' && game_state[move_loc] != '0')){
			sprintf(buf, "%c", invalid_move);
			send(active_socket, buf, strlen(buf), 0);
			move_len = recv(active_socket, moveBuf, sizeof(moveBuf), 0);							
			moveBuf[move_len] = '0';
			move_loc = moveBuf[1] - '0';
		    game_state = update_game_state(moveBuf, game_state, active_socket, buf, player1_turn, move_loc, game_type);
		    valid_move = false;
		}

		if(valid_move && moveBuf[0] == 'A'){
			while(game_state[move_loc + k] == '0' && k != 42){
				k = k + 7;
			}
			if(player1_turn){
				game_state[(k-7) + move_loc] = '1';
			}
			if(!player1_turn){
				game_state[(k-7) + move_loc] = '2';
			}
		}
	
		if(valid_move && moveBuf[0] == 'P'){
			while(game_state[move_loc + j] != '0' && j != -7){
				if(j == 0){
					game_state[move_loc + j] = '0';
				}
				else{	
					game_state[move_loc + j] = game_state[(move_loc + j) - 7];
					j = j - 7;
				}	
			}
		}	
	}
	
	return game_state;
}

/*
Pre: takes in player2 sd, player1 sd, buf, game state, player1 turn and game type
Post: runs the game until a win or tie has occured
*/
void connect_four_game(int sd2, int sd3, char *buf, char game_state [], bool player1_turn,char *game_type){

	int move_len;			/* stores legth of move reived from client */
	char your_turn = 'Y';	/* character sent indicate your turn */
	char hold_turn = 'H';	/* character sent indicate hold till your turn */
	char you_win = 'W';		/* character sent indicate win */
	char you_lose = 'L';    /* character sent indcate lose */
	char you_tie = 'T';		/* character sent indicate tie */
	char moveBuf[4];		/* stores move made by player */
	bool winner = false;	/* stores whether win has occured */
	bool tie = false;		/* stores whether tie has occured */
	int move_loc;           /* stores players move location (column) */
	
	
	while(!winner && !tie){
		if(player1_turn){	
			sprintf(buf, "%c", your_turn);
			send(sd3, buf, strlen(buf), 0);
			sleep(1);
			send(sd3, game_state, strlen(game_state), 0);
			sprintf(buf, "%c", hold_turn);
			send(sd2, buf, strlen(buf), 0);
			sleep(1);
			send(sd2, game_state, strlen(game_state), 0);
			move_len = recv(sd3, moveBuf, sizeof(moveBuf), 0);
			moveBuf[move_len] = '0';
			move_loc = moveBuf[1] - '0';
			game_state = update_game_state(moveBuf, game_state, sd3, buf, player1_turn, move_loc, game_type);
			winner = check_for_winner(move_loc,game_state,player1_turn, game_type);
			tie = check_for_tie(game_state);
			
			if(winner && ((strcmp (game_type, "standard") == 0)  || (strcmp (game_type, "popout") == 0))){
				sprintf(buf, "%c", you_win);
				send(sd3, buf, strlen(buf), 0);
				close(sd3);
				sleep(1);
				sprintf(buf, "%c", you_lose);
				send(sd2, buf, strlen(buf), 0);
				close(sd2);
			}
			else if(winner && (strcmp (game_type, "antistack") == 0)){
				sprintf(buf, "%c", you_win);
				send(sd2, buf, strlen(buf), 0);
				close(sd2);
				sleep(1);
				sprintf(buf, "%c", you_lose);
				send(sd3, buf, strlen(buf), 0);
				close(sd3);
			}
			
			if(tie){
				sprintf(buf, "%c", you_tie);
				send(sd3, buf, strlen(buf), 0);
				close(sd3);
				sleep(1);
				sprintf(buf, "%c", you_tie);
				send(sd2, buf, strlen(buf), 0);
				close(sd2);
			}
			player1_turn = false;		
		}
		
		if(!player1_turn && !winner && !tie){	
			sprintf(buf, "%c", your_turn);
			send(sd2, buf, strlen(buf), 0);
			sleep(1);
			send(sd2, game_state, strlen(game_state), 0);
			sprintf(buf, "%c", hold_turn);
			send(sd3, buf, strlen(buf), 0);
			sleep(1);
			send(sd3, game_state, strlen(game_state), 0);
			move_len = recv(sd2, moveBuf, sizeof(moveBuf), 0);
			moveBuf[move_len] = '0';
			move_loc = moveBuf[1] - '0';
			game_state = update_game_state(moveBuf, game_state, sd2, buf, player1_turn, move_loc, game_type);
			winner = check_for_winner(move_loc,game_state,player1_turn, game_type);
			tie = check_for_tie(game_state);
			
			if(winner && ((strcmp (game_type, "standard") == 0)  || (strcmp (game_type, "popout") == 0))){
				sprintf(buf, "%c", you_win);
				send(sd2, buf, strlen(buf), 0);
				close(sd2);
				sleep(1);
				sprintf(buf, "%c", you_lose);
				send(sd3, buf, strlen(buf), 0);
				close(sd3);
			}
			else if(winner && (strcmp (game_type, "antistack") == 0)){
				sprintf(buf, "%c", you_win);
				send(sd3, buf, strlen(buf), 0);
				close(sd3);
				sleep(1);
				sprintf(buf, "%c", you_lose);
				send(sd2, buf, strlen(buf), 0);
				close(sd2);
			}
			
			if(tie){
				sprintf(buf, "%c", you_tie);
				send(sd3, buf, strlen(buf), 0);
				close(sd3);
				sleep(1);
				sprintf(buf, "%c", you_tie);
				send(sd2, buf, strlen(buf), 0);
				close(sd2);
			}
			player1_turn = true;		
		}	
	}
}




int main(int argc, char **argv) {
	struct protoent *ptrp; 		/* pointer to a protocol table entry */
	struct sockaddr_in sad; 	/* structure to hold server's address */
	struct sockaddr_in cad; 	/* structure to hold client's address */
	int sd, sd2, sd3; 			/* socket descriptors */
	int port; 					/* protocol port number */
	int alen; 					/* length of address */
	char buf[1000]; 			/* buffer for string the server sends */
	pid_t cpid; 				/* child pid from fork */
	char *game_type; 			/* game type given from command line */
	char gameT_char;			/* game type character sent to client */
	char waiting_char = '2';	/* char 2 indicating waitting for player 2 */
	int num_players = 0;		/* stores the amount of players */
	char game_state [42];		/* array that stores the game state */
	int i = 0; 					/* index iterator used when creating game state */;
	bool player1_turn = true; 	/* bool for player one turn */
	
	
	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port game_type\n");
		exit(EXIT_FAILURE);
	}

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */
	sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

	port = atoi(argv[1]); /* convert argument to binary */
	game_type = argv[2];
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
		num_players++;
		
		
		if (strcmp (game_type, "standard") == 0){
			gameT_char = 'S';
		}
		
		if (strcmp (game_type, "popout") == 0){
			gameT_char = 'P';
		}
		if (strcmp (game_type, "antistack") == 0){
			gameT_char = 'K';
		}
		
		sprintf(buf, "%c", gameT_char);
		send(sd2, buf, strlen(buf), 0);
		sleep(1);
		if (num_players == 1){
			sprintf(buf, "%c", waiting_char);
			send(sd2, buf, strlen(buf), 0);
			sd3 = sd2;
		}
		else if(num_players == 2){
			num_players = 0;
			cpid = fork();
			if (cpid < 0) {
     			fprintf(stderr, "Error: Fork failed\n");
     			return -1;
   			}
   			/* We are the child, run the connect four game */
   			if (cpid == 0){
   				close(sd);
   				for(i = 0; i <= 41; i++){
   					game_state[i] = '0';
      			}	
   				connect_four_game(sd2, sd3, buf, game_state, player1_turn,game_type);
   				return 0;
   			}
		}
	}
}
