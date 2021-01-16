#include "communication.h"
#include "mathHelper.h"
#include "fifo.h"
#include "serializer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>


bool communicationInit(PipesPtr *pipePtr, int argc, char *argv[]){
    return (*pipePtr=initPipes(argc,argv)) != NULL;
}

void communicationClose(PipesPtr *pipePtr){
    if(pipePtr != NULL){
        closePipes(*pipePtr);
        *pipePtr = NULL;
    }
}

void communicationSendMovePacket(PipesPtr pipePtr, PointInt point){
    char s[MAX_PACKET_SIZE];
    sprintf(s, "%d %d %d ", MovePacketType, point.x, point.y);
    
    sendStringToPipe(pipePtr, s);
}

void communicationSendBasePathPacket(PipesPtr pipePtr, Vector *path){
    Serializer *serializer = serializerCreate();

    serializeInt(serializer, BasePathPacketType);
    serializeVectorPointInt(serializer, path);
    
    sendStringToPipe(pipePtr, serializer->s);

    serializerDestroy(serializer);
}

void communicationSendEndOfTurnPacket(PipesPtr pipePtr){
    char s[MAX_PACKET_SIZE];
    sprintf(s, "%d ", EndOfTurnPacketType);
    sendStringToPipe(pipePtr, s);
}

void communicationSendSurrenderPacket(PipesPtr pipePtr){
    char s[MAX_PACKET_SIZE];
    sprintf(s, "%d ", SurrenderPacketType);
    sendStringToPipe(pipePtr, s);
}

void communicationSendGameStatePacket(PipesPtr pipePtr, GameLogic *gameState, bool areWePlayerA){
    Serializer *serializer = serializerCreate();

    serializeInt(serializer, SavedGameStatePacketType);
    serializeGameLogic(serializer, gameState, areWePlayerA);
    
    sendStringToPipe(pipePtr, serializer->s);

    serializerDestroy(serializer);
}

static Vector *loadBasePathPacketData(Serializer *serializer);
static PointInt *loadMovePacketData(Serializer *serializer);
static Packet* packetCreate(PacketType type, void *data);
static GameLogic *loadSavedGameState(Serializer *serializer, bool areWePlayerA);


Vector *communicationReceivePackets(PipesPtr pipePtr, bool areWePlayerA){
    Serializer *serializer = serializerCreate();

    Vector *res = vectorCreate();
    if (!getStringFromPipe(pipePtr,serializer->s,MAX_PACKET_SIZE)){
        serializerDestroy(serializer);
        return res;
    } 
    
    while(true){
        int packetType = deserializeInt(serializer);

        if(packetType == INT_MIN)
            break;

        void *data = NULL;
        if(packetType == MovePacketType)
            data = loadMovePacketData(serializer);
        if(packetType == BasePathPacketType)
            data = loadBasePathPacketData(serializer);
        if(packetType == SavedGameStatePacketType)
            data = loadSavedGameState(serializer, areWePlayerA);

        vectorPush(res, packetCreate(packetType, data));
    }

    serializerDestroy(serializer);
    return res;
}

static Packet* packetCreate(PacketType type, void *data){
    Packet *packet = (Packet*)malloc(sizeof(Packet));
    packet->type = type;
    packet->data = data;
    return packet;
}

void packetDestroy(Packet *packet){
    if(packet->type == MovePacketType)
        free(packet->data);
    if(packet->type == BasePathPacketType)
        vectorDestroyWithItems((Vector*)packet->data, free);
    if(packet->type == SavedGameStatePacketType){
        /// nie niszczymy gamelogic bo bedziemy go wykorzystywac
    }
    free(packet);
}

void packetDestroyVoidPtr(void *packet){
    packetDestroy((Packet*)packet);
}

static PointInt *loadMovePacketData(Serializer *serializer){
    return deserializePointInt(serializer);
}

/// zrobic destroy na elementach zwroconych
static Vector *loadBasePathPacketData(Serializer *serializer){
    return deserializeVectorPointInt(serializer); 
}

static GameLogic *loadSavedGameState(Serializer *serializer, bool areWePlayerA){
    return deserializeGameLogic(serializer, areWePlayerA);
}
