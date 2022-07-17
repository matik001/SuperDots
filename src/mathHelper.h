#ifndef HELPERS_H
#define HELPERS_H


typedef struct PointInt PointInt;
struct PointInt{
    int x;
    int y;
};
typedef struct PointDouble PointDouble;
struct PointDouble{
    double x;
    double y;
};

PointInt *pointIntCreate(int x, int y);
PointInt *pointDoubleCreate(double x, double y);

PointInt pointInt(int x, int y);
PointDouble pointDouble(double x, double y);
double distance2(PointDouble a, PointDouble b);

double min(double a, double b);
double max(double a, double b);
int abs(int a);
void swap(int *a, int *b);

#endif