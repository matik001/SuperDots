#include "vector.h"
#include "mathHelper.h"
#include "queue.h"
#include "gameLogic.h"
#include "serializer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void dotInit(Dot *dot, PointInt p);
static void markConnected(GameLogic *game, bool *vis, PointInt p);

/// tworzy baze
Base *baseCreate(){
    Base *base = (Base*)malloc(sizeof(Base));
    base->path = vectorCreate();
    base->inside = vectorCreate();
    return base;
}

/// niszczy baze
void baseDestroy(Base *base){
    vectorDestroy(base->inside);
    vectorDestroy(base->path);
    free(base);
}

/// niszczy baze
/// konwertowanie funkcji jest niebezpieczne, wiec musialem stworzyc druga
void baseDestroyVoidPtr(void *base){
    baseDestroy((Base*)base);
}

/// tworzy nowa instancje gameLogic
GameLogic *gameLogicCreate(int linesCnt, bool isPlayerATurn){
    GameLogic *game = (GameLogic*)malloc(sizeof(GameLogic));
    game->isPlayerATurn = isPlayerATurn;
    game->didMakeMove = false;
    game->linesCnt = linesCnt;
    game->pointsA = 0;
    game->pointsB = 0;
    game->onScoreChanged = NULL;
    game->onScoreChangedData = NULL;
    game->freeSpaces = linesCnt*linesCnt;
    game->existingDots = vectorCreate();
    game->bases = vectorCreate();

    game->dots = (Dot**)malloc(linesCnt * sizeof(Dot*));
    for(int i = 0; i<linesCnt; i++){
        game->dots[i] = (Dot*)malloc(linesCnt * sizeof(Dot));
        for(int j = 0; j<linesCnt; j++){
            dotInit(&game->dots[i][j], pointInt(i, j));
        }
    }

    return game;
}

/// niszczy instancje gameLogic
void gameLogicDestroy(GameLogic *game){
    for(int i = 0; i<game->linesCnt; i++)
        free(game->dots[i]);
    free(game->dots);
    
    vectorDestroy(game->existingDots);
    vectorDestroyWithItems(game->bases, baseDestroyVoidPtr);

    free(game);
}


/// zwraca true jezeli istnieje kropka przeciwnika w bazie zdefiniowanej przez path, 
/// ktora nie jest obecnie wewnatrz jakiejsc innej bazy
bool gameLogicIsAnyOpponentsDotInsideBase(GameLogic *game, Vector *path){
    bool isOutside[game->linesCnt][game->linesCnt];
    memset(isOutside, 0, sizeof(isOutside[0][0]) * game->linesCnt * game->linesCnt);

    for(int i = 0; i<vectorSize(path); i++){
        Dot* dot = vectorGet(path, i);
        isOutside[dot->p.x][dot->p.y] = true;
    }

    for(int i = 0; i<game->linesCnt; i++){
        markConnected(game, (bool*)isOutside, pointInt(i,0)); 
        markConnected(game, (bool*)isOutside, pointInt(i,game->linesCnt-1)); 
        markConnected(game, (bool*)isOutside, pointInt(0,i)); 
        markConnected(game, (bool*)isOutside, pointInt(game->linesCnt-1,i)); 
    }

    if(vectorSize(path) == 0)
        return false;

    bool baseOwner = ((Dot*)vectorGet(path, 0))->belongsToA;
    for(int i = 0; i<game->linesCnt; i++){
        for(int j = 0; j<game->linesCnt; j++){
            if(!isOutside[i][j]){
                Dot*dot = &game->dots[i][j];
                if(dot->exists
                    && dot->belongsToA != baseOwner
                    && !dot->isInsideBase)
                        return true;
            }
        }
    }

    return false;
}

/// dodaje nowa baze na podstawie listy punktow bedacych jej obwodem
void gameLogicAddBase(GameLogic *game, Vector *path){
    Base *base = baseCreate();
    for(int i = 0; i<vectorSize(path); i++){
        Dot *dot = vectorGet(path, i);
        vectorPush(base->path, dot);
    }

    bool isOutside[game->linesCnt][game->linesCnt];
    memset(isOutside, 0, sizeof(isOutside[0][0]) * game->linesCnt * game->linesCnt);

    for(int i = 0; i<vectorSize(path); i++){
        Dot* dot = vectorGet(path, i);
        isOutside[dot->p.x][dot->p.y] = true;
    }

    for(int i = 0; i<game->linesCnt; i++){
        /// w tej funkcji sprawdzam czy juz bylo odwiedzone wiec nie musze tutaj
        markConnected(game, (bool*)isOutside, pointInt(i,0)); 
        markConnected(game, (bool*)isOutside, pointInt(i,game->linesCnt-1)); 
        markConnected(game, (bool*)isOutside, pointInt(0,i)); 
        markConnected(game, (bool*)isOutside, pointInt(game->linesCnt-1,i)); 
    }

    int deltaPoints1 = 0, deltaPoints2 = 0;

    bool baseOwner = ((Dot*)vectorGet(path, 0))->belongsToA;
    for(int i = 0; i<game->linesCnt; i++){
        for(int j = 0; j<game->linesCnt; j++){
            if(!isOutside[i][j]){
                if(game->dots[i][j].exists){
                    if(game->dots[i][j].belongsToA != baseOwner){
                        deltaPoints1 -= game->dots[i][j].points;
                        game->dots[i][j].points = 1;
                        deltaPoints1 += game->dots[i][j].points;
                    }
                    else{
                        deltaPoints2 -= game->dots[i][j].points;
                        game->dots[i][j].points = 0;
                        deltaPoints2 += game->dots[i][j].points;
                    }
                    vectorPush(base->inside, &game->dots[i][j]);
                }
                if(!game->dots[i][j].exists)
                    game->freeSpaces--;
                game->dots[i][j].isInsideBase = true; /// jezeli nie nalezy do nikogo tez zaznaczyc
            }
        }
    }

    vectorPush(game->bases, base);
    if(!baseOwner)
        swap(&deltaPoints1, &deltaPoints2);
    game->pointsA += deltaPoints1;
    game->pointsB += deltaPoints2;
    if( (deltaPoints1 != 0 || deltaPoints2 != 0) && game->onScoreChanged != NULL){
        game->onScoreChanged(game->pointsA, game->pointsB, game->onScoreChangedData);
    }
}

/// sprawdza czy ruch jest mozliwy
bool gameLogicIsMoveLegal(GameLogic *game, PointInt p){
    Dot *dot = &game->dots[p.x][p.y];

    return !dot->exists && !dot->isInsideBase && !game->didMakeMove;
}

/// funkcja wykonujaca ruch
void gameLogicMakeMove(GameLogic *game, PointInt p){
    Dot *dot = &game->dots[p.x][p.y];

    dot->exists = true;
    dot->belongsToA = game->isPlayerATurn;
    game->didMakeMove = true;
    game->freeSpaces--;
    vectorPush(game->existingDots, dot);
}

/// fukcja konczaca ture
void gameLogicEndTurn(GameLogic *game){
    game->isPlayerATurn = !game->isPlayerATurn;
    game->didMakeMove = false;
}

/// dodaje event zmiany wyniku
void gameLogicOnScoreChanged(GameLogic *game, void (*handler)(int playerA, 
                                int playerB, void* data), void *data){
    game->onScoreChangedData = data;
    game->onScoreChanged = handler;
}

/// zwraca czy obecnie jest nasza tura
bool gameLogicIsPlayerATurn(GameLogic *game){
    return game->isPlayerATurn;
}

/// zwraca czy obecnie jest nasza tura i ju?? zrobili??my ruch
bool gameLogicIsMoveMadeInThisTurn(GameLogic *game){
    return game->didMakeMove;
}

/// inicjalizuje kropke
static void dotInit(Dot *dot, PointInt p){
    dot->p.x = p.x;
    dot->p.y = p.y;
    dot->belongsToA = true;
    dot->exists = false;
    dot->isInsideBase = false;
    dot->points = 0;
}

/// sprawdza czy punkt(indeks) jest poprawny (nie wychodzi poza tablice)
static bool isPointValid(GameLogic *game, PointInt p){
    if(p.x < 0 || p.y<0)
        return false;
    if(p.x >= game->linesCnt || p.y >= game->linesCnt)
        return false;
    return true;
}


/// startuje z p i zaznacza wszedzie gdzie moze z tego punktu dojsc
static void markConnected(GameLogic *game, bool *vis, PointInt p){
    int n = game->linesCnt;
    
    Queue *queue = queueCreate();
    queuePush(queue, pointIntCreate(p.x, p.y));
    while(!queueIsEmpty(queue)){
        PointInt *pos = queuePop(queue);
        if(isPointValid(game, *pos) && !*(vis+pos->x*n+pos->y)){
            *(vis+pos->x*n+pos->y) = true;
            queuePush(queue, pointIntCreate(pos->x+1, pos->y));
            queuePush(queue, pointIntCreate(pos->x-1, pos->y));
            queuePush(queue, pointIntCreate(pos->x, pos->y+1));
            queuePush(queue, pointIntCreate(pos->x, pos->y-1));
        }
        free(pos);
    }

    free(queue); 
}

