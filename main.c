// PROGRAMACAO II - Trabalho II: Jogo Grafico (Boulder Dash)
// Feito por Gabriel de Oliveira Pontarolo      GRR20203895
// Programa principal
#include <stdlib.h>
#include "boulder-dash.h"
#include "game-states.h"

int main (void)
{
    state current = INICIO;
    GAME_ENV *env;

    for (;;)
    {   // maquina de estados para controlar o fluxo do jogo
        switch (current)
        {
            case INICIO: 
                current = gameInit(&env);
                break;

            case JOGANDO: 
                current = gamePlay(&env);
                break;

            case VITORIA:
                current = gameWin(&env);
                break;
            
            case FECHAJOGO: 
                gameClose(&env); 
                break;
            
            default: 
                break;
        }
    }
}