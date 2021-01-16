#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H

#include <gtk/gtk.h>
#include <math.h>
#include "gameWidget.h"
#include "communication.h"

typedef struct GameWindow GameWindow;
struct GameWindow{
    GtkWindow *window;
    Game *game;
    GtkWidget *keepTurnBtn;
    GtkWidget *labelScore;
    GtkWidget *surrenderBtn;
    GtkWidget *grid;
    GtkWidget *saveBtn;
    int handlePacketsTimerId;

    bool areWeKeepingTurn;
    PipesPtr pipePtr;
    bool isA;
};



GameWindow* gameWindowCreate(bool isA, PipesPtr pipePtr, GameLogic *gameLogic);

void gameWindowShow(GameWindow *gameWindow);

void gameWindowDestroy(GameWindow *gameWindow);

#endif