#ifndef GAME_WIDGET_H
#define GAME_WIDGET_H

#include <gtk/gtk.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "vector.h"
#include "mathHelper.h"
#include "gameLogic.h"

#define GRID_LINE_WIDTH 0.8

/// margin jest konieczny do prawidlowego wyswietlania, poniewaz jezeli siatka,
/// zajmie cala przestrzen to w krawedziach nie narysuje sie pelna kropka
/// oraz DOT_CLICK_RADIUS bedzie dzialalo od mniejszej odleglosci
#define MARGIN 20.0
#define DOT_RADIUS 10.0
#define DOT_CLICK_RADIUS 16.0
#define COLOR_MY_DOTS 1,0,0,1
#define COLOR_OPPONENTS_DOTS 0,0,1,1

/// kolor mojej kropki ktora ma zostac dodana do sciezki 
#define COLOR_HOVER_NEXT_IN_PATH 0,0.6,0,1
#define COLOR_HOVERED_NEW 1,110.0/255,51.0/255, 1
#define COLOR_HOVERED_NEW_INVERTED 0,110.0/255,1, 1
#define COLOR_MY_BASE_BG 1,0,0,0.2
#define COLOR_MY_BASE_PATH 1,0,0,0.8
#define COLOR_OPPONTENTS_BASE_BG 0,0,1,0.2
#define COLOR_OPPONTENTS_BASE_PATH 0,0,1,0.8
#define COLOR_ACTIVE_BASE_PATH 0,0.4,0,1
#define COLOR_ACTIVE_BASE_BG 0,0.7,0,0.2
#define COLOR_ACTIVE_BASE_DOTS 0,0.4,0,1
#define COLOR_ACTIVE_BASE_DOTS_TO_BE_REMOVED 1,0.7,0.7,1
#define COLOR_ACTIVE_BASE_PATH_TO_BE_REMOVED 1,0.9,0.9,1
#define BASE_LINE_WIDTH 3

typedef struct Dot Dot;

typedef struct Game Game;
struct Game{
    /// Kontrolka na ktorej bede wszystko rysowal
    GtkWidget *drawingArea;

    /// Obiekt odpowiedzialny za trzymanie stanu gry
    GameLogic *gameLogic;

    int width;
    int height;

    /// Kropka w poblizu ktorej jest myszka
    Dot *hoverDot;

    /// Kropki otoczki tworzonej bazy
    /// GameLogic jes odpowiedzialne za logike gry, jezeli tworzy sie baze to wysyla mu sie punkty ja tworzace,
    /// Trzymanie aktywnie budowanej bazy postanowilem trzymac w tym module
    Vector *activeBaseDots;

    /// czy odwrotnie rysowac kolory (moje jego - jego moje)
    bool invertColors;

    /// Odpowiednik gpointer z gtk
    void *onMoveMadeData;
    /// Event gdy zrobiono ruch
    void (*onMoveMade)(PointInt p, void* data);

    /// Odpowiednik gpointer z gtk
    void *onBaseCreatedData;
    /// Event gdy zrobiono baze
    void (*onBaseCreated)(Vector *pathPoints, void* data);


    /// Odpowiednik gpointer z gtk
    void *onScoreChangedData;
    /// Event gdy zmienily sie punkty
    void (*onScoreChanged)(int mine, int his, void* data);

    /// Odpowiednik gpointer z gtk
    void *onEndOfGameData;
    /// Event gdy koniec gry
    void (*onEndOfGame)(int mine, int his, void* data);
};


Game *gameCreate(int width, int height, int linesCnt, bool isOurMove);
Game *gameCreateFromGameLogic(int width, int height, GameLogic *gameLogic, bool areWeRed);
void gameDestroy(Game *game);
void gameReset(Game *game, bool isOurMove);

void gameMakeMove(Game* game, PointInt *p);
void gameMakeBase(Game* game, Vector *pathPoints);
void gameChangeTurn(Game *game);

void gameOnMoveMade(Game *game, void (*handler)(PointInt p, void* data), void *data);
void gameOnBaseCreated(Game *game, void (*handler)(Vector *pathPoints, void* data), void *data);
void gameOnScoreChanged(Game *game, void (*handler)(int mine, int his, void* data), void *data);
void gameOnEndOfGame(Game *game, void (*handler)(int mine, int his, void* data), void *data);

bool gameIsMyTurn(Game *game);



#endif