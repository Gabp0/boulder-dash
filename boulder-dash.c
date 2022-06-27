// Implementacao das funcoes da biblioteca "boulder-dash.h"
// Feito por Gabriel de Oliveira Pontarolo
#include "boulder-dash.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_ttf.h>

#define RIGHT 1
#define LEFT -1

void _entitiesDeinit(BOARD *entities);

long int _mapIndexP(COORDINATE p)
// retorna index de uma matriz dinamica com aritimetica de ponteiros para uma struct de coordenada
{
    return (p.y * MAP_W) + p.x;
}

long int _mapIndexXY(int x, int y)
// retorna index de uma matriz dinamica com aritimetica de ponteiros para um int _x_ e _y_
{
    return (y * MAP_W) + x;
}

bool _sameCoordinate(COORDINATE a, COORDINATE b)
// testa duas structs de coordenada
{
    return ((a.x == b.x) && (a.y == b.y));
}

void _checkInit(bool init, char *exit_message)
// testa a inicializacao das funcoes
{
    if (init)
        return;    

    perror(exit_message);
    exit(EXIT_FAILURE);
}

int _randInRange(int s, int e)
// gera numero pseudo aleatorio entre o range _s_ e _e_
{
    return s + (rand() % (e - s + 1));
}

ALLEGRO_BITMAP* _getSprite(ALLEGRO_BITMAP *spritesheet, int x, int y)    
// recorta o sprite da spritesheet
{
    ALLEGRO_BITMAP* sprite = al_create_sub_bitmap(spritesheet, x, y, SPRITES_SIZE, SPRITES_SIZE);
    _checkInit(sprite, "nao foi possivel carregar os sprites");

    return sprite;
}

SPRITES *_initSprites()
// carrega os sprites
{
    SPRITES *game_sprites = malloc(sizeof(SPRITES)); 
    ALLEGRO_BITMAP *spritesheet = al_load_bitmap("resources/spritesheet.png");

    int i;
    COORDINATE p;
    
    _checkInit(spritesheet, "nao foi possivel carregar a spritesheet");

    i = 0;  // jewels
    for (p.y = 0; p.y <= 16; p.y+=16)
    {
        for (p.x = 0; p.x <= 16; p.x+=16)
        {
            game_sprites->jewels[i] = _getSprite(spritesheet, p.x, p.y);
            i++;
        }
    }
    
    game_sprites->boulders[0] = _getSprite(spritesheet, 32, 0);  // boulders
    game_sprites->boulders[1] = _getSprite(spritesheet, 32, 16);

    game_sprites->outer_walls[0] = _getSprite(spritesheet, 48, 0); // paredes externas
    game_sprites->outer_walls[1] = _getSprite(spritesheet, 64, 0);

    i = 0;  // paredes internas
    for (p.y = 0; p.y <= 16; p.y+=16)
    {
        for (p.x = 80; p.x <= 96; p.x+=16)
        {
            game_sprites->inner_walls[i] = _getSprite(spritesheet, p.x, p.y);
            i++;
        }
    }

    i = 0;  // terra
    for (p.y = 0; p.y <= 16; p.y+=16)
    {
        for (p.x = 112; p.x <= 160; p.x+=16)
        {
            game_sprites->dirt[i] = _getSprite(spritesheet, p.x, p.y);
            i++;
        }
    }

    game_sprites->miner[0] = _getSprite(spritesheet, 176, 0);   // minerador direita
    game_sprites->miner[1] = _getSprite(spritesheet, 192, 0);   // minerador esquerda

    game_sprites->close_door = _getSprite(spritesheet, 176, 16);    // porta fechada
    game_sprites->open_door = _getSprite(spritesheet, 192, 16);     // porta aberta

    return game_sprites;
}

bool _isBorder(COORDINATE p)
// testa se a coordenada _p_ eh uma das bordas do mapa
{
    if ((p.x <= 0) || (p.y <= 0) || (p.x >= MAP_W-1) || (p.y >= MAP_H-1))
        return true;
    return false;
}

void _addDirt(COORDINATE p, SPRITES *game_sprites, BOARD *entities, bool destroyed)
// adiciona uma entidade do tipo terra
{
    DIRT** aux;
    DIRT* new;

    if (entities->dirt_num > 0) // caso ja houver outras entidades
    {
        aux = realloc(entities->floor, sizeof(DIRT*) * (entities->dirt_num+1));
        _checkInit(aux, "nao foi possivel alocar memoria para as entidades");
        entities->floor = aux;
    }
    else 
    {
        entities->floor = malloc(sizeof(DIRT*));
        _checkInit(entities->floor, "nao foi possivel alocar memoria para as entidades");
    }

    new = malloc(sizeof(DIRT)); // aloca memoria para nova entidade
    _checkInit(new, "nao foi possivel alocar memoria para as entidades");

    new->p.x = p.x;    // inicia variaveis
    new->p.y = p.y;
    new->destroyed = destroyed;
    new->sprite = game_sprites->dirt[_randInRange(0, DIRT_FRAMES-1)];

    entities->floor[entities->dirt_num] = new;
    entities->dirt_num++;

    entities->map[_mapIndexP(p)].index = entities->dirt_num - 1;    // guarda posicao na matriz
    if (destroyed)
        entities->map[_mapIndexP(p)].entity = EMPTY_C;
    else 
        entities->map[_mapIndexP(p)].entity = DIRT_C;
}

void _addBoulder(COORDINATE p, SPRITES *game_sprites, BOARD *entities)
// adiciona uma entidade do tipo boulder
{
    BOULDER** aux;
    BOULDER* new;

    if (entities->bould_num > 0) // caso ja houver outras entidades
    {
        aux = realloc(entities->boulders, sizeof(BOULDER*) * (entities->bould_num+1));
        _checkInit(aux, "nao foi possivel alocar memoria para as entidades");
        entities->boulders = aux;
    }
    else 
    {
        entities->boulders = malloc(sizeof(BOULDER*));
        _checkInit(entities->boulders, "nao foi possivel alocar memoria para as entidades");
    }

    new = malloc(sizeof(BOULDER)); // aloca memoria para nova entidade
    _checkInit(new, "nao foi possivel alocar memoria para as entidades");

    new->p.x = p.x;     // inicia variaveis
    new->p.y = p.y;
    new->can_fall = false;
    new->destroyed = false;
    new->sprite = game_sprites->boulders[_randInRange(0, BOULDER_FRAMES-1)];

    entities->boulders[entities->bould_num] = new;
    entities->bould_num++;

    entities->map[_mapIndexP(p)].index = entities->bould_num - 1;   // guarda a posicao na matriz
    entities->map[_mapIndexP(p)].entity = BOULDER_C;
}

void _addInWall(COORDINATE p, SPRITES *game_sprites, BOARD *entities)
// adiciona uma entidade do tipo parede interna
{
    IN_WALL** aux;
    IN_WALL* new;

    if (entities->inn_wall_num > 0) // caso ja houver outras entidades
    {
        aux = realloc(entities->inner_wall, sizeof(IN_WALL*) * (entities->inn_wall_num+1));
        _checkInit(aux, "nao foi possivel alocar memoria para as entidades");
        entities->inner_wall = aux;
    }
    else 
    {
        entities->inner_wall = malloc(sizeof(IN_WALL*));
        _checkInit(entities->inner_wall, "nao foi possivel alocar memoria para as entidades");
    }

    new = malloc(sizeof(IN_WALL)); // aloca memoria para nova entidade
    _checkInit(new, "nao foi possivel alocar memoria para as entidades");

    new->p.x = p.x;     // inicia variaveis
    new->p.y = p.y;
    new->destroyed = false;
    new->sprite = game_sprites->inner_walls[_randInRange(0, IN_WALL_FRAMES-1)];

    entities->inner_wall[entities->inn_wall_num] = new;
    entities->inn_wall_num++;

    entities->map[_mapIndexP(p)].index = entities->inn_wall_num - 1;    // guarda posicao na matriz
    entities->map[_mapIndexP(p)].entity = WALL_C;
}

void _addOutWall(COORDINATE p, SPRITES *game_sprites, BOARD *entities)
// adiciona uma entidade do tipo parede externa
{
    OUT_WALL** aux = entities->outer_walls;
    OUT_WALL* new;

    if (entities->out_wall_num > 0)  // caso ja houver outras entidades
    {
        aux = realloc(aux, sizeof(OUT_WALL*) * (entities->out_wall_num+1));
        _checkInit(aux, "nao foi possivel alocar memoria para as entidades");
        entities->outer_walls = aux;
    }
    else
    {
        entities->outer_walls = malloc(sizeof(OUT_WALL*));
        _checkInit(entities->outer_walls, "nao foi possivel alocar memoria para as entidades");
    }

    
    new = malloc(sizeof(OUT_WALL)); // aloca memoria para nova entidade
    _checkInit(new, "nao foi possivel alocar memoria para as entidades");

    new->p.x = p.x;     // inicia variaveis
    new->p.y = p.y;
    new->sprite = game_sprites->outer_walls[_randInRange(0, OUT_WALL_FRAMES-1)];

    entities->outer_walls[entities->out_wall_num] = new;
    entities->out_wall_num++;

    entities->map[_mapIndexP(p)].index = entities->out_wall_num - 1;    // guarda posicao na matriz
    entities->map[_mapIndexP(p)].entity = WALL_C;
}

void _addJewel(COORDINATE p, SPRITES *game_sprites, BOARD *entities)
// adiciona uma entidade do tipo joia
{
    JEWEL** aux;
    JEWEL* new;

    if (entities->jew_num > 0)  // caso ja houver outras entidades
    {
        aux = realloc(entities->jewels, sizeof(JEWEL*) * (entities->jew_num+1));
        _checkInit(aux, "nao foi possivel alocar memoria para as entidades");
        entities->jewels = aux;
    }
    else
    {
        entities->jewels = malloc(sizeof(JEWEL*));
        _checkInit(entities->jewels, "nao foi possivel alocar memoria para as entidades");
    }

    new = malloc(sizeof(JEWEL)); // aloca memoria para nova entidade
    _checkInit(new, "nao foi possivel alocar memoria para as entidades");

    new->p.x = p.x;     // inicia variaveis
    new->p.y = p.y;
    new->collected = false;
    new->can_fall = false;
    new->sprite = game_sprites->jewels[_randInRange(0, JEWELS_FRAMES-1)];

    entities->jewels[entities->jew_num] = new;
    entities->jew_num++;

    entities->map[_mapIndexP(p)].index = entities->jew_num - 1; // guarda posicao na matriz
    entities->map[_mapIndexP(p)].entity = JEWEL_C;
}

void _addPlayerSpawn(COORDINATE p, SPRITES *game_sprites, BOARD *entities)
// adiciona o jogador e o ponto de spawn
{
    entities->player = malloc(sizeof(MINER));   // aloca memoria
    _checkInit(entities->player, "nao foi possivel alocar memoria para as entidades");

    entities->player->p.x = p.x;     // inicia variaveis
    entities->player->p.y = p.y;
    entities->player->alive = true;
    entities->player->invul = false;
    entities->player->time_left = MAX_TIME;
    entities->player->direction = RIGHT;
    entities->player->jew = 0;
    entities->player->sprite_right = game_sprites->miner[0];
    entities->player->sprite_left = game_sprites->miner[1];

    entities->map[_mapIndexP(p)].index = 0;         // guarda posicao na matriz
    entities->map[_mapIndexP(p)].entity = PLAYER_C;
}

void _addExit(COORDINATE p, SPRITES *game_sprites, BOARD *entities)
// adiciona a porta de saida
{
    entities->exit_point = malloc(sizeof(EXIT)); // aloca memoria
    _checkInit(entities->exit_point, "nao foi possivel alocar memoria para as entidades");

    entities->exit_point->p.x = p.x;     // inicia variveis
    entities->exit_point->p.y = p.y;
    entities->exit_point->open = false;
    entities->exit_point->sprite_doorclose = game_sprites->close_door;
    entities->exit_point->sprite_dooropen = game_sprites->open_door;

    entities->map[_mapIndexP(p)].index = 0;
    entities->map[_mapIndexP(p)].entity = EXIT_C;
}

BOARD *_initEntities(void)
// inicializa a struct que guarda as entidades
{
    BOARD *new;
    
    new = malloc(sizeof(BOARD));   // aloca memoria para as entidades do jogo
    _checkInit(new, "nao foi possivel alocar memoria para as entidades");

    new->map = malloc(sizeof(ENT_POS) * MAP_H * MAP_W); // aloca memoria para o mapa do jogo
    _checkInit(new->map, "nao foi possivel alocar memoria para o mapa do jogo");

    new->bould_num = 0;
    new->dirt_num = 0;
    new->jew_num = 0;
    new->inn_wall_num = 0;
    new->out_wall_num = 0;

    return new;
}

void _readMap(BOARD *entities, SPRITES *game_sprites)
// le o arquivo "map.txt" e cria as entidades de acordo
{
    int w, h;
    COORDINATE p;
    char c;
    bool hasExit = false, hasSpawn = false;
    FILE *map;

    map = fopen("resources/map.txt", "r"); // le o arquivo com o mapa
    _checkInit(map, "nao foi possivel carregar o mapa do jogo");

    entities->bould_num = 0;        // inicializa as variaveis
    entities->dirt_num = 0;
    entities->jew_num = 0;
    entities->inn_wall_num = 0;
    entities->out_wall_num = 0;

    fscanf(map, "%d %d", &h, &w);   // checa o tamanho do mapa
    if ((h != MAP_H) || (w != MAP_W))
    {
        fprintf(stderr, "tamanho do mapa invalido");
        exit(EXIT_FAILURE);
    }

    getc(map);
    for (p.y = 0; p.y < MAP_H; p.y++) // cria as entidades
    {
        for (p.x = 0; p.x < MAP_W; p.x++)
        {
            c = getc(map);
            switch (c)
            {
            case DIRT_C:        // terra
                _addDirt(p, game_sprites, entities, false);
                break;
            
            case EMPTY_C:       // espaco vazio
                _addDirt(p, game_sprites, entities, true);
                break;

            case BOULDER_C:     // boulder
                _addBoulder(p, game_sprites, entities);
                break;

            case JEWEL_C:       // joia
                _addJewel(p, game_sprites, entities);
                break;
            
            case WALL_C:        // parede
                if (_isBorder(p))
                    _addOutWall(p, game_sprites, entities); // externa
                else
                    _addInWall(p, game_sprites, entities);  // interna
                break;

            case PLAYER_C:  // jogador
                if (!hasSpawn)
                {   // testa se o jogador ja foi adicinado
                    _addPlayerSpawn(p, game_sprites, entities);
                    hasSpawn = true;
                }
                else 
                {
                    fprintf(stderr, "formato do mapa invalido, ha mais de um spawn");
                    exit(EXIT_FAILURE);
                }
                break;
            
            case EXIT_C:    // porta de saida
                if (!hasExit)
                {   // testa se a porta de saida ja foi adicionada
                    _addExit(p, game_sprites, entities);
                    hasExit = true;
                }
                else 
                {
                    fprintf(stderr, "formato do mapa invalido, ha mais de uma saida");
                    exit(EXIT_FAILURE);
                }
                break;

            default:
                fprintf(stderr, "formato do mapa invalido, character desconhecido '%c'", c);
                exit(EXIT_FAILURE);
            }
        }
        getc(map);  // remove o "\n"
    }
    
    fclose(map);
}

HUD *_initHud()
// inicializa a hud no topo
{
    HUD *new;

    new = malloc(sizeof(HUD));  // aloca memoria
    _checkInit(new, "nao foi possivel alocar memoria para a hud");

    new->font = al_load_ttf_font("resources/BebasNeue-Regular.ttf", 16, 0); // inicializa as variaveis
    new->timer_color = al_map_rgb_f(1, 1, 1);
    new->score_color = al_map_rgb_f(0.984, 0.886, 0.439);
    new->score_x = 7 * SPRITES_SIZE;
    new->timer_x = 27 * SPRITES_SIZE;
    new->jew_x = 5 * SPRITES_SIZE;

    return new;
}

MENU *_initMenu(void)
// incializa o menu de pausa
{
    MENU *new = malloc(sizeof(MENU));   // aloca memoria
    _checkInit(new, "nao foi possivel alocar memoria para o menu");

    new->info = al_load_bitmap("resources/pausemenu.png");  // incializa as variaveis
    _checkInit(new->info, "nao foi possivel carregar a imagem para o menu");
    new->show_menu = false;

    return new;
}

SCREEN *_initScreen(void)
// inicializa a tela de score
{
    SCREEN *new = malloc(sizeof(SCREEN));   // aloca memoria
    _checkInit(new, "nao foi possivel alocar memoria para a tela");

    new->regular = al_load_ttf_font("resources/OpenSans/OpenSans-Regular.ttf", 12, 0);  // carrega as fontes
    new->title = al_load_ttf_font("resources/OpenSans/OpenSans-Bold.ttf", 32, 0);
    new->credits = al_load_ttf_font("resources/OpenSans/OpenSans-Italic.ttf", 12, 0);

    return new;
}

SAMPLES *_initSamples(void)
// carrega os arquivos de audio
{
    SAMPLES *new = malloc(sizeof(SAMPLES)); // aloca memoria
    _checkInit(new, "nao foi possivel alocar memoria para o audio");

    new->miner_death = al_load_sample("resources/sounds/minerdeath.wav");       // carrega os arquivos
    _checkInit(new->miner_death, "nao foi possivel carregar os sons");
    new->pickup_jewel = al_load_sample("resources/sounds/pickupjew.wav");
    _checkInit(new->pickup_jewel, "nao foi possivel carregar os sons");
    new->open_door = al_load_sample("resources/sounds/dooropen.wav");
    _checkInit(new->open_door, "nao foi possivel carregar os sons");
    new->win_game = al_load_sample("resources/sounds/win.wav");
    _checkInit(new->win_game, "nao foi possivel carregar os sons");
    new->power_up = al_load_sample("resources/sounds/powerup.wav");
    _checkInit(new->power_up, "nao foi possivel carregar os sons");
    new->bakcground_music = al_load_audio_stream("resources/sounds/cave.ogg", 2, 2048);
    _checkInit(new->bakcground_music, "nao foi possivel carregar a musica de fundo");

    return new;
}

SCOREBOARD *_initScoreboard(void)
// le o arquivo "scores.txt" e inicializa a scoreboard
{
    FILE *scores_file;
    SCOREBOARD *highscores;
    int i;

    scores_file = fopen("resources/scores.txt", "r");   // abre o arquivo
    _checkInit(scores_file, "nao foi possivel carregar a scoreboard");

    highscores = malloc(sizeof(SCOREBOARD));    // aloca memoria
    _checkInit(highscores, "nao foi possivel alocar memoria para a scoreboard");

    fscanf(scores_file, "%d", &highscores->size);   //quantidade de scores armazenados no arquivo
    highscores->scores = malloc(sizeof(SCORE)*highscores->size);
    _checkInit(highscores->scores, "nao foi possivel alocar memoria para os scores");

    getc(scores_file);  // remove "\n"
    for (i=0; i < highscores->size; i++)
    {
        fgets(highscores->scores[i].datetime, 20, scores_file); // le data
        getc(scores_file);  // remove ";"
        fscanf(scores_file, "%d", &highscores->scores[i].time); // le o tempo
        getc(scores_file);  // remove "\n"
    }

    fclose(scores_file);

    return highscores;
}

GAME_ENV* loadGame(void)
// Executado uma vez ao abrir o jogo, realiza a inicializacao das bibliotecas, leitura de arquivos e alocacao de memoria.
// Retorna um ponteiro para uma struct "GAME_ENV" com as variaveis necessarias para o funcionamento do jogo 
{
    GAME_ENV* env;

    env = malloc(sizeof(GAME_ENV)); // aloca memoria para a struct
    _checkInit(env, "nao foi possivel inicializar as variaveis do jogo");

    _checkInit(al_init(), "nao foi possivel inicializar a biblioteca allegro"); // inicializacao de biblioteca e addons
    _checkInit(al_install_keyboard(), "nao foi possivel inicializar o teclado");
    _checkInit(al_init_image_addon(), "nao foi possivel inicializar o addon de imagens");
    _checkInit(al_init_font_addon(), "nao foi possivel iniciar o addon de fontes");
    _checkInit(al_init_ttf_addon(), "nao foi possivel incializar o addon de fontes");
    _checkInit(al_install_audio(), "nao foi possivel inicializar o audio");
    _checkInit(al_init_acodec_addon(), "nao foi possivel inicializar o addon de codecs de audio");
    _checkInit(al_reserve_samples(SAMPLES_NUM), "nao foi possivel reservar as samples");
    
    env->timer = al_create_timer(1.0 / FPS);    // timer
    _checkInit(env->timer, "nao foi possivel inicializar o timer");

    env->queue = al_create_event_queue();   // event queue
    _checkInit(env->queue, "nao foi possivel inicializar os eventos");

    al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_SUGGEST);  // tela e buffer da tela
    al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);
    al_set_new_bitmap_flags(ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
    env->disp = al_create_display(DISP_W, DISP_H);
    _checkInit(env->disp, "nao foi possivel inicializar o display");
    env->buffer = al_create_bitmap(BUFFER_W, BUFFER_H);
    _checkInit(env->buffer, "nao foi possivel inicializar o display");

    al_register_event_source(env->queue, al_get_keyboard_event_source());   // registra as fontes de evento
    al_register_event_source(env->queue, al_get_display_event_source(env->disp));
    al_register_event_source(env->queue, al_get_timer_event_source(env->timer));

    env->entities = _initEntities();      // inicializacao dos componentes
    env->highscores = _initScoreboard();
    env->game_samples = _initSamples();
    env->game_sprites = _initSprites();
    env->pause_menu = _initMenu();
    env->score_screen = _initScreen();
    env->top_hud = _initHud();

    return env;
}

void _startHud(GAME_ENV *env)
// valores iniciais das variaveis da hud 
{
    env->top_hud->score = 0;
    env->top_hud->jew_num = env->entities->jew_num;
    env->top_hud->time = MAX_TIME;
}

bool _entityIsAllocated(BOARD *entities)
// testa se ja ha entidades alocadas na struct tipo BOARD
{
    if ((entities->dirt_num > 0) || (entities->bould_num > 0) || (entities->inn_wall_num > 0) ||
        (entities->out_wall_num > 0) || (entities->jew_num > 0))
        return true;
    return false;
}

void initGame(GAME_ENV *env)
// Define os valores iniciais das variaveis do jogo em _env_. Executado sempre que a fase é inciada/reiniciada
{
    memset(env->key, 0, sizeof(env->key));
    env->frames = 0;
    env->redraw = false;
    env->pause_menu->show_menu = false;
    env->konami_code = INVALID;

    al_set_audio_stream_playmode(env->game_samples->bakcground_music, ALLEGRO_PLAYMODE_LOOP);
    al_set_audio_stream_gain(env->game_samples->bakcground_music, 0.35);
    al_attach_audio_stream_to_mixer(env->game_samples->bakcground_music, al_get_default_mixer());
    
    if(_entityIsAllocated(env->entities))
        _entitiesDeinit(env->entities);
    _readMap(env->entities, env->game_sprites);

    _startHud(env);
    al_start_timer(env->timer);
}

void _drawHud(HUD *top_hud, ALLEGRO_BITMAP **jewel_spirtes)
// desenha a hud no topo da tela
{
    al_draw_textf(top_hud->font, top_hud->score_color, top_hud->score_x, 0, 0, "%d", top_hud->score);   // joias coletadas
    al_draw_textf(top_hud->font, top_hud->timer_color, top_hud->timer_x, 0, 0, "%d", top_hud->time);    // tempo restante
    al_draw_textf(top_hud->font, top_hud->score_color, top_hud->jew_x, 0, 0, "%d", top_hud->jew_num);   // joias restantes
    al_draw_bitmap(jewel_spirtes[top_hud->time % 4], top_hud->jew_x+SPRITES_SIZE, 0, 0);    // sprite das joias
}

void _drawEntities(BOARD *entities)
// desenha todas as entidades
{
    int i;

    for (i=0; i<entities->dirt_num; i++)    // terra
    {
        if (!entities->floor[i]->destroyed)
            al_draw_bitmap(entities->floor[i]->sprite, entities->floor[i]->p.x*SPRITES_SIZE, (entities->floor[i]->p.y*SPRITES_SIZE)+HUD_H, 0);
    }

    for (i=0; i<entities->jew_num; i++) // joias
    {
        if (!entities->jewels[i]->collected)    
            al_draw_bitmap(entities->jewels[i]->sprite, entities->jewels[i]->p.x*SPRITES_SIZE, (entities->jewels[i]->p.y*SPRITES_SIZE)+HUD_H, 0);
    }

    for (i=0; i<entities->inn_wall_num; i++)    // paredes internas
    {
        if (!entities->inner_wall[i]->destroyed)    
            al_draw_bitmap(entities->inner_wall[i]->sprite, entities->inner_wall[i]->p.x*SPRITES_SIZE, (entities->inner_wall[i]->p.y*SPRITES_SIZE)+HUD_H, 0);
    }

    for (i=0; i<entities->out_wall_num; i++)    // paredes externas
        al_draw_bitmap(entities->outer_walls[i]->sprite, entities->outer_walls[i]->p.x*SPRITES_SIZE, (entities->outer_walls[i]->p.y*SPRITES_SIZE)+HUD_H, 0);

    for (i=0; i<entities->bould_num; i++)   // boulders
    {
        if (!entities->boulders[i]->destroyed) 
            al_draw_bitmap(entities->boulders[i]->sprite, entities->boulders[i]->p.x*SPRITES_SIZE, (entities->boulders[i]->p.y*SPRITES_SIZE)+HUD_H, 0);
    }

    if (entities->player->alive)    // jogador
        switch (entities->player->direction)
        { // direcao do sprite
        case RIGHT: 
            al_draw_bitmap(entities->player->sprite_right, entities->player->p.x*SPRITES_SIZE, (entities->player->p.y*SPRITES_SIZE)+HUD_H, 0);
            break;
        case LEFT:
            al_draw_bitmap(entities->player->sprite_left, entities->player->p.x*SPRITES_SIZE, (entities->player->p.y*SPRITES_SIZE)+HUD_H, 0);
            break;
        default:
            break;
        }
        
    if (entities->exit_point->open) // porta de saida, aberta ou fechada
        al_draw_bitmap(entities->exit_point->sprite_dooropen, entities->exit_point->p.x*SPRITES_SIZE, (entities->exit_point->p.y*SPRITES_SIZE)+HUD_H, 0);
    else 
        al_draw_bitmap(entities->exit_point->sprite_doorclose, entities->exit_point->p.x*SPRITES_SIZE, (entities->exit_point->p.y*SPRITES_SIZE)+HUD_H, 0);
}

void drawBoard(GAME_ENV *env)
// Desenha a tela durante o jogo. Executada todo final de loop em cada frame.
{
    if(al_event_queue_is_empty(env->queue) && env->redraw)
    {
        al_set_target_bitmap(env->buffer);
        al_clear_to_color(al_map_rgb(0,0,0));

        if (env->pause_menu->show_menu) // menu de pausa
        {
            al_draw_bitmap(env->pause_menu->info, 0, 0, 0);
        }
        else    // hud e entidades
        {
            _drawHud(env->top_hud, env->game_sprites->jewels);
            _drawEntities(env->entities);
        }

        al_set_target_backbuffer(env->disp);
        al_draw_scaled_bitmap(env->buffer, 0, 0, BUFFER_W, BUFFER_H, 0, 0, DISP_W, DISP_H, 0);
        al_flip_display();
        env->redraw = false;
    }
}

bool _checkMinerCollision(ENT_POS *map, COORDINATE p, bool invul)
// testa a colisao do minerador na coordenada _p_
{
    if (invul && !_isBorder(p))
        return false;
    if (map[_mapIndexP(p)].entity == DIRT_C)
        return false;
    if (map[_mapIndexP(p)].entity == EMPTY_C)
        return false;
    if (map[_mapIndexP(p)].entity == JEWEL_C)
        return false;
    return true;
}

bool _checkBoulderPush(BOARD *entities, COORDINATE p, int d)
// testa se e possivel empurrar a boulder
{
    switch (d)
    {
        case RIGHT: // empurrar para direita
            if ((entities->map[_mapIndexP(p)].entity == BOULDER_C) && (entities->map[_mapIndexXY(p.x+1, p.y)].entity == EMPTY_C))
            {
                entities->boulders[entities->map[_mapIndexP(p)].index]->p.x += ENTITY_SPEED;
                return true;
            }
            return false;

        case LEFT:  // empurrar para esquerda
            if ((entities->map[_mapIndexP(p)].entity == BOULDER_C) && (entities->map[_mapIndexXY(p.x-1, p.y)].entity == EMPTY_C))
            {
                entities->boulders[entities->map[_mapIndexP(p)].index]->p.x -= ENTITY_SPEED;
                return true;
            }
            return false;
        
        default:
            return false;
    }
}

void _minerUpdate(GAME_ENV *env)
// atualiza a movimentacao do jogador
{
    COORDINATE aux;

    if (!env->entities->player->alive)  // morto
        return;

    if ((env->frames % 5) != 0) // reduz a velocidade de movimentacao
        return;

    if (env->key[ALLEGRO_KEY_UP])   // tecla para cima
    {
        aux.x = env->entities->player->p.x;
        aux.y = env->entities->player->p.y-MINER_SPEED;
        if (!_checkMinerCollision(env->entities->map, aux, env->entities->player->invul))   // checa colisao
            env->entities->player->p.y -= MINER_SPEED; 

    }
    else if (env->key[ALLEGRO_KEY_DOWN])    // tecla para baixo
    {
        aux.x = env->entities->player->p.x;
        aux.y = env->entities->player->p.y+MINER_SPEED;
        if (!_checkMinerCollision(env->entities->map, aux, env->entities->player->invul))   // checa colisao
            env->entities->player->p.y += MINER_SPEED; 
    }
    else if (env->key[ALLEGRO_KEY_LEFT])    // tecla para esquerda
    {
        aux.x = env->entities->player->p.x-MINER_SPEED;
        aux.y = env->entities->player->p.y;
        if (!_checkMinerCollision(env->entities->map, aux, env->entities->player->invul) || _checkBoulderPush(env->entities, aux, LEFT))
        {   // checa colisao e empurrar as boulders
            env->entities->player->p.x -= MINER_SPEED;
            env->entities->player->direction = LEFT;
        }
    }
    else if (env->key[ALLEGRO_KEY_RIGHT])   // tecla para direita
    {   // checa colisao e empurrar as boulders
        aux.x = env->entities->player->p.x+MINER_SPEED;
        aux.y = env->entities->player->p.y;
        if (!_checkMinerCollision(env->entities->map, aux, env->entities->player->invul) || _checkBoulderPush(env->entities, aux, RIGHT))
        {
            env->entities->player->p.x += MINER_SPEED; 
            env->entities->player->direction = RIGHT;
        }
    }
    
}

void _dirtUpdate(BOARD *entities)
// remove a terra na posicao do minerador
{
    if (entities->player->invul)
        return;

    if (entities->map[_mapIndexP(entities->player->p)].entity == DIRT_C)
        entities->floor[entities->map[_mapIndexP(entities->player->p)].index]->destroyed = true;
}

bool _checkFall(BOARD *entities, COORDINATE p, bool can_fall)
// testa a queda das boulders e das joias
{
    if (!((entities->map[_mapIndexXY(p.x, p.y+1)].entity == EMPTY_C) || // colisao com as paredes
        (entities->map[_mapIndexXY(p.x, p.y+1)].entity == PLAYER_C))) 
        return false;    

    // colide com o jogador apenas se ja estiver em queda
    if ((entities->map[_mapIndexXY(p.x, p.y+1)].entity == PLAYER_C) && !can_fall)  
        return false;

    return true;
}

int _canRoll(ENT_POS *map, COORDINATE p)
// testa o rolamento das boulders e das joias
{

    // testa se ha uma boulder ou gema em baixo
    if ((map[_mapIndexXY(p.x, p.y+1)].entity != BOULDER_C) && (map[_mapIndexXY(p.x, p.y+1)].entity != JEWEL_C))
        return 0;

    // testa a direita e direita a baixo
    if (map[_mapIndexXY(p.x+1, p.y)].entity == EMPTY_C) 
    {
        if (map[_mapIndexXY(p.x+1, p.y+1)].entity == EMPTY_C)
            return RIGHT;
    }

    // testa a esquerda e esquerda a baixo
    if (map[_mapIndexXY(p.x-1, p.y)].entity == EMPTY_C) 
    {
        if (map[_mapIndexXY(p.x-1, p.y+1)].entity == EMPTY_C)
            return LEFT;
    }

    return 0;
}

void _boulderUpdate(BOARD *entities, int frames)
// atualiza a posicao das boulders testando o rolamento e a queda 
{
    if ((frames % 5) != 0) // reduz a velocidade de movimentacao
        return;

    int i;
    for (i = 0; i < entities->bould_num; i++)
    {
        // testa a queda
        entities->boulders[i]->can_fall = _checkFall(entities, entities->boulders[i]->p, entities->boulders[i]->can_fall);    // testa a colisao
        if (entities->boulders[i]->can_fall)
            entities->boulders[i]->p.y += ENTITY_SPEED;

        switch (_canRoll(entities->map, entities->boulders[i]->p))
        {
        case RIGHT:
            entities->boulders[i]->p.x += ENTITY_SPEED;
            break;
        
        case LEFT:
            entities->boulders[i]->p.x -= ENTITY_SPEED;
            break;

        default:
            break;
        }
    }
    
}

void _collectJewel(BOARD *entities, JEWEL *j, ALLEGRO_SAMPLE *pickup_jew)
{
    if ((entities->map[_mapIndexP(j->p)].entity == PLAYER_C) && !j->collected)
    {
        al_play_sample(pickup_jew, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        entities->player->jew++;
        j->collected = true;
    }
}

void _jewelUpdate(BOARD *entities, ALLEGRO_SAMPLE *pickup_jew, int frames)
{

    int i;
    for (i=0; i<entities->jew_num; i++)
    {
        if (!entities->jewels[i]->collected)
        {
            _collectJewel(entities, entities->jewels[i], pickup_jew);

            if ((frames % 5) == 0) // reduz a velocidade de queda e rolamento
            {
                // testa a queda
                entities->jewels[i]->can_fall = _checkFall(entities, entities->jewels[i]->p, entities->jewels[i]->can_fall);    
                if (entities->jewels[i]->can_fall)
                    entities->jewels[i]->p.y += ENTITY_SPEED;

                // testa o rolamento
                switch (_canRoll(entities->map, entities->jewels[i]->p))
                {
                case RIGHT: 
                    entities->jewels[i]->p.x += ENTITY_SPEED;
                    break;
                
                case LEFT:
                    entities->jewels[i]->p.x -= ENTITY_SPEED;
                    break;

                default:
                    break;
                }
            }
        }
    }
}

bool _checkMinerDeath(GAME_ENV *env)
// testa se o jogador foi morto
{
    int i;

    if(env->entities->player->invul)
        return false;

    for (i = 0; i < env->entities->bould_num; i++)
    {  // atingido por uma boulder
        if (_sameCoordinate(env->entities->player->p, env->entities->boulders[i]->p) && env->entities->player->alive)
        {
            env->entities->player->alive = false;
            al_play_sample(env->game_samples->miner_death, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            env->top_hud->time = 4; // contagem para reset
            return true;
        }
    }

    if ((env->top_hud->time <= 0) && env->entities->player->alive)  // tempo acabou
    {
        env->entities->player->alive = false;
        al_play_sample(env->game_samples->miner_death, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        env->top_hud->time = 4; // contagem para reset
        return true;
    }

    return false;
}

void _updateTopHud(GAME_ENV *env)
// atualiza os valores da hud do topo
{
    if (env->frames == 60)
        env->top_hud->time -= 1;
    
    env->top_hud->jew_num = env->entities->jew_num - env->entities->player->jew;
    env->top_hud->score = env->entities->player->jew;
}

void _updateMap(BOARD *entities)
// atualiza a posicao das entidades na matriz
{
    COORDINATE p;
    for (p.y=1; p.y<MAP_H-1; p.y++)   // mapa vazio
    {
        for (p.x=1; p.x<MAP_W-1; p.x++)
        {
            entities->map[_mapIndexP(p)].entity = EMPTY_C;
        }
    }
    for (p.y = 0; p.y < entities->dirt_num; p.y++)    // atualiza a terra 
    {
        if (!entities->floor[p.y]->destroyed)
        {
            entities->map[_mapIndexP(entities->floor[p.y]->p)].entity = DIRT_C;
            entities->map[_mapIndexP(entities->floor[p.y]->p)].index = p.y;
        }
    }
    for (p.y = 0; p.y < entities->bould_num; p.y++)   // atualiza a posicao das boulders
    {
        if (!entities->boulders[p.y]->destroyed)
        {
            entities->map[_mapIndexP(entities->boulders[p.y]->p)].entity = BOULDER_C;
            entities->map[_mapIndexP(entities->boulders[p.y]->p)].index = p.y;
        }
    }
    for (p.y = 0; p.y < entities->jew_num; p.y++)     // atualiza as gemas 
    {
        if (!entities->jewels[p.y]->collected)
        {
            entities->map[_mapIndexP(entities->jewels[p.y]->p)].entity = JEWEL_C;
            entities->map[_mapIndexP(entities->jewels[p.y]->p)].index = p.y;
        }
    }
    for (p.y = 0; p.y < entities->inn_wall_num; p.y++)    // atualiza as paredes internas
    {
        if (!entities->inner_wall[p.y]->destroyed)
        {
            entities->map[_mapIndexP(entities->inner_wall[p.y]->p)].entity = WALL_C;
            entities->map[_mapIndexP(entities->inner_wall[p.y]->p)].index = p.y;
        }
    }
    entities->map[_mapIndexP(entities->player->p)].entity = PLAYER_C;   // atualiza o jogador
}

bool _updateExit(GAME_ENV *env)
// testa se a porta de saida pode ser aberta e se o jogador pode sair
{
    if (!env->entities->exit_point->open && (env->top_hud->jew_num == 0))   // abre a porta
    {
        al_play_sample(env->game_samples->open_door, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        env->entities->exit_point->open = true;
    }

    if (env->entities->exit_point->open &&  // sai pela porta e ganha o jogo
        (_sameCoordinate(env->entities->player->p, env->entities->exit_point->p)))
    {
        al_play_sample(env->game_samples->win_game, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        return WIN;
    }
    return IN_GAME;
}

void _triggerCheat(GAME_ENV *env)
// liga a invulnerabilidade
{
    al_play_sample(env->game_samples->power_up, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    env->entities->player->invul = true;
}

CHEAT_CODE _checkCheatCode(GAME_ENV *env)
// maquina de estados para ativar o cheat
// cima, cima, baixo, baixo, esquerda, direita, esquerda, direita, b, a
{
    switch (env->konami_code)
    {
    case INVALID:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_UP)
            return UP1;
        else
            return INVALID;
        break;

    case UP1:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_UP)
            return UP2;
        else
            return INVALID;
        break;

    case UP2:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_DOWN)
            return DOWN1;
        else
            return INVALID;
        break;

    case DOWN1:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_DOWN)
            return DOWN2;
        else
            return INVALID;
        break;

    case DOWN2:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_LEFT)
            return LEFT1;
        else
            return INVALID;
        break;
    case LEFT1:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
            return RIGHT1;
        else
            return INVALID;
        break;
    case RIGHT1:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_LEFT)
            return LEFT2;
        else
            return INVALID;
        break;
    case LEFT2:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
            return RIGHT2;
        else
            return INVALID;
        break;
    case RIGHT2:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_A)
            return A;
        else
            return INVALID;
        break;
    case A:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_B)
        {
            _triggerCheat(env);
            return B;
        }
        else
            return INVALID;
        break;

    default:
        return INVALID;
        break;
    }
}

void _keyboardUpdate(GAME_ENV *env)
{
    int i;

    switch (env->event.type)
    {
        case ALLEGRO_EVENT_TIMER:
            for(i = 0; i < ALLEGRO_KEY_MAX; i++)
                env->key[i] &= KEY_SEEN;
            break;

        case ALLEGRO_EVENT_KEY_DOWN:
            env->key[env->event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
            break;
        case ALLEGRO_EVENT_KEY_UP:
            env->key[env->event.keyboard.keycode] &= KEY_RELEASED;
            break;
    }
}

int updateGameState(GAME_ENV *env)
// Atualiza o estado de todas as entidades do jogo, variaveis da struct _env_ e le os inputs de teclado. 
// Retorna IN_GAME para continuar o jogo, CLOSE para fechar, WIN para mostrar a tela de score e GAME_OVER para reiniciar.
{
    int done = IN_GAME;

    al_wait_for_event(env->queue, &env->event); 

    switch(env->event.type)
    {
        case ALLEGRO_EVENT_TIMER:
            if (env->entities->player->alive && !env->pause_menu->show_menu)    // atualiza as entidades
            {
                _minerUpdate(env);
                _dirtUpdate(env->entities);
                _boulderUpdate(env->entities, env->frames);
                _jewelUpdate(env->entities, env->game_samples->pickup_jewel, env->frames);
                _updateTopHud(env);
                _checkMinerDeath(env);
                done = _updateExit(env);
                _updateMap(env->entities);
                env->redraw = true;
            }
            else if (!env->entities->player->alive) // contagem para respawn caso o player morra
            {
                _updateTopHud(env); 
                if (env->top_hud->time <= 0)
                    done = GAME_OVER;
                env->redraw = true;
            }
                
            if(env->key[ALLEGRO_KEY_ESCAPE])    // fecha oo jogo
                done = CLOSE;

            break;

        case ALLEGRO_EVENT_KEY_DOWN:    // abre menu de ajuda
            if (((env->event.keyboard.keycode == ALLEGRO_KEY_H) || (env->event.keyboard.keycode == ALLEGRO_KEY_F1)) && !env->pause_menu->show_menu)
            {
                env->pause_menu->show_menu = true;
                env->redraw = true;
            }
            // fecha o menu de ajuda
            else if (((env->event.keyboard.keycode == ALLEGRO_KEY_H) || (env->event.keyboard.keycode == ALLEGRO_KEY_F1)) && env->pause_menu->show_menu)
                env->pause_menu->show_menu = false;

            else if (env->event.keyboard.keycode == ALLEGRO_KEY_ENTER)  // reinicia a fase
                done = GAME_OVER;

            env->konami_code = _checkCheatCode(env);    // testa ativacao do cheat
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:   // fecha o jogo
            done = CLOSE;
            break;
    }

    if(env->frames < FPS)   // contador de fps
        env->frames++;
    else
        env->frames = 1;

    _keyboardUpdate(env);   // atualiza o teclado

    return done;
}

void _drawWinScreen(GAME_ENV *env, int position)
// desenha a tela de vitoria com o score
{
    int i;
    
    al_set_target_bitmap(env->buffer);
    al_clear_to_color(al_map_rgb(0,0,0));

    al_draw_text(env->score_screen->title, al_map_rgb(250, 20, 20), 236, 2, 0, "PARABÉNS");
    al_draw_textf(env->score_screen->regular, al_map_rgb(250, 250, 250), 128, 60, 0, "Seu tempo foi de %d segundos                Sua posição na scoreboard é %d", MAX_TIME-env->top_hud->time, position);
    al_draw_text(env->score_screen->regular, al_map_rgb(199, 130, 26), 70, 90, 0, "POSIÇÃO");
    al_draw_text(env->score_screen->regular, al_map_rgb(250, 250, 250), 200, 90, 0, "DATA");
    al_draw_text(env->score_screen->regular, al_map_rgb(219, 46, 79), 485, 90, 0, "TEMPO");
    for (i = 0; (i < env->highscores->size) && (i < 10); i++)   // desenha scoreboard
    {
        if (i == position-1)
            al_draw_filled_rectangle(70, 110+(i*20), 570, 130+(i*20), al_map_rgb(80, 173, 95));

        al_draw_line(70, 110+(i*20), 570, 110+(i*20), al_map_rgb(163, 160, 160), 2);
        al_draw_textf(env->score_screen->regular, al_map_rgb(199, 130, 26), 90, 110+(i*20), 0, "%d", i+1);
        al_draw_textf(env->score_screen->regular, al_map_rgb(250, 250, 250), 200, 110+(i*20), 0, "%s", env->highscores->scores[i].datetime);
        al_draw_textf(env->score_screen->regular, al_map_rgb(219, 46, 79), 475, 110+(i*20), 0, "%d segundos", env->highscores->scores[i].time);
    }
    al_draw_text(env->score_screen->regular, al_map_rgb(255, 250, 250), 70, 330, 0, "Presione Enter para jogar novamente ou Esc para sair");
    al_draw_text(env->score_screen->credits, al_map_rgb(255, 0, 0), 475, 336, 0, "Feito por Gabriel Pontarolo");    // creditos
    al_draw_text(env->score_screen->credits, al_map_rgb(255, 0, 0), 160, 350, 0, "Todos os recursos audiovisuais utilizados foram retirados de https://opengameart.org/");    // creditos

    al_set_target_backbuffer(env->disp);
    al_draw_scaled_bitmap(env->buffer, 0, 0, BUFFER_W, BUFFER_H, 0, 0, DISP_W, DISP_H, 0);
    al_flip_display();
    env->redraw = false;
}

void _swapScores(SCORE *highscores, int a, int b)
// troca dois elementos de um array do tipo SCORE
{
    SCORE aux;
    
    strncpy(aux.datetime, highscores[a].datetime, 20);
    aux.time = highscores[a].time;

    strncpy(highscores[a].datetime, highscores[b].datetime, 20);
    highscores[a].time = highscores[b].time;

    strncpy(highscores[b].datetime, aux.datetime, 20);
    highscores[b].time = aux.time;
}

int _insertionSortScore(SCORE *highscores, int e)
// coloca o ultimo elemento de um array do tipo score em ordem
{
    int i = e - 1;
    while ((i >= 0) && (highscores[i].time > highscores[i+1].time))
    {
        _swapScores(highscores, i, i+1);
        i--;
    }

    return i + 1;
}

int _addNewScore(GAME_ENV *env)
// adiciona um novo score na scoreboard a partir do tempo da ultima jogada e no horario
{
    time_t t;
    struct tm ct;
    SCORE *aux = NULL;

    t = time(NULL);
    ct = *localtime(&t);

    aux = realloc(env->highscores->scores, sizeof(SCORE)*(env->highscores->size+1));    // aloca memoria
    _checkInit(aux, "nao foi possivel salvar o score");
    env->highscores->scores = aux;

    // data no formato dd/mm/aaaa hh:mm:ss
    snprintf(env->highscores->scores[env->highscores->size].datetime, sizeof(char)*20, "%02d/%02d/%d %02d:%02d:%02d",ct.tm_mday, ct.tm_mon+1, ct.tm_year+1900, ct.tm_hour, ct.tm_min, ct.tm_sec);
    env->highscores->scores[env->highscores->size].time = MAX_TIME - env->top_hud->time;    // tempo gasto na ultima jogada
    env->highscores->size++;

    return _insertionSortScore(env->highscores->scores, env->highscores->size-1);   // ordena o vetor
}

int winScreen(GAME_ENV *env)
// Desenha a tela de vitoria e registra o novo score na scoreboard. Executada quando a funcao de atualizacao retorna WIN.
// Retorna CLOSE para fechar o jogo ou PLAY_AGAIN para reiniciar
{
    if (env->redraw)    // desenha a tela e registra o score
        _drawWinScreen(env, _addNewScore(env) + 1);

    al_wait_for_event(env->queue, &env->event);
    switch (env->event.type)    // espera pelo input de teclado
    {
    case ALLEGRO_EVENT_KEY_DOWN:
        if (env->event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
            return CLOSE;
        else if (env->event.keyboard.keycode == ALLEGRO_KEY_ENTER)
            return PLAY_AGAIN;
        break;
    
    case ALLEGRO_EVENT_DISPLAY_CLOSE:
        return CLOSE;
        break;

    default:
        break;
    }

    return IN_GAME;
}

void _saveScoreToFile(SCOREBOARD *highscores)
// guarda a scoreboard no arquivo "scores.txt"
{
    FILE *score_file;
    int i;

    score_file = fopen("resources/scores.txt", "w");    // abre o arquivo
    _checkInit(score_file, "nao foi possivel salvar o score");

    fprintf(score_file, "%d\n", highscores->size);
    for (i=0; i < highscores->size; i++)    // registra os scores
    {
        fprintf(score_file, "%s;%d\n", highscores->scores[i].datetime, highscores->scores[i].time);
    }

    fclose(score_file);
}

void _spritesDeinit(SPRITES *game_sprites)
// destroi os sprites e libera o espaco de memoria
{
    int i;
    for (i=0; i < DIRT_FRAMES; i++)
        al_destroy_bitmap(game_sprites->dirt[i]);
    for (i=0; i < OUT_WALL_FRAMES; i++)
        al_destroy_bitmap(game_sprites->outer_walls[i]);
    for (i=0; i < IN_WALL_FRAMES; i++)
        al_destroy_bitmap(game_sprites->inner_walls[i]);
    for (i=0; i < BOULDER_FRAMES; i++)
        al_destroy_bitmap(game_sprites->boulders[i]);
    for (i=0; i < JEWELS_FRAMES; i++)
        al_destroy_bitmap(game_sprites->jewels[i]);
    for (i=0; i < MINER_FRAMES; i++)
        al_destroy_bitmap(game_sprites->miner[i]);

    al_destroy_bitmap(game_sprites->open_door);
    al_destroy_bitmap(game_sprites->close_door);
    free(game_sprites);
}

void _audioDeinit(SAMPLES *game_samples)
// destroi os arquivos de audio e libera o espaco de memoria
{
    al_destroy_audio_stream(game_samples->bakcground_music);
    al_destroy_sample(game_samples->miner_death);
    al_destroy_sample(game_samples->open_door);
    al_destroy_sample(game_samples->pickup_jewel);
    al_destroy_sample(game_samples->power_up);
    al_destroy_sample(game_samples->win_game);
    free(game_samples);
}

void _scoreboardDeinit(SCOREBOARD *highscore)
// libera o espaco de memoria da scoreboard
{
    free(highscore->scores);
    free(highscore);
}

void _boardDeinit(BOARD *entities)
// libera o espaco de memoria da struct das entidades
{
    free(entities->map);
    free(entities);
}

void _entitiesDeinit(BOARD *entities)
// libera o espaco de memoria de cada entidade
{
    int i;

    for (i=0; i<entities->bould_num; i++)
        free(entities->boulders[i]);
    free(entities->boulders);
    for (i=0; i<entities->dirt_num; i++)
        free(entities->floor[i]);
    free(entities->floor);
    for (i=0; i<entities->jew_num; i++)
        free(entities->jewels[i]);
    free(entities->jewels);
    for (i=0; i<entities->out_wall_num; i++)
        free(entities->outer_walls[i]);
    free(entities->outer_walls);
    for (i=0; i<entities->inn_wall_num; i++)
        free(entities->inner_wall[i]);
    free(entities->inner_wall);

    free(entities->player);
    free(entities->exit_point);
}

void _hudDeinit(HUD *top_hud)
// libera o espaco de memoria da HUD
{
    al_destroy_font(top_hud->font);
    free(top_hud);
}

void _menuDeinit(MENU *pause_menu)
// libera o espaco de memoria do menu de pausa
{
    al_destroy_bitmap(pause_menu->info);
    free(pause_menu);
}

void _screenDeinit(SCREEN *scorescreen)
// libera o espaco de memoria da tela de score
{
    al_destroy_font(scorescreen->credits);
    al_destroy_font(scorescreen->regular);
    al_destroy_font(scorescreen->title);
    free(scorescreen);
}

void gameDeinit(GAME_ENV *env)
// Salva a scoreboard em disco e libera o espaço de memória utilizado. Executada antes de fechar o jogo.
{
    _saveScoreToFile(env->highscores);

    al_destroy_timer(env->timer);
    al_destroy_event_queue(env->queue);
    al_destroy_bitmap(env->buffer);
    al_destroy_display(env->disp);

    _scoreboardDeinit(env->highscores);
    _audioDeinit(env->game_samples);
    _spritesDeinit(env->game_sprites);
    _entitiesDeinit(env->entities);
    _boardDeinit(env->entities);
    _hudDeinit(env->top_hud);
    _menuDeinit(env->pause_menu);
    _screenDeinit(env->score_screen);

    free(env);
}