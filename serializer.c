#include "serializer.h"
#include "gameLogic.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>



Serializer *serializerCreate(){
    Serializer *serializer = (Serializer*)malloc(sizeof(Serializer));
    serializer->s = (char*)malloc(MAX_BUFOR_SIZE);
    serializer->nextPos = 0;

    return serializer;
}

Serializer *serializerCreateFromStr(char *s){
    Serializer *serializer = (Serializer*)malloc(sizeof(Serializer));
    serializer->s = s;
    serializer->nextPos = 0;

    return serializer;
}

void serializerToFile(const char*filename, Serializer *serializer){
    FILE *file = fopen(filename, "w");
    for(int i = 0; i<serializer->nextPos; i++){
        putc(serializer->s[i], file);
    }
    fclose(file);
}

void serializerRewind(Serializer *serializer){
    serializer->nextPos = 0;
}


Serializer *serializerCreateFromFile(const char *filename){
    Serializer *res = serializerCreate();
    FILE *file = fopen(filename, "r");
    char c;
    int i = 0;
    while( EOF != (c = getc(file))){
        res->s[i++] = c;
    }
    fclose(file);
    res->s[i] = 0;
    
    return res;
}


void serializerDestroy(Serializer *serializer){
    free(serializer->s);
    free(serializer);
}

void serializeInt(Serializer *serializer, int val){
    char *next = serializer->s+serializer->nextPos;
    int len;
    sprintf(next, "%d %n", val, &len);
    serializer->nextPos+=len;
    next[len] = 0;
}

void serializeBool(Serializer *serializer, bool val){
    serializeInt(serializer, (int)(val==1) );
}

/// zwraca INT_MIN jeżeli deserializacja się nie powiodła
int deserializeInt(Serializer *serializer){
    char *next = serializer->s+serializer->nextPos;
    int len, val;
    if(*next == 0){ 
        return INT_MIN;
    }
    sscanf(next, "%d %n", &val, &len);
    serializer->nextPos+=len;
    
    if(len<=0){
        return INT_MIN;
    }

    return val;
}

bool deserializeBool(Serializer *serializer){
    return deserializeInt(serializer) == 1;
}

void serializeCustomVector(Serializer *serializer, Vector *vector, void(*serializeOne)(Serializer *serializer, void *obj)){
    int n = vectorSize(vector);
    serializeInt(serializer, n);

    for(int i = 0; i<n; i++){
        serializeOne(serializer, vectorGet(vector, i));
    }
}

Vector *deserializeCustomVector(Serializer *serializer, void*(*deserializeOne)(Serializer *serializer)){
    Vector *res = vectorCreate();
    int n = deserializeInt(serializer);

    for(int i = 0; i<n; i++){
        vectorPush(res, deserializeOne(serializer));
    }
    return res;
}

void serializePointInt(Serializer *serializer, PointInt *val){
    serializeInt(serializer, val->x);
    serializeInt(serializer, val->y);
}

PointInt *deserializePointInt(Serializer *serializer){
    int x = deserializeInt(serializer);
    int y = deserializeInt(serializer);
    return pointIntCreate(x, y);
}

void serializePointIntVoidPtr(Serializer *serializer, void *val){
    serializePointInt(serializer, (PointInt*)val);
}

void *deserializePointIntVoidPtr(Serializer *serializer){
    return (void*)deserializePointInt(serializer);
}

void serializeVectorPointInt(Serializer *serializer, Vector *points){
    serializeCustomVector(serializer, points, serializePointIntVoidPtr);
}

Vector *deserializeVectorPointInt(Serializer *serializer){
    return deserializeCustomVector(serializer, deserializePointIntVoidPtr);
}


void serializeGameLogic(Serializer *serializer, GameLogic *state){
    serializeInt(serializer, state->linesCnt);
    serializeBool(serializer, state->isPlayerATurn);
    serializeBool(serializer, state->didMakeMove);
    serializeInt(serializer, state->pointsA);
    serializeInt(serializer, state->pointsB);
    serializeInt(serializer, state->freeSpaces);
    
    /// zapisywanie kropek
    for(int i = 0; i<state->linesCnt; i++){
        for(int j = 0; j<state->linesCnt; j++){
            serializeBool(serializer, state->dots[i][j].exists);
            serializeBool(serializer, state->dots[i][j].isInsideBase);
            serializeBool(serializer, state->dots[i][j].belongsToA);
            serializeInt(serializer, state->dots[i][j].points);
        }
    }


    serializeInt(serializer, vectorSize(state->bases));
    /// zapisywanie baz
    for(int i = 0; i<vectorSize(state->bases); i++){
        Base * base = (Base*)vectorGet(state->bases, i);

        serializeInt(serializer, vectorSize(base->path));
        for(int j = 0; j<vectorSize(base->path); j++){
            Dot *dot = vectorGet(base->path, j);
            serializePointInt(serializer, &dot->p);
        }
        
        serializeInt(serializer, vectorSize(base->inside));
        for(int j = 0; j<vectorSize(base->inside); j++){
            Dot *dot = vectorGet(base->inside, j);
            serializePointInt(serializer, &dot->p);
        }
    }
}

GameLogic *deserializeGameLogic(Serializer *serializer){
    int linesCnt;
    bool isPlayerAOnMove;
    linesCnt = deserializeInt(serializer);
    isPlayerAOnMove = deserializeBool(serializer);

    GameLogic *state  = gameLogicCreate(linesCnt, isPlayerAOnMove);
    state->didMakeMove = deserializeBool(serializer);
    state->pointsA = deserializeInt(serializer);
    state->pointsB = deserializeInt(serializer);
    state->freeSpaces = deserializeInt(serializer);
    
    /// odczytywanie kropek
    for(int i = 0; i<state->linesCnt; i++){
        for(int j = 0; j<state->linesCnt; j++){
            Dot *dot = &state->dots[i][j];
            dot->exists = deserializeBool(serializer);
            dot->isInsideBase = deserializeBool(serializer);
            dot->belongsToA = deserializeBool(serializer);
            dot->points = deserializeInt(serializer);
            dot->p.x = i;
            dot->p.y = j;
            if(dot->exists){
                vectorPush(state->existingDots, dot);
            }
        }
    }

    int baseCnt = deserializeInt(serializer);
    /// odczytywanie baz
    for(int i = 0; i<baseCnt; i++){
        Base * base = baseCreate();

        int pathCnt = deserializeInt(serializer);
        for(int j = 0; j<pathCnt; j++){
            int x, y;
            x = deserializeInt(serializer);
            y = deserializeInt(serializer);
            Dot *dot = &state->dots[x][y];
            vectorPush(base->path, dot);
        }
        
        int insideCnt = deserializeInt(serializer);
        for(int j = 0; j<insideCnt; j++){
            int x, y;
            x = deserializeInt(serializer);
            y = deserializeInt(serializer);
            Dot *dot = &state->dots[x][y];
            vectorPush(base->inside, dot);
        }
        vectorPush(state->bases, base);
    }

    return state;
}

