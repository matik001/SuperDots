#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "vector.h"
#include "mathHelper.h"
#include "gameLogic.h"


#define MAX_BUFOR_SIZE 100000

typedef struct Serializer Serializer;
struct Serializer{
    char *s;
    int nextPos;
};

Serializer *serializerCreate();

Serializer *deserializerCreate(const char *s); /// s musi byc zaalokowane dynamicznie, bo przy destroy jest czyszczone

Serializer *serializerCreateFromFile(const char*filename);

void serializerRewind(Serializer *serializer);

void serializerDestroy(Serializer *serializer);

void serializeInt(Serializer *serializer, int val);

void serializeBool(Serializer *serializer, bool val);

int deserializeInt(Serializer *serializer);

bool deserializeBool(Serializer *serializer);

void serializeCustomVector(Serializer *serializer, Vector *vector, void(*serializeOne)(Serializer *serializer, void *obj));

Vector *deserializeCustomVector(Serializer *serializer, void*(*deserializeOne)(Serializer *serializer));

void serializePointInt(Serializer *serializer, PointInt *val);

PointInt *deserializePointInt(Serializer *serializer);

void serializePointIntVoidPtr(Serializer *serializer, void *val);

void serializeVectorPointInt(Serializer *serializer, Vector *points);

Vector *deserializeVectorPointInt(Serializer *serializer);

void serializeGameLogic(Serializer *serializer, GameLogic *state);

GameLogic *deserializeGameLogic(Serializer *serializer);

void serializerToFile(const char*filename, Serializer *serializer);




#endif