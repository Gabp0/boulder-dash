// Biblioteca "boulder-dash.h"
// Funcoes para execucao do jogo "boulder dash"
// Feito por Gabriel de Oliveira Pontarolo
#ifndef __BDDSH__
#define __BDDSH__

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>

#define FPS 60      // frames per second

// controles de execucao para maquina de estados
#define IN_GAME 0   
#define WIN 1
#define GAME_OVER 2
#define CLOSE 3
#define PLAY_AGAIN 4

#define DISP_SCALE 2    // escala do buffer

#define BUFFER_W 640    // largura do buffer
#define BUFFER_H 372    // altura do buffer

#define DISP_W (DISP_SCALE * BUFFER_W)  // largura real da tela
#define DISP_H (DISP_SCALE * BUFFER_H)  // altura real da tela

#define HUD_H 20    // altura da hud

#define MAP_W 40    // largura do mapa do jogo
#define MAP_H 22    // altura do mapa do jogo

#define SAMPLES_NUM 16  // quantidade maxima de samples a serem tocadas por vez

#define DIRT_FRAMES 8       // sprites de terra
#define OUT_WALL_FRAMES 2   // sprites das paredes externas
#define IN_WALL_FRAMES 4    // sprites das paredes internas
#define BOULDER_FRAMES 2    // sprites das boulder
#define JEWELS_FRAMES 4     // sprites das joias
#define MINER_FRAMES 2      // sprites do jogador

#define SPRITES_SIZE 16     // tamanho dos sprites 16x16

#define MAX_TIME 150    // tempo maximo de cada fase

#define KEY_SEEN     1  // mascara de bits para teclado
#define KEY_RELEASED 2  // mascara de bits para teclado

#define MINER_SPEED 1   // velocidade do jogador
#define ENTITY_SPEED 1  // velocidade das boulders e joias

// simbolos para matriz de posicoes
#define DIRT_C 46       // .
#define BOULDER_C 111   // o
#define JEWEL_C 42      // *
#define WALL_C 35       // #
#define EXIT_C 115      // s
#define PLAYER_C 64     // @
#define EMPTY_C 32      // espaco vazio

typedef struct COORDINATE
// struct auxiliar para coordenadas cartesianas
{
    int x, y;
} COORDINATE;

typedef struct BOULDER
// struct para as boulders
{
    COORDINATE p;
    bool can_fall;
    bool destroyed;
    ALLEGRO_BITMAP *sprite;

} BOULDER;

typedef struct DIRT
// struct para terra
{
    COORDINATE p;
    bool destroyed;
    ALLEGRO_BITMAP *sprite;

} DIRT;

typedef struct JEWEL
// struct para as joias
{
    COORDINATE p;
    bool collected;
    bool can_fall;
    ALLEGRO_BITMAP *sprite;

} JEWEL;

typedef struct OUT_WALL
// struct para as paredes externas
{
    COORDINATE p;
    ALLEGRO_BITMAP *sprite;

} OUT_WALL;

typedef struct IN_WALL
// struct para as paredes internas
{
    COORDINATE p;
    bool destroyed;
    ALLEGRO_BITMAP *sprite;

} IN_WALL;

typedef struct MINER
// struct do jogador
{
    COORDINATE p;
    int jew;
    int time_left;
    bool alive;
    bool invul;
    int direction;
    ALLEGRO_BITMAP *sprite_right;
    ALLEGRO_BITMAP *sprite_left;

} MINER;

typedef struct EXIT
// struct para saida
{
    COORDINATE p;
    bool open;
    ALLEGRO_BITMAP *sprite_doorclose;
    ALLEGRO_BITMAP *sprite_dooropen;

} EXIT;

typedef struct ENT_POS
// struct da matriz de posicoes
{
    char entity;
    int index;

} ENT_POS;

typedef struct BOARD
// struct contendo todas as entidades, quantidade e suas posicoes
{
    ENT_POS *map;
    BOULDER **boulders;
    int bould_num;
    DIRT **floor;
    int dirt_num;
    JEWEL **jewels;
    int jew_num;
    OUT_WALL **outer_walls;
    int out_wall_num;
    IN_WALL **inner_wall;
    int inn_wall_num;
    MINER *player;
    EXIT *exit_point;

} BOARD;

typedef struct HUD
// struct para a hud no topo
{
    ALLEGRO_FONT *font;
    ALLEGRO_COLOR score_color;
    ALLEGRO_COLOR timer_color;
    unsigned int score;
    int score_x;
    unsigned int time;
    int timer_x;
    unsigned int jew_num;
    int jew_x;

} HUD;

typedef struct SPRITES 
// struct para armazenas os bitmaps dos sprites
{
    ALLEGRO_BITMAP *dirt[DIRT_FRAMES];
    ALLEGRO_BITMAP *outer_walls[OUT_WALL_FRAMES];
    ALLEGRO_BITMAP *inner_walls[IN_WALL_FRAMES];
    ALLEGRO_BITMAP *boulders[BOULDER_FRAMES];
    ALLEGRO_BITMAP *jewels[JEWELS_FRAMES];
    ALLEGRO_BITMAP *miner[MINER_FRAMES];
    ALLEGRO_BITMAP *open_door;
    ALLEGRO_BITMAP *close_door;

}SPRITES;

typedef struct SCREEN
// struct para armazenas as fontes usadas na tela de score
{
    ALLEGRO_FONT *regular;
    ALLEGRO_FONT *title;
    ALLEGRO_FONT *credits;

} SCREEN;

typedef struct MENU
// struct para armazenar o menu de pausa
{
    ALLEGRO_BITMAP *info;
    bool show_menu;
} MENU;

typedef struct SAMPLES
// struct para armazenar os arquivos de audio
{
    ALLEGRO_SAMPLE *open_door;
    ALLEGRO_SAMPLE *miner_death;
    ALLEGRO_SAMPLE *pickup_jewel;
    ALLEGRO_SAMPLE *win_game;
    ALLEGRO_SAMPLE *power_up;
    ALLEGRO_AUDIO_STREAM *bakcground_music;

} SAMPLES;

typedef struct SCORE
// struct para armazenar um score
{
    char datetime[20];
    int time;
} SCORE;

typedef struct SCOREBOARD
// struct para armazenar a scoreboard
{
    int size;
    SCORE *scores;
} SCOREBOARD;

typedef enum CHEAT_CODE
// estados da maquina de estados que testa o codigo de cheat
{
    INVALID, UP1, UP2, DOWN1, DOWN2, LEFT1, RIGHT1, LEFT2, RIGHT2, A, B 
} CHEAT_CODE;

typedef struct GAME_ENV
// struct principal para armazenar todas as variaveis necessarias para o funcionamento do jogo
{
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *queue;
    ALLEGRO_EVENT event;
    ALLEGRO_BITMAP *buffer;
    ALLEGRO_DISPLAY *disp;
    SCOREBOARD *highscores;
    unsigned int frames;
    unsigned char key[ALLEGRO_KEY_MAX];
    SPRITES *game_sprites;
    SAMPLES *game_samples;
    BOARD *entities;
    HUD *top_hud;
    MENU *pause_menu;
    SCREEN *score_screen;
    CHEAT_CODE konami_code;
    bool redraw;


} GAME_ENV;

GAME_ENV* loadGame(void);               // Executado uma vez ao abrir o jogo, realiza a inicializacao das bibliotecas, leitura de arquivos e alocacao de memoria. Retorna um ponteiro para uma struct "GAME_ENV" com as variaveis necessarias para o funcionamento do jogo
void initGame(GAME_ENV *env);           // Define os valores iniciais das variaveis do jogo em _env_. Executado sempre que a fase é inciada/reiniciada
void drawBoard(GAME_ENV *env);          // Desenha a tela durante o jogo. Executada todo final de loop em cada frame.
int updateGameState(GAME_ENV *env);     // Atualiza o estado de todas as entidades do jogo, variaveis da struct _env_ e le os inputs de teclado. Retorna IN_GAME para continuar o jogo, CLOSE para fechar, WIN para mostrar a tela de score e GAME_OVER para reiniciar.
int winScreen(GAME_ENV *env);           // Desenha a tela de vitoria e registra o novo score na scoreboard. Executada quando a funcao de atualizacao retorna WIN. Retorna CLOSE para fechar o jogo ou PLAY_AGAIN para reiniciar
void gameDeinit(GAME_ENV *env);         // Salva a scoreboard em disco e libera o espaço de memória utilizado. Executada antes de fechar o jogo.

#endif