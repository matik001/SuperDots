#include "mathHelper.h"
#include <stdlib.h>

PointInt *pointIntCreate(int x, int y){
    PointInt *p = (PointInt*)malloc(sizeof(PointInt));
    p->x = x;
    p->y = y;
    return p;
}

PointInt *pointDoubleCreate(double x, double y){
    PointInt *p = (PointInt*)malloc(sizeof(PointDouble));
    p->x = x;
    p->y = y;
    return p;
}

PointInt pointInt(int x, int y){
    PointInt p;
    p.x = x;
    p.y = y;
    return p;
}

PointDouble pointDouble(double x, double y){
    PointDouble res;
    res.x = x;
    res.y = y;
    return res;
}
double distance2(PointDouble a, PointDouble b){
    double dx = (a.x-b.x);
    double dy = (a.y-b.y);
    
    return dx*dx+dy*dy;   
}

double min(double a, double b){
    return a<b?a:b;
}
double max(double a, double b){
    return a<b?b:a;
}
int abs(int a){
    return a<0 ? -a : a;
}
void swap(int *a, int *b){
    int c = *a;
    *a = *b;
    *b = c;
}