#include <gtk/gtk.h>
#include <math.h>
#include "communication.h"
#include "gameWindow.h"
#include "menuWindow.h"
#include "serializer.h"




/// zeby nie robic zmiennej globalnej komunikat nie bedzie powiazany z oknem
/// ma to nawet sens, bo inicjalizuje pipe zanim okno jest tworzone
void showError(char *msg){
    GtkWidget *dialog;
    dialog=gtk_message_dialog_new (NULL,GTK_DIALOG_MODAL,
				   GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"%s",msg);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}





/// zapisanie gry, wczytanie gry, poddanie i zaczecie od nowa
int main(int argc, char *argv[]) {
    PipesPtr pipePtr;
    gtk_init(&argc, &argv);
    if (!communicationInit(&pipePtr, argc, argv))
        return 1;

    bool isA = (argc == 2 && strcmp(argv[1],"A") == 0);

    

    MenuWindow *menuWindow = menuWindowCreate(isA, pipePtr);
    menuWindowShow(menuWindow);
    GameLogic *gameState = menuWindow->gameState;
    menuWindowDestroy(menuWindow);
    if(gameState == NULL){ /// zamknieto okno bez wybrania rodzaju gry
        return 0;
    }

    communicationSendGameStatePacket(pipePtr, gameState, isA);

    GameWindow *gameWindow = gameWindowCreate(isA, pipePtr, gameState);
    gameWindowShow(gameWindow);

    gameWindowDestroy(gameWindow);
    communicationClose(&pipePtr);


    return 0;
}
