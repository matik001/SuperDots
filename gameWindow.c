#include "gameWindow.h"
#include <gtk/gtk.h>
#include <math.h>
#include "gtkHelper.h"
#include "gameWidget.h"
#include "serializer.h"
#include "communication.h"

static void init(GameWindow *obj, bool isA, PipesPtr pipePtr, GameLogic *gameLogic);
static void updateKeepTurnBtn(GameWindow *gameWindow);


GameWindow* gameWindowCreate(bool isA, PipesPtr pipePtr, GameLogic *gameLogic){
    GameWindow *obj = (GameWindow*)malloc(sizeof(GameWindow));
    init(obj, isA, pipePtr, gameLogic);
    return obj;
}

void gameWindowShow(GameWindow *gameWindow){
    gtk_widget_show_all(GTK_WIDGET(gameWindow->window));
    updateKeepTurnBtn(gameWindow);
    gtk_main();
}

void gameWindowDestroy(GameWindow *gameWindow){
    g_source_remove(gameWindow->handlePacketsTimerId);
    gameDestroy(gameWindow->game);
    free(gameWindow);
}



/// jezeli zamknieto okno
static void windowClosedHandler(GtkWidget *widget, gpointer data){
    GameWindow *state = (GameWindow*)data;

    communicationSendSurrenderPacket(state->pipePtr);
    gtk_main_quit();
}


/// jezeli zakonczyla sie gra
static void endOfGameHandler(int mine, int his, void*data){
    GameWindow *state = (GameWindow*)data;
    
    char msg[50] = "Koniec gry!\n";
    strcat(msg, mine==his?"Remis":(mine<his ? "Porażka" : "Wygrana") );
    showMessage(msg, state->window);

    gameReset(state->game);
    state->areWeKeepingTurn = false;
    updateKeepTurnBtn(state);
}

/// kliknieto przyisk poddania sie
static void surrenderButtonClicked(GtkButton *btn, gpointer data){
    GameWindow *state = (GameWindow*)data;
    communicationSendSurrenderPacket(state->pipePtr); 
    endOfGameHandler(0, 1, state);
}

static void changeTurn(GameWindow *state){
    bool toMine = !gameIsMyTurn(state->game);

    if(toMine){
        gameChangeTurn(state->game);
    }
    else{
        communicationSendEndOfTurnPacket(state->pipePtr);
        gameChangeTurn(state->game); 
    }

    updateKeepTurnBtn(state);
}

/// jezeli przelaczono przycisk trzymania ruchu
static void keepTurnToggledHandler (GtkButton *btn, gpointer data){
    GameWindow *state = (GameWindow*)data;
    
    state->areWeKeepingTurn = !state->areWeKeepingTurn;

    if(!state->areWeKeepingTurn){
        changeTurn(state);
    }
    else{
        updateKeepTurnBtn(state);
    }
}

/// odbiera pakiety i je obsluguje
static gboolean handlePackets(gpointer data){
    GameWindow *state = (GameWindow*)data;
    Vector* packets = communicationReceivePackets(state->pipePtr);
    for(int i = 0; i<vectorSize(packets); i++){
        Packet *packet = vectorGet(packets, i);
        if(packet->type == MovePacketType){
            gameMakeMove(state->game, packet->data);
        }
        else if(packet->type == BasePathPacketType){
            gameMakeBase(state->game, (Vector*)packet->data);
        }
        else if(packet->type == EndOfTurnPacketType){
            changeTurn(state);
        }
        else if(packet->type == SurrenderPacketType){
            endOfGameHandler(1, 0, state);
        }

    }
    vectorDestroyWithItems(packets, packetDestroyVoidPtr);
    return TRUE;
}

/// jezeli zrobilismy ruch
static void moveMadeHandler(PointInt point, void *data){
    GameWindow *state = (GameWindow*)data;
    communicationSendMovePacket(state->pipePtr, point);

    if(!state->areWeKeepingTurn){
        changeTurn(state);
    }
}

static void setScoreText(GtkWidget *labelScore, int mine, int his, bool isMineRed){
    char *myColor = (isMineRed ? "red" : "blue");  
    char *hisColor = (isMineRed ? "blue" : "red");  

    char format[] = "<span foreground=\"%s\" size=\"28000\"><b>%d</b></span>" 
                    "<span foreground=\"black\" size=\"28000\"><b> - </b></span>"
                    "<span foreground=\"%s\" size=\"28000\"><b>%d</b></span>";
    char markup[200];
    sprintf (markup, format, myColor, mine, hisColor, his);
    gtk_label_set_markup (GTK_LABEL (labelScore), markup);
}


/// jezeli zmienil sie wynik punktowy
static void scoreChangedHandler(int mine, int his, void *data){
    GameWindow *state = (GameWindow*)data;
    setScoreText(state->labelScore, mine, his, state->isA);
}


/// jezeli stworzylismy baze
static void baseCreatedHandler(Vector *pathPoints, void *data){
    GameWindow *state = (GameWindow*)data;
    communicationSendBasePathPacket(state->pipePtr, pathPoints);
}

/// zapisuje stan gry
static void savedBtnClickedHandler(GtkWidget*widget, gpointer data){
    GameWindow *state = (GameWindow*)data;

    gchar *filename = pickFileToSave(state->window, "Wybierz gdzie zapisać grę");
    if(filename == NULL)
        return;

    Serializer *serializer = serializerCreate();
    serializeGameLogic(serializer, state->game->gameLogic);   
    serializerToFile(filename, serializer);
    serializerDestroy(serializer);

    g_free(filename);
}




static void init(GameWindow *obj, bool isA, PipesPtr pipePtr, GameLogic *gameLogic){
    obj->isA = isA;
    obj->pipePtr = pipePtr;
    obj->areWeKeepingTurn = gameLogicIsMoveMadeInThisTurn(gameLogic);
        // : (gameLogicIsMyTurn(gameLogic) && gameLogicIsMoveMadeInThisTurn(gameLogic));

    obj->window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_title(GTK_WINDOW(obj->window), "Gra w kropki by Mateusz Kisiel");


    g_signal_connect(obj->window, "destroy", G_CALLBACK(windowClosedHandler), obj);
    
    gtk_window_set_position(obj->window, GTK_WIN_POS_CENTER_ALWAYS);

    loadCss("styles.css");          

    GtkWidget *fixedLayout = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(obj->window), fixedLayout);

    gtk_widget_set_halign(fixedLayout, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(fixedLayout, GTK_ALIGN_CENTER);

    GdkPixbuf *savePixbuf = gdk_pixbuf_new_from_file_at_size("saveIcon.png",25, 25, NULL);
    GtkWidget *saveIcon = gtk_image_new_from_pixbuf(savePixbuf);
    obj->saveBtn = gtk_button_new();
    addClass(obj->saveBtn, "pureBtn");
    gtk_button_set_image(GTK_BUTTON(obj->saveBtn), saveIcon);
    g_signal_connect(obj->saveBtn, "clicked", G_CALLBACK(savedBtnClickedHandler), obj);
    gtk_fixed_put(GTK_FIXED(fixedLayout), obj->saveBtn, 15, 50);

    obj->grid = gtk_grid_new();
    gtk_fixed_put(GTK_FIXED(fixedLayout), obj->grid, 0, 0);

    gtk_container_set_border_width (GTK_CONTAINER (obj->grid), 40);

    gtk_grid_set_column_spacing(GTK_GRID(obj->grid), 20);
    gtk_grid_set_row_spacing(GTK_GRID(obj->grid), 6);
    gtk_grid_set_column_homogeneous (GTK_GRID(obj->grid), true);

    obj->labelScore = gtk_label_new (NULL);
    if(gameLogic == NULL)
        setScoreText(obj->labelScore, 0, 0, obj->isA);
    else 
        setScoreText(obj->labelScore, gameLogic->pointsA, gameLogic->pointsB, obj->isA);

    obj->surrenderBtn = gtk_button_new_with_label("Podaj się");
    g_signal_connect(obj->surrenderBtn, "clicked", G_CALLBACK(surrenderButtonClicked), obj);
    gtk_widget_set_margin_end(obj->surrenderBtn, 20);
    addClass(obj->surrenderBtn, "blackBtn");


    obj->keepTurnBtn = gtk_toggle_button_new_with_label("Nie oddawaj tury");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(obj->keepTurnBtn), obj->areWeKeepingTurn);
    g_signal_connect(obj->keepTurnBtn, "toggled", G_CALLBACK(keepTurnToggledHandler), obj);
    addClass(obj->keepTurnBtn, "toggleBtn");

    
    gtk_widget_set_margin_start(obj->keepTurnBtn, 20);
    obj->game = gameCreateFromGameLogic(800, 800, gameLogic, obj->isA);
    gameOnBaseCreated(obj->game, baseCreatedHandler, obj);
    gameOnMoveMade(obj->game, moveMadeHandler, obj);
    gameOnScoreChanged(obj->game, scoreChangedHandler, obj);
    gameOnEndOfGame(obj->game, endOfGameHandler, obj);



    gtk_grid_attach(GTK_GRID(obj->grid), obj->keepTurnBtn, 0,1,1,1);
    gtk_grid_attach(GTK_GRID(obj->grid), obj->labelScore, 1,1,1,1);
    gtk_grid_attach(GTK_GRID(obj->grid), obj->surrenderBtn, 2,1,1,1);
    gtk_grid_attach(GTK_GRID(obj->grid), obj->game->drawingArea, 0,0,3,1);

    obj->handlePacketsTimerId = g_timeout_add(100,handlePackets,obj);

}



/// Updatuje stan toggle buttona
static void updateKeepTurnBtn(GameWindow *gameWindow){
    if(!gameIsMyTurn(gameWindow->game))
        gtk_widget_hide(gameWindow->keepTurnBtn);
    else
        gtk_widget_show(gameWindow->keepTurnBtn);

    bool toggled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(gameWindow->keepTurnBtn));
    if(toggled != gameWindow->areWeKeepingTurn)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gameWindow->keepTurnBtn), gameWindow->areWeKeepingTurn);

    gtk_button_set_label(GTK_BUTTON(gameWindow->keepTurnBtn), gameWindow->areWeKeepingTurn? "Zakończ turę" : "Nie oddawaj tury");
}

