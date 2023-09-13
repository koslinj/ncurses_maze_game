#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "functions.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdatomic.h> 

struct death_info deaths[100];
int death_counter;

int init_deaths(){
	for(int i=0;i<100;i++){
		deaths[i].x=-1;
		deaths[i].y=-1;
		deaths[i].num=0;
	}
	death_counter=0;
	return 0;
}

_Atomic int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

int read_file(char* filename, struct board* b){
	FILE* f;
    f=fopen(filename,"r");
    
    if(f==NULL) return 2;
    char temp;
	for(int i=0;i<29;i++){
		for(int j=0;j<56;j++){
			temp=getc(f);
			b->pattern[i][j]=temp;
		}
	}
	fclose(f);
	return 0;
}

int player_init(struct player* p, char id){
	int x = rand()%(54-2) + 4;
	int y = rand()%(28-2) + 4;
	char znak;
	while((znak=mvinch(y,x))=='a' || znak=='b' || (znak>='1' && znak<='4')){
		x++;
		if(x>=56) x=2;
	}
	p->spawnx = x;
	p->spawny = y;
	p->x=x;
    p->y=y;
    p->id=id;
    p->carrying=0;
    p->brought=0;
    p->exited=0;
    p->campsite=0;
    p->campx=28;
    p->campy=15;
    p->player_deaths_counter=0;
    p->type=1;
    p->pid=0;
	return 0;
}

int draw_map(struct board* b){
	int height;
	int width;
	getmaxyx(stdscr,height,width);
	if(height<36 || width<60) return -1;
	attron(A_INVIS);
	for(int i=0;i<29;i++){
		move(2+i,2);
		for(int j=0;j<56;j++){
			if(b->pattern[i][j] == 'a'){
				attron(COLOR_PAIR(3));
			}
			else if(b->pattern[i][j] == 'b'){
				
				attron(COLOR_PAIR(2));
			}
			else if(b->pattern[i][j] == 'A'){
				
				attron(COLOR_PAIR(6));
				attroff(A_INVIS);
			}
			
			printw("%c", b->pattern[i][j]);
			attroff(COLOR_PAIR(2));
			attroff(COLOR_PAIR(3));
			attroff(COLOR_PAIR(6));
			attron(A_INVIS);
		}
	}
        attroff( COLOR_PAIR( 1 ) );
        attroff(A_INVIS);
        attroff(A_REVERSE);
        return 0;
}

int draw_player_map(struct miniboard* b, struct player* p){
	clear();
	int height;
	int width;
	attron(A_INVIS);
	getmaxyx(stdscr,height,width);
	if(height<36 || width<60) return -1;
	for(int i=0;i<5;i++){
		move(p->y-2+i,p->x-2);
		for(int j=0;j<5;j++){
			if(b->pattern[i][j] == 'a'){
				attron(COLOR_PAIR(3));
			}
			else if(b->pattern[i][j] == 'b'){
				
				attron(COLOR_PAIR(2));
			}
			else if(b->pattern[i][j] >= '1' && b->pattern[i][j] <= '4'){
				
				attron(COLOR_PAIR(4));
				attroff(A_INVIS);
			}
			else if(b->pattern[i][j]=='C' || b->pattern[i][j]=='t' || b->pattern[i][j]=='T'){
				
				attron(COLOR_PAIR(5));
				attroff(A_INVIS);
			}
			else if(b->pattern[i][j]=='A'){
				
				attron(COLOR_PAIR(6));
				attroff(A_INVIS);
			}
			else if(b->pattern[i][j]=='*'){
				
				attron(COLOR_PAIR(7));
				attroff(A_INVIS);
			}
			else if(b->pattern[i][j]=='D'){
				
				attron(COLOR_PAIR(8));
				attroff(A_INVIS);
			}
			
			printw("%c", b->pattern[i][j]);
			attroff(COLOR_PAIR(2));
			attroff(COLOR_PAIR(3));
			attroff(COLOR_PAIR(4));
			attroff(COLOR_PAIR(5));
			attroff(COLOR_PAIR(6));
			attroff(COLOR_PAIR(8));
			attron(A_INVIS);
		}
	}
        attroff( COLOR_PAIR( 1 ) );
        attroff(A_INVIS);
        attroff(A_REVERSE);
        return 0;
}

int movement(struct board* b, struct player* p, int client1, pthread_mutex_t* mutex, sem_t* sem1, int* close, unsigned long long* rounds, pid_t serv_pid, struct player tab[4], int index){
	
	struct player pom;
	struct miniboard mb;
	int xdiff;
	int ydiff;
	unsigned long long counter=0;
    int flag=0;
 
    char act='\0';
    for(int i=0;i<5;i++){
			for(int j=0;j<5;j++){
				mb.pattern[i][j]=mvinch(p->y-2+i, p->x-2+j);
			}
		}
		send(client1, &mb, sizeof(mb), 0);
    pthread_mutex_lock(mutex);
    attron(COLOR_PAIR(4));
	mvaddch(p->y,p->x,p->id);
	attroff(COLOR_PAIR(4));
	refresh();
	pthread_mutex_unlock(mutex);
	while(1)
	{
		flag=0;
		
		read(client1,&pom,sizeof(pom));
		xdiff=p->x-pom.x;
		ydiff=p->y-pom.y;
		p->x=pom.x;
		p->y=pom.y;
		p->carrying=pom.carrying;
		p->brought=pom.brought;
		sem_wait(sem1);
		pthread_mutex_lock(mutex);
		if(ydiff>0){
			//next=mvinch(p->y-1,p->x);
				if(1){
					if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(p->y+1,p->x,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
					}
					else if(act == 'A'){
						left_in_campsite(p);
						attron(COLOR_PAIR(6));
						mvaddch(p->y+1,p->x,'A');
						attroff(COLOR_PAIR(6));
					}
					else if(act == 'D'){
						p->y+=1;
						check_death(p);
						p->y-=1;
						mvaddch(p->y+1,p->x,' ');
					}
					else{
						if(act=='C' || act=='t' || act=='T') check_coin_type(act,p);
						mvaddch(p->y+1,p->x,' ');
						refresh();
					}
				}
		}
		if(ydiff<0){
			//next=mvinch(p->y+1,p->x);
				if(1){
					if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(p->y-1,p->x,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
					}
					else if(act == 'A'){
						left_in_campsite(p);
						attron(COLOR_PAIR(6));
						mvaddch(p->y-1,p->x,'A');
						attroff(COLOR_PAIR(6));
					}
					else if(act == 'D'){
						p->y-=1;
						check_death(p);
						p->y+=1;
						mvaddch(p->y-1,p->x,' ');
					}
					else{
						if(act=='C' || act=='t' || act=='T') check_coin_type(act,p);
						mvaddch(p->y-1,p->x,' ');
						refresh();
					}
				}
		}
		if(xdiff>0){
			//next=mvinch(p->y,p->x-1);
				if(1){
					if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(p->y,p->x+1,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
					}
					else if(act == 'A'){
						left_in_campsite(p);
						attron(COLOR_PAIR(6));
						mvaddch(p->y,p->x+1,'A');
						attroff(COLOR_PAIR(6));
					}
					else if(act == 'D'){
						p->x+=1;
						check_death(p);
						p->x-=1;
						mvaddch(p->y,p->x+1,' ');
					}
					else{
						if(act=='C' || act=='t' || act=='T') check_coin_type(act,p);
						mvaddch(p->y,p->x+1,' ');
						refresh();
					}
				}
		}
		if(xdiff<0){
			//next=mvinch(p->y,p->x+1);
				if(1){
					if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(p->y,p->x-1,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
					}
					else if(act == 'A'){
						left_in_campsite(p);
						attron(COLOR_PAIR(6));
						mvaddch(p->y,p->x-1,'A');
						attroff(COLOR_PAIR(6));
					}
					else if(act == 'D'){
						p->x-=1;
						check_death(p);
						p->x+=1;
						mvaddch(p->y,p->x-1,' ');
					}
					else{
						if(act=='C' || act=='t' || act=='T') check_coin_type(act,p);
						mvaddch(p->y,p->x-1,' ');
						refresh();
					}
				}
		}
		refresh();

		if(xdiff==0 && ydiff==0) flag=1;
		if(pom.exited==1){

			mvaddch(p->y,p->x,' ');
			refresh();
			pom.type=0;
			pom.x=-1;
			pom.y=-1;
			tab[index] = pom;
			pthread_mutex_unlock(mutex);
			break;
		}
		if(flag==0) act=mvinch(p->y,p->x);
		char beast='\0';
		for(int k=0;k<1;k++){
			for(int l=0;l<1;l++){
				beast=mvinch(p->y+k,p->x+l);
				if(beast=='*'){

					player_death(p);

				}
			}
		}
		beast=mvinch(p->y,p->x);
		if((beast>='1' && beast<='4' && beast!=p->id)){

			player_death(p);

		}

		int flag_to_counter=0;
		for(int i=0;i<5;i++){
			for(int j=0;j<5;j++){
				if(mb.pattern[i][j]=='D') flag_to_counter=1;
			}
		}
		if(flag_to_counter==1) counter++;
		if(flag_to_counter==0) counter=0;
		if(beast=='D' && counter<1){

			player_death(p);

		}
			
		pom = *p;
		if(*close==-1){
			pom.x=-1;
			pom.y=-1;
		}
		send(client1, &pom, sizeof(pom), 0);

		attron(COLOR_PAIR(4));
		mvaddch(p->y,p->x,p->id);
		attroff(COLOR_PAIR(4));
		refresh();

		
		char is_campsite;
		for(int i=0;i<5;i++){
			for(int j=0;j<5;j++){
				is_campsite=mvinch(p->y-2+i, p->x-2+j);
				mb.pattern[i][j]=is_campsite;
				if(is_campsite=='A') p->campsite=1;
			}
		}
		send(client1, &mb, sizeof(mb), 0);
		
		send(client1, rounds, sizeof(*rounds), 0);
		tab[index] = pom;

		draw_server_info(p,serv_pid,*rounds,tab);
		pthread_mutex_unlock(mutex);
		}
		return 0;
}


int player_movement(struct board* b, struct player* p, int client, pid_t pid){
	signal(SIGINT, intHandler);
	int c;
    int exit=0;
    int bush=0;
    char next;
    struct miniboard mb;
    read(client,&mb,sizeof(mb));
    draw_player_map(&mb, p);
	attron(COLOR_PAIR(4));
	nodelay(stdscr,1);
	mvprintw(p->y,p->x,"%c",p->id);
	attroff(COLOR_PAIR(4));
	while(keepRunning)
	{
		c = getch();
		flushinp();
		switch(c)
		{	case KEY_UP:
				next=mvinch(p->y-1,p->x);
				if(next != 'a'){
					if(bush==1){
						bush=0;
					}
					else{
						if(next == 'b') bush=1;
					}
					if(bush==0) p->y--;
				}
				break;
			case KEY_DOWN:
				next=mvinch(p->y+1,p->x);
				if(next != 'a'){
					if(bush==1){
						bush=0;
					}
					else{
						if(next == 'b') bush=1;
					}
					if(bush==0) p->y++;
				}
				break;
			case KEY_LEFT:
				next=mvinch(p->y,p->x-1);
				if(next != 'a'){
					if(bush==1){
						bush=0;
					}
					else{
						if(next == 'b') bush=1;
					}
					if(bush==0) p->x--;
				}
				break;
			case KEY_RIGHT:
				next=mvinch(p->y,p->x+1);
				if(next != 'a'){
					if(bush==1){
						bush=0;
					}
					else{
						if(next == 'b') bush=1;
					}
					if(bush==0) p->x++;
				}
				break;
			case 'q':
				exit=1;
				break;
			
		}
		if(exit==1) break;

		send(client, p, sizeof(*p), 0);
		read(client,p,sizeof(*p));
		read(client,&mb,sizeof(mb));
		draw_player_map(&mb,p);
		
		if(p->x==-1 && p->y==-1)
		{
			break;
		}
		unsigned long long round_number;
		read(client,&round_number,sizeof(round_number));
		draw_player_info(p,pid,round_number);
		
		refresh();
		
	}
	p->exited=1;
	send(client, p, sizeof(*p), 0);
		return 0;
}


int put_money(){
	int x = rand()%(44-2) + 4;
	int y = rand()%(28-2) + 4;
	int type = rand()%16;
	char znak;
	while((znak=mvinch(y,x))=='a' || znak=='b' || (znak>='1' && znak<='4')){
		x++;
		if(x>=56) x=2;
	}
	attron(COLOR_PAIR(5));
	if(type<=9) mvaddch(y,x,'C');
	else if(type<=13) mvaddch(y,x,'t');
	else mvaddch(y,x,'T');
	attroff(COLOR_PAIR(5));
	return 0;
}
int put_coin(){
	int x = rand()%(44-2) + 4;
	int y = rand()%(28-2) + 4;
	char znak;
	while((znak=mvinch(y,x))=='a' || znak=='b' || (znak>='1' && znak<='4')){
		x++;
		if(x>=56) x=2;
	}
	attron(COLOR_PAIR(5));
	mvaddch(y,x,'C');
	attroff(COLOR_PAIR(5));
	return 0;
}
int put_t(){
	int x = rand()%(44-2) + 4;
	int y = rand()%(28-2) + 4;
	char znak;
	while((znak=mvinch(y,x))=='a' || znak=='b' || (znak>='1' && znak<='4')){
		x++;
		if(x>=56) x=2;
	}
	attron(COLOR_PAIR(5));
	mvaddch(y,x,'t');
	attroff(COLOR_PAIR(5));
	return 0;
}
int put_T(){
	int x = rand()%(44-2) + 4;
	int y = rand()%(28-2) + 4;
	char znak;
	while((znak=mvinch(y,x))=='a' || znak=='b' || (znak>='1' && znak<='4')){
		x++;
		if(x>=56) x=2;
	}
	attron(COLOR_PAIR(5));
	mvaddch(y,x,'T');
	attroff(COLOR_PAIR(5));
	return 0;
}

int check_coin_type(char a, struct player* p){
	if(a=='C') p->carrying+=1;
	else if(a=='t') p->carrying+=10;
	else if(a=='T') p->carrying+=50;
	return 0;
}

int left_in_campsite(struct player* p){
	p->brought+=p->carrying;
	p->carrying=0;
	return 0;
}

int draw_player_info(struct player* p, pid_t pid, unsigned long long round_number){
	mvprintw(2,58,"SERVER'S PID: %d",pid);
	if(p->campsite==0) mvprintw(3,58,"Campsite X/Y: unknown");
	else mvprintw(3,58,"Campsite X/Y: %d/%d",p->campx,p->campy);
	mvprintw(4,58,"Round Number: %llu",round_number);
	mvprintw(7,58,"Player:");
	mvprintw(8,59,"Number: %c",p->id);
	mvprintw(9,59,"Type: HUMAN");
	mvprintw(10,59,"Curr X/Y: %d/%d",p->x, p->y);
	mvprintw(11,59,"Deaths: %d",p->player_deaths_counter);
	mvprintw(23,58,"Coins carrying");
	mvprintw(23,73,"%d",p->carrying);
	mvprintw(25,58,"Coins brought");
	mvprintw(25,73,"%d",p->brought);
	mvprintw(29,58,"Legend:");
	
	attron(COLOR_PAIR(4));
	mvprintw(30,59,"1234");
	attroff(COLOR_PAIR(4));
	mvprintw(30,64,"- Players");
	
	attron(A_INVIS);
	attron(COLOR_PAIR(3));
	mvprintw(31,59,"a");
	attroff(A_INVIS);
	attroff(COLOR_PAIR(3));
	mvprintw(31,61,"- Wall");
	attron(A_INVIS);
	attron(COLOR_PAIR(2));
	mvprintw(32,59,"b");
	attroff(COLOR_PAIR(2));
	attroff(A_INVIS);
	mvprintw(32,61,"- Bushes (slow down)");
	attron(COLOR_PAIR(7));
	mvprintw(33,59,"*");
	attroff(COLOR_PAIR(7));
	mvprintw(33,61,"- enemy");
	attron(COLOR_PAIR(5));
	mvprintw(34,59,"c");
	attroff(COLOR_PAIR(5));
	mvprintw(34,61,"- One coin");
	attron(COLOR_PAIR(5));
	mvprintw(35,59,"t");
	attroff(COLOR_PAIR(5));
	mvprintw(35,61,"- Treasure (10 coins)");
	attron(COLOR_PAIR(5));
	mvprintw(36,59,"T");
	attroff(COLOR_PAIR(5));
	mvprintw(36,61,"- Large treasure (50 coins)");
	attron(COLOR_PAIR(6));
	mvprintw(37,59,"A");
	attroff(COLOR_PAIR(6));
	mvprintw(37,61,"- Campsite");
	refresh();
	
	return 0;
}

int draw_server_info(struct player* p, pid_t serv_pid, unsigned long long round_number, struct player tab[4]){
	
	for(int i=0;i<40;i++)
	{
		move(i,58);
		clrtoeol();
	}
	
	mvprintw(2,58,"SERVER'S PID: %d",serv_pid);
	mvprintw(3,58,"Campsite X/Y: 28/15");
	mvprintw(4,58,"Round Number: %llu",round_number);
	mvprintw(7,58,"Parameter:  Player1  Player2  Player3  Player4");
	mvprintw(9,59,"PID");
	if(tab[0].type==1) mvprintw(9,70,"%d",tab[0].pid);
	else mvprintw(9,70,"-----");
	if(tab[1].type==1) mvprintw(9,79,"%d",tab[1].pid);
	else mvprintw(9,79,"-----");
	if(tab[2].type==1) mvprintw(9,88,"%d",tab[2].pid);
	else mvprintw(9,88,"-----");
	if(tab[3].type==1) mvprintw(9,97,"%d",tab[3].pid);
	else mvprintw(9,97,"-----");
	
	mvprintw(11,59,"Type");
	if(tab[0].type==1) mvprintw(11,70,"HUMAN");
	else mvprintw(11,70,"-----");
	if(tab[1].type==1) mvprintw(11,79,"HUMAN");
	else mvprintw(11,79,"-----");
	if(tab[2].type==1) mvprintw(11,88,"HUMAN");
	else mvprintw(11,88,"-----");
	if(tab[3].type==1) mvprintw(11,97,"HUMAN");
	else mvprintw(11,97,"-----");
	
	mvprintw(13,59,"Curr X/Y");
	if(tab[0].x<=0 && tab[0].y<=0) mvprintw(13,70,"--/--");
	else mvprintw(13,70,"%d/%d",tab[0].x,tab[0].y);
	if(tab[1].x<=0 && tab[1].y<=0) mvprintw(13,79,"--/--");
	else mvprintw(13,79,"%d/%d",tab[1].x,tab[1].y);
	if(tab[2].x<=0 && tab[2].y<=0) mvprintw(13,88,"--/--");
	else mvprintw(13,88,"%d/%d",tab[2].x,tab[2].y);
	if(tab[3].x<=0 && tab[3].y<=0) mvprintw(13,97,"--/--");
	else mvprintw(13,97,"%d/%d",tab[3].x,tab[3].y);
	
	mvprintw(15,59,"Deaths");
	if(tab[0].type==1) mvprintw(15,72,"%d",tab[0].player_deaths_counter);
	else mvprintw(15,70,"-----");
	if(tab[1].type==1) mvprintw(15,81,"%d",tab[1].player_deaths_counter);
	else mvprintw(15,79,"-----");
	if(tab[2].type==1) mvprintw(15,90,"%d",tab[2].player_deaths_counter);
	else mvprintw(15,88,"-----");
	if(tab[3].type==1) mvprintw(15,99,"%d",tab[3].player_deaths_counter);
	else mvprintw(15,97,"-----");
	
	mvprintw(19,58,"Coins");
	mvprintw(21,59,"Carrying");
	if(tab[0].type==1) mvprintw(21,72,"%d",tab[0].carrying);
	else mvprintw(21,70,"-----");
	if(tab[1].type==1) mvprintw(21,81,"%d",tab[1].carrying);
	else mvprintw(21,79,"-----");
	if(tab[2].type==1) mvprintw(21,90,"%d",tab[2].carrying);
	else mvprintw(21,88,"-----");
	if(tab[3].type==1) mvprintw(21,99,"%d",tab[3].carrying);
	else mvprintw(21,97,"-----");
	
	mvprintw(23,59,"Brought");
	if(tab[0].type==1) mvprintw(23,72,"%d",tab[0].brought);
	else mvprintw(23,70,"-----");
	if(tab[1].type==1) mvprintw(23,81,"%d",tab[1].brought);
	else mvprintw(23,79,"-----");
	if(tab[2].type==1) mvprintw(23,90,"%d",tab[2].brought);
	else mvprintw(23,88,"-----");
	if(tab[3].type==1) mvprintw(23,99,"%d",tab[3].brought);
	else mvprintw(23,97,"-----");
	
	mvprintw(29,58,"Legend:");
	attron(COLOR_PAIR(4));
	mvprintw(30,59,"1234");
	attroff(COLOR_PAIR(4));
	mvprintw(30,64,"- Players");
	attron(A_INVIS);
	attron(COLOR_PAIR(3));
	mvprintw(31,59,"a");
	attroff(A_INVIS);
	attroff(COLOR_PAIR(3));
	mvprintw(31,61,"- Wall");
	attron(A_INVIS);
	attron(COLOR_PAIR(2));
	mvprintw(32,59,"b");
	attroff(COLOR_PAIR(2));
	attroff(A_INVIS);
	mvprintw(32,61,"- Bushes (slow down)");
	attron(COLOR_PAIR(7));
	mvprintw(33,59,"*");
	attroff(COLOR_PAIR(7));
	mvprintw(33,61,"- enemy");
	attron(COLOR_PAIR(5));
	mvprintw(34,59,"c");
	attroff(COLOR_PAIR(5));
	mvprintw(34,61,"- One coin");
	attron(COLOR_PAIR(5));
	mvprintw(35,59,"t");
	attroff(COLOR_PAIR(5));
	mvprintw(35,61,"- Treasure (10 coins)");
	attron(COLOR_PAIR(5));
	mvprintw(36,59,"T");
	attroff(COLOR_PAIR(5));
	mvprintw(36,61,"- Large treasure (50 coins)");
	attron(COLOR_PAIR(6));
	mvprintw(37,59,"A");
	attroff(COLOR_PAIR(6));
	mvprintw(37,61,"- Campsite");
	refresh();
	
	return 0;
}


int player_death(struct player* p){
	attron(COLOR_PAIR(8));
	mvaddch(p->y,p->x,'D');
	attroff(COLOR_PAIR(8));
	refresh();
	deaths[death_counter].x=p->x;
	deaths[death_counter].y=p->y;
	deaths[death_counter].num+=p->carrying;
	death_counter++;
	
	p->x=p->spawnx;
    p->y=p->spawny;
    //p->id=id;
    p->carrying=0;
    //p->brought=0;
    p->exited=0;
    p->player_deaths_counter++;
    
	return 0;
}

int check_death(struct player* p){
	for(int i=0;i<death_counter;i++){
		if(p->x == deaths[i].x && p->y == deaths[i].y){
			p->carrying+=deaths[i].num;
		}
	}
	return 0;
}


int beast_move(pthread_mutex_t* mutex, sem_t* sem1){
	int x = rand()%(44-2) + 4;
	int y = rand()%(28-2) + 4;
	char znak;
	while((znak=mvinch(y,x))=='a' || znak=='b' || znak=='A' || (znak>='1' && znak<='4')){
		x++;
		if(x>=56) x=2;
	}
	int choice;
	char next;
	char act;
	int py;
	int px;
	char see;
	
	char check_dir;
	
	int flag=0;
	int moved=0;
	while(1){
		see='\0';
		sem_wait(sem1);
		pthread_mutex_lock(mutex);
		flag=0;
		moved=0;
		
		for(int i=0;i<5;i++){
			for(int j=0;j<5;j++){
			see=mvinch(y-2+i,x-2+j);
			if(see>='1' && see<='4'){
				
				py=y-2+i;
				px=x-2+j;
				flag=1;
				
				if(py<y){

				check_dir=mvinch(y-1,x);
				if(check_dir=='a'){
					flag=0;
				}
				if(px!=x){
				check_dir=mvinch(y-1,x-1);
				if(check_dir=='a'){
					flag=0;
				}
				check_dir=mvinch(y-1,x+1);
				if(check_dir=='a'){
					flag=0;
				}
			}

					
					
					next=mvinch(y-1,x);
					if(next != 'a' && moved==0 && flag==1){
						if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(y,x,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
						}
						else if(act == 'A'){
						attron(COLOR_PAIR(6));
						mvaddch(y,x,'A');
						attroff(COLOR_PAIR(6));
						}
						else if(act=='D'){
						attron(COLOR_PAIR(8));
						mvaddch(y,x,'D');
						attroff(COLOR_PAIR(8));
						}
						else mvprintw(y,x," ");
						y--;
						moved=1;
					}
				}
				
				flag=1;
				if(py>y){
					check_dir=mvinch(y+1,x);
				if(check_dir=='a'){
					flag=0;
				}
				if(px!=x){
				check_dir=mvinch(y+1,x-1);
				if(check_dir=='a'){
					flag=0;
				}
				check_dir=mvinch(y+1,x+1);
				if(check_dir=='a'){
					flag=0;
				}
			}
					
					next=mvinch(y+1,x);
					if(next != 'a' && moved==0 && flag==1){
						if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(y,x,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
						}
						else if(act == 'A'){
						attron(COLOR_PAIR(6));
						mvaddch(y,x,'A');
						attroff(COLOR_PAIR(6));
						}
						else if(act=='D'){
						attron(COLOR_PAIR(8));
						mvaddch(y,x,'D');
						attroff(COLOR_PAIR(8));
						}
						else mvprintw(y,x," ");
						y++;
						moved=1;
					}
				}
				
				flag=1;
				if(px<x){
					check_dir=mvinch(y,x-1);
				if(check_dir=='a'){
					flag=0;
				}
				if(py!=y){
				check_dir=mvinch(y-1,x-1);
				if(check_dir=='a'){
					flag=0;
				}
				check_dir=mvinch(y+1,x-1);
				if(check_dir=='a'){
					flag=0;
				}
			}
					
					next=mvinch(y,x-1);
					if(next != 'a' && moved==0 && flag==1){
						if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(y,x,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
						}
						else if(act == 'A'){
						attron(COLOR_PAIR(6));
						mvaddch(y,x,'A');
						attroff(COLOR_PAIR(6));
						}
						else if(act=='D'){
						attron(COLOR_PAIR(8));
						mvaddch(y,x,'D');
						attroff(COLOR_PAIR(8));
						}
						else mvprintw(y,x," ");
						x--;
						moved=1;
					}
				}
				
				flag=1;
				if(px>x){
					check_dir=mvinch(y,x+1);
				if(check_dir=='a'){
					flag=0;
				}
				if(py!=y){
				check_dir=mvinch(y-1,x+1);
				if(check_dir=='a'){
					flag=0;
				}
				check_dir=mvinch(y+1,x+1);
				if(check_dir=='a'){
					flag=0;
				}
			}
					
					next=mvinch(y,x+1);
					if(next != 'a' && moved==0 && flag==1){
						if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(y,x,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
						}
						else if(act == 'A'){
						attron(COLOR_PAIR(6));
						mvaddch(y,x,'A');
						attroff(COLOR_PAIR(6));
						}
						else if(act=='D'){
						attron(COLOR_PAIR(8));
						mvaddch(y,x,'D');
						attroff(COLOR_PAIR(8));
						}
						else mvprintw(y,x," ");
						x++;
						moved=1;
					}
				}
			}
		}
	}
		
		if(moved==0){
		
		choice = rand()%4;
		switch(choice)
		{	case 0:
				next=mvinch(y-1,x);
				if(next != 'a'){
					if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(y,x,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
					}
					else if(act == 'A'){
						attron(COLOR_PAIR(6));
						mvaddch(y,x,'A');
						attroff(COLOR_PAIR(6));
					}
					else if(act=='D'){
						attron(COLOR_PAIR(8));
						mvaddch(y,x,'D');
						attroff(COLOR_PAIR(8));
						}
					else mvprintw(y,x," ");
					y--;
					moved=1;
				}
				break;
			case 1:
				next=mvinch(y+1,x);
				if(next != 'a'){
					if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(y,x,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
					}
					else if(act == 'A'){
						attron(COLOR_PAIR(6));
						mvaddch(y,x,'A');
						attroff(COLOR_PAIR(6));
					}
					else if(act=='D'){
						attron(COLOR_PAIR(8));
						mvaddch(y,x,'D');
						attroff(COLOR_PAIR(8));
						}
					else mvprintw(y,x," ");
					y++;
					moved=1;
				}
				break;
			case 2:
				next=mvinch(y,x-1);
				if(next != 'a'){
					if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(y,x,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
					}
					else if(act == 'A'){
						attron(COLOR_PAIR(6));
						mvaddch(y,x,'A');
						attroff(COLOR_PAIR(6));
					}
					else if(act=='D'){
						attron(COLOR_PAIR(8));
						mvaddch(y,x,'D');
						attroff(COLOR_PAIR(8));
						}
					else mvprintw(y,x," ");
					x--;
					moved=1;
				}
				break;
			case 3:
				next=mvinch(y,x+1);
				if(next != 'a'){
					if(act == 'b'){
						attron(COLOR_PAIR(2));
						attron(A_INVIS);
						mvaddch(y,x,'b');
						attroff(COLOR_PAIR(2));
						attroff(A_INVIS);
					}
					else if(act == 'A'){
						attron(COLOR_PAIR(6));
						mvaddch(y,x,'A');
						attroff(COLOR_PAIR(6));
					}
					else if(act=='D'){
						attron(COLOR_PAIR(8));
						mvaddch(y,x,'D');
						attroff(COLOR_PAIR(8));
						}
					else mvprintw(y,x," ");
					x++;
					moved=1;
				}
				break;
			
		}
	}
		if(moved==1) act=mvinch(y,x);
		attron(COLOR_PAIR(7));
		char is_player=mvinch(y,x);
		if(is_player>='1' && is_player<='4'){
			act='D';
		}
		mvprintw(y,x,"*");
		attroff(COLOR_PAIR(7));
		refresh();
		pthread_mutex_unlock(mutex);

		
	}
}

