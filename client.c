#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <arpa/inet.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#include <ncurses.h>
#include "functions.h"


int main()
{

	int network_socket;
	network_socket = socket(AF_INET,
							SOCK_STREAM, 0);

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(9010);

	int connection = connect(network_socket, (struct sockaddr*)&server_address, sizeof(server_address));
	if (connection < 0) {
		printf("Thats an error\n");
		return 0;
	}
	
	int full=0;
	read(network_socket,&full,sizeof(full));
	if(full==1){
		close(network_socket);
		printf("SERVER IS FULL\n");
		return 0;
	}
	
	initscr();
    noecho();
    curs_set(0);
	cbreak();
	struct board b;
    struct player p;
    
    start_color();
    init_pair( 1, COLOR_BLACK, COLOR_WHITE );
    init_pair( 2, COLOR_BLACK, COLOR_GREEN );
    init_pair( 3, COLOR_WHITE, COLOR_BLACK );
    init_pair( 4, COLOR_WHITE, COLOR_MAGENTA );
    init_pair( 5, COLOR_BLACK, COLOR_YELLOW );
    init_pair( 6, COLOR_BLACK, COLOR_CYAN );
    init_pair( 7, COLOR_RED, COLOR_WHITE );
    init_pair( 8, COLOR_WHITE, COLOR_RED );
    wbkgd(stdscr, COLOR_PAIR(1));
    
    
    int res=read_file("map4.txt",&b);
    if(res!=0){
		mvprintw(3,3,"NO FILE");
		refresh();
		endwin();
		return 0;
    }
    
    keypad(stdscr, TRUE);
    
    pid_t server_pid;
    pid_t client_pid = getpid();
    send(network_socket, &client_pid, sizeof(client_pid), 0);
    
    read(network_socket,&server_pid,sizeof(server_pid));
    read(network_socket,&p,sizeof(p));
	player_movement(&b,&p,network_socket, server_pid);
    refresh();
    
    clear();
    mvprintw(12,12,"YOU EXITED GAME");
    refresh();
    endwin();
	
	
	close(network_socket);

	return 0;
}
