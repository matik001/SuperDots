#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "vector.h"
#include "mathHelper.h"
#include "fifo.h"
#include "gameLogic.h"
#include <stdbool.h>

#define MAX_PACKET_SIZE 100000




typedef enum PacketType PacketType;
enum PacketType{
    MovePacketType,
    BasePathPacketType,
    EndOfTurnPacketType,
    SurrenderPacketType,
    SavedGameStatePacketType,
};
typedef struct Packet Packet;
struct Packet{
    PacketType type;
    void *data;
};

/// zwraca true jezeli wszystko ok
bool communicationInit(PipesPtr *pipePtr, int argc, char *argv[]);

void communicationClose(PipesPtr *pipePtr);

/// wysyla informacje gdzie postawiono kropke
void communicationSendMovePacket(PipesPtr pipePtr, PointInt point);

/// wysyla informacje o nowo utworzonej bazie
/// przyjmuje liste pozycji kropek
void communicationSendBasePathPacket(PipesPtr pipePtr, Vector *pathPoints);

/// wysyla informacje ze zakonczono ture
void communicationSendEndOfTurnPacket(PipesPtr pipePtr);

/// wysyla inforamcje ze sie poddano
void communicationSendSurrenderPacket(PipesPtr pipePtr);

/// wysyla dane gry ktora przeciwnik ma zaladowac
void communicationSendGameStatePacket(PipesPtr pipePtr, GameLogic *gameState, bool areWePlayerA);


/// odbiera liste pakietow
/// !!! zrobic destroy na kazdym pakiecie
Vector *communicationReceivePackets(PipesPtr pipePtr, bool areWePlayerA);


/// zwalnia miejsce po pakiecie
void packetDestroy(Packet *packet);

/// zwalnia miejsce po pakiecie
/// funkcja pomocnicza dla vectora
void packetDestroyVoidPtr(void *packet);



#endif