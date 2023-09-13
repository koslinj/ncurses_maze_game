#define _GNU_SOURCE 

#include <arpa/inet.h>

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <stdatomic.h> 

#include <ncurses.h>
#include "functions.h"

struct thread_data{
	int client;
	char id;
};

unsigned long long rounds=0;
struct player player_tab[4];
_Atomic int player_index=0;

sem_t sem1;
sem_t sem2;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid;
pthread_t beast_tid[4];
_Atomic int beast_index=0;
pthread_t time_td;
pthread_t communication_threads[10];
int go=0;
int server_close=0;
int number_of_players=0;
int is_place=0;

_Atomic int is_there_beast = 0;


void* beast_thread(void* arg){
	beast_move(&mutex, &sem1);
	return NULL;
}


void* timefun(void* arg){
	while(1){
		usleep(800000);
		rounds++;
		for(int i=0;i<beast_index;i++){
			sem_post(&sem1);
		}
		for(int i=0;i<player_index+1;i++){
			sem_post(&sem2);
		}
		
		
	}
	return NULL;
}



void* communication_fun(void* param)
{
	pid_t server_pid = getpid();
	srand(time(NULL));

	struct thread_data t_data = *((struct thread_data*)param);
	int client1 = t_data.client;
	
	struct board b;
	struct player p;
	read_file("map4.txt",&b);
    
	refresh();
	player_init(&p,t_data.id);
	for(int i=0;i<20;i++) put_money();
	pid_t client_pid;
	read(client1, &client_pid, sizeof(client_pid));
	p.pid=client_pid;
	send(client1, &server_pid, sizeof(server_pid), 0);
	send(client1, &p, sizeof(p), 0);
	
	player_index=p.id-'1';
	player_tab[player_index] = p;

	movement(&b, &p, client1, &mutex, &sem2, &server_close, &rounds, server_pid, player_tab, player_index);
	
	
	close(client1);

	pthread_exit(NULL);
}
void* getter(void* arg){
	char ch;
	timeout(200);
	while(1){
		ch='\0';
		ch=getchar();
		if((ch=='b' || ch=='B') && beast_index<4){
			pthread_create(&beast_tid[beast_index], NULL, beast_thread, NULL);
			beast_index++;
		}
		else if(ch=='c'){
			pthread_mutex_lock(&mutex);
			put_coin();
			pthread_mutex_unlock(&mutex);
		}
		else if(ch=='t'){
			pthread_mutex_lock(&mutex);
			put_t();
			pthread_mutex_unlock(&mutex);
		}
		else if(ch=='T'){
			pthread_mutex_lock(&mutex);
			put_T();
			pthread_mutex_unlock(&mutex);
		}
		else if(ch=='q' || ch=='Q'){
			break;
		}
	}
	int* res = (int*)arg;
	*res=-1;
	
	return res;
}


int main()
{
	sem_init(&sem1, 0, 0);
	sem_init(&sem2, 0, 0);

	init_deaths();

	int serverSocket, newSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;

	socklen_t addr_size;
	
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    
    
    struct timeval timeout;      
    timeout.tv_sec = 2;
    timeout.tv_usec = 400000;
    
    setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    //setsockopt(serverSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);
    
    

	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(9010);


	bind(serverSocket,
		(struct sockaddr*)&serverAddr,
		sizeof(serverAddr));


	if (listen(serverSocket, 10) == 0)
		printf("Succesful\n");
	else
		printf("Error\n");



	initscr();
    noecho();
    curs_set(0);
	cbreak();
	
	struct board b;
	read_file("map4.txt",&b);
    
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
    draw_map(&b);
	refresh();

	pthread_create(&tid,NULL,getter,&server_close);
	pthread_create(&time_td,NULL,timefun,NULL);

	int i = 0;
	char player_number='1';
	struct thread_data t_data;
	int is_server_full=0;

	while (1) {
		is_server_full=0;
		is_place=0;
		addr_size = sizeof(serverStorage);
		
		newSocket = accept(serverSocket,
						(struct sockaddr*)&serverStorage,
						&addr_size);
		if(newSocket!=-1){
			
		for(int l=0;l<4;l++){
			if(player_tab[l].type==0){
				is_place=l+1;
				break;
			}
		}
		player_number='0'+is_place;
		t_data.id=player_number;

		t_data.client=newSocket;
		
		if(is_place==0){
			is_server_full=1;
		}
		send(newSocket, &is_server_full, sizeof(is_server_full), 0);
		if(is_place!=0){
			if (pthread_create(&communication_threads[i++], NULL, communication_fun, &t_data)!= 0) printf("Failed to create thread\n");
			for(int k=0;k<i;k++){
				pthread_tryjoin_np(communication_threads[k], NULL);
			}
		}
		if(is_place==0){
			close(newSocket);
		}
		}
		
		
		if(server_close==-1){
			for(int k=0;k<i;k++){
				pthread_join(communication_threads[k], NULL);
			}
			pthread_tryjoin_np(tid, NULL);
			pthread_tryjoin_np(time_td, NULL);
			pthread_mutex_lock(&mutex);

			clear();
			refresh();
			mvprintw(20,20,"SERVER CLOSED... CLOSING PLAYERS");
			refresh();
			close(serverSocket);
			endwin();
			pthread_mutex_unlock(&mutex);
			return 0;
		}
	}

	return 0;
}
