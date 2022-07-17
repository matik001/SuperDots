#include "vector.h"
#include <stdlib.h>


static void _realloc(Vector *vector, int n){
    void **newArr = (void**)malloc(n*sizeof(void*));
    for(int i = 0; i<vector->size; i++)
        newArr[i] = vector->arr[i];
    free(vector->arr);
    vector->arr = newArr;
    vector->allocated = n;
}

Vector *vectorCreate(){
    Vector *vector = (Vector*)malloc(sizeof(Vector));
    vector->size = 0;
    vector->allocated = 2;
    vector->arr = (void**)malloc(vector->allocated * sizeof(void*));
    return vector;
}

void vectorClear(Vector *vector){
    free(vector->arr);
    vector->allocated = 2;
    vector->size = 0;
    vector->arr = (void**)malloc(vector->allocated * sizeof(void*));
}

void vectorPush(Vector *vector, void *val){
    if(vector->size+1 > vector->allocated)
        _realloc(vector, vector->allocated*2);
    
    vector->arr[vector->size++] = val;
}

int vectorSize(Vector *vector){
    return vector->size;    
}

void *vectorGet(Vector *vector, int i){
    return vector->arr[i];
}

/// zakladam ze tych operacji bedzie niewiele, wiec nie zmniejszam tablicy
void *vectorPop(Vector *vector){
    return vector->arr[--vector->size];
}
void vectorDestroy(Vector *vector){
    free(vector->arr);
    free(vector);
}
void vectorDestroyWithItems(Vector *vector, void (*destroyItem)(void*)){
    for(int i = 0; i<vector->size; i++){
        destroyItem(vector->arr[i]);
    }
    free(vector->arr);
    free(vector);
}
