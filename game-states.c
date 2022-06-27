// Implementacao das funcoes da biblioteca "game-states.h"
// Feito por Gabriel de Oliveira Pontarolo
#include "game-states.h"
#include "boulder-dash.h"
#include <stdlib.h>
#include <stdio.h>

state gameInit(GAME_ENV **env)
// Carregamento inicial do jogo
{
    *env = loadGame();

    return JOGANDO;
}

state gamePlay(GAME_ENV **env)
// Inicia o jogo
{
    int done;

    initGame(*env);
    while (true)
    {   
        done = updateGameState(*env);

        switch (done)
        {
        case IN_GAME:
            break;

        case GAME_OVER:
            return JOGANDO;
        
        case CLOSE:
            return FECHAJOGO;

        case WIN:
            return VITORIA;

        default:
            break;
        }

        drawBoard(*env);
    }
}

state gameWin(GAME_ENV **env)
// Tela de score
{
    int done;

    while(true)
    {
        done = winScreen(*env);
        switch (done)
        {
        case PLAY_AGAIN:
            return JOGANDO;
            break;

        case CLOSE:
            return FECHAJOGO;
            break;
        
        default:
            break;
        }
    }
}

void gameClose(GAME_ENV **env)
// Fecha o jogo
{
    gameDeinit(*env);
    exit(EXIT_SUCCESS);
}