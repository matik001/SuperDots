#ifndef MENU_WINDOW_H
#define MENU_WINDOW_H

#include <gtk/gtk.h>
#include <math.h>
#include "gameWidget.h"
#include "communication.h"

typedef struct MenuWindow MenuWindow;
struct MenuWindow{
    GtkWindow *window;
    GtkWidget *grid;
    GtkWidget *newGameBtn;
    GtkWidget *loadGameBtn;
    int handlePacketsTimerId;
    GameLogic *gameState;

    PipesPtr pipePtr;
    bool isA;
};


MenuWindow* menuWindowCreate(bool isA, PipesPtr pipePtr);

void menuWindowShow(MenuWindow *menuWindow);

void menuWindowDestroy(MenuWindow *menuWindow);

#endif