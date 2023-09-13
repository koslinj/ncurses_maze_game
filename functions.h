struct board{
	char pattern[29][56];
};

struct miniboard{
	char pattern[5][5];
};

struct death_info{
	int x;
	int y;
	int num;
};

struct player{
	int spawnx;
	int spawny;
	int x;
	int y;
	char id;
	int carrying;
	int brought;
	int exited;
	int campsite;
	int campx;
	int campy;
	int player_deaths_counter;
	int type; //1 -> HUMAN
	int pid;
};

int init_deaths();

int read_file(char*, struct board*);

int player_init(struct player*, char id);

int movement(struct board*, struct player*, int, pthread_mutex_t*, sem_t*, int*, unsigned long long*, pid_t, struct player tab[4], int index);
int player_movement(struct board*, struct player*, int, pid_t);
int beast_move(pthread_mutex_t*, sem_t*);

int draw_map(struct board*);
int draw_player_map(struct miniboard*, struct player*);

int put_money();
int put_coin();
int put_t();
int put_T();
int left_in_campsite(struct player*);
int check_coin_type(char, struct player*);

int draw_player_info(struct player*, pid_t, unsigned long long);
int draw_server_info(struct player*, pid_t, unsigned long long, struct player tab[4]);

int player_death(struct player* p);
int check_death(struct player* p);
