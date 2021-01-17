#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "vector.h"
#include "mathHelper.h"
#include "queue.h"
#include <stdio.h>

typedef struct Dot Dot; 
struct Dot{
    /// Do kogo nalezy kropa
    bool belongsToA;

    /// Czy kropka zostala juz postawiona, 
    /// W przypadku podswietlenia myszka jest to dalej false
    bool exists;
    
    /// Pozycja kropki w tablicy
    /// Koszty trzymania tego sa mizerne a robi kod bardziej przejrzysty
    PointInt p; 

    /// Czy jest wewnatrz bazy
    bool isInsideBase;

    /// ile przeciwnik dostaje/lub nie punktow za te kropke
    int points;
};



typedef struct Base Base;
struct Base{
    /// Kolejne kropki otoczki bazy
    Vector *path;
    /// Kropki WEWNATRZ bazy (obu graczy)
    Vector *inside;
};

typedef struct GameLogic GameLogic;
struct GameLogic{
    /// ilosc lini na planszy
    int linesCnt;

    /// tablica dwuwymiarowa kropek
    Dot **dots;

    /// lista kropek juz postawionych (jest to optymalizacja, ale takze wygoda)
    Vector *existingDots;

    /// kto ma ruch, w rozumieniu czyja jest teraz tura
    bool isPlayerATurn;

    /// czy zrobil w tej turze ruch
    bool didMakeMove;

    /// lista baz
    Vector *bases;

    /// punkty graczy
    int pointsA;
    int pointsB;

    /// Odpowiednik gpointer z gtk
    void *onScoreChangedData;
    /// Event gdy zmienily sie punkty
    void (*onScoreChanged)(int playerA, int playerB, void* data);

    /// ilosc wolnych miejsc gdzie mozna postawic kropke
    int freeSpaces;
};


/// komentarze do funkcji w pliku .c

Base *baseCreate();
void baseDestroy(Base *base);
void baseDestroyVoidPtr(void *base);
GameLogic *gameLogicCreate(int linesCnt, bool isPlayerATurn);
void gameLogicDestroy(GameLogic *game);

bool gameLogicIsAnyOpponentsDotInsideBase(GameLogic *game, Vector *path);
void gameLogicAddBase(GameLogic *game, Vector *path);

bool gameLogicIsMoveLegal(GameLogic *game, PointInt p);
void gameLogicMakeMove(GameLogic *game, PointInt p);
void gameLogicEndTurn(GameLogic *game);
bool gameLogicIsPlayerATurn(GameLogic *game);
bool gameLogicIsMoveMadeInThisTurn(GameLogic *game);
void gameLogicOnScoreChanged(GameLogic *game, void (*handler)(int playerA, int playerB, void* data), void *data);

#endif