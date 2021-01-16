#ifndef VECTOR_H
#define VECTOR_H


typedef struct Vector Vector;
struct Vector{
    void **arr;
    int size;
    int allocated;
};


Vector *vectorCreate();

void vectorClear(Vector *vector);

void vectorPush(Vector *vector, void *val);

int vectorSize(Vector *vector);

void *vectorGet(Vector *vector, int i);

/// usuwa z konca
/// zakladam ze tych operacji bedzie niewiele, wiec nie zmniejszam tablicy
void *vectorPop(Vector *vector);
void vectorDestroy(Vector *vector);
void vectorDestroyWithItems(Vector *vector, void (*destroyItem)(void*));

#endif