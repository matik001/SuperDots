#ifndef FIFO_H
#define FIFO_H

#include <stdbool.h>
#include <stddef.h>

typedef struct pipes *PipesPtr;

PipesPtr initPipes(int argc, char *argv[]);
void     sendStringToPipe(PipesPtr channel, const char *data);
bool     getStringFromPipe(PipesPtr channel, char *buffer, size_t size);
void     closePipes(PipesPtr channel);

#endif