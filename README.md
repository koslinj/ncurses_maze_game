# ncurses Maze Game
2D Linux console game for up to 4 players

## Gameplay

![gra1](https://github.com/koslinj/ncurses_maze_game/assets/97230028/4bb0e72b-19b3-4a7c-83d4-42bec92d6d41)
![gra2](https://github.com/koslinj/ncurses_maze_game/assets/97230028/f1ef7413-ebb1-4dd0-a559-258c00a662b1)
![gra3](https://github.com/koslinj/ncurses_maze_game/assets/97230028/10301aba-5c77-46b6-95b3-032e4175868f)

Every player spawns in random place on map. The only goal is to get as many coins as possible and bring them back to the camp, where they can be stored. From server, we can also spawn beasts which moves randomly. They will follow the player as soon as they will be in theirs field of view. If a beast catch player, he dies, losing coins that he was carrying and then respawns in random position. On the place of death, player leaves an amount of coins that he was carrying.

## Tech stack

- ncurses - UNIX console graphics library for C
- UNIX Sockets
- UNIX Multithreading - mutex and semaphore

## Build

To build this project:
```bash
  make all
```

## Run

To run server:

```bash
  ./server
```

To run game:

```bash
  ./client
```
## Keybindings

### Server

- `b` - spawn a beast (init a beast thread)
- `q` - close the server
- `c` - spawn a coin
- `t` - spawn a treasure
- `T` - spawn a large treasure

### Client

- `q` - quit the game
- &uarr; - move up
- &darr; - move down
- &larr; - move left
- &rarr; - move right
