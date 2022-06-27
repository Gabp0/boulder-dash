// Biblioteca "game-states.h"
// Maquina de estados para controlar o fluxo do jogo "boulder dash"
// Feito por Gabriel de Oliveira Pontarolo
#include "boulder-dash.h"

typedef enum 
// possiveis estados da maquina de estados
{
    INICIO, JOGANDO, VITORIA, FECHAJOGO
} state;

state gameInit(GAME_ENV **env);   // Carregamento inicial do jogo
state gamePlay(GAME_ENV **env);   // Inicia o jogo
state gameWin(GAME_ENV **env);    // Tela de score
void gameClose(GAME_ENV **env);   // Fecha o jogo
