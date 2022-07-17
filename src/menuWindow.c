#include "menuWindow.h"
#include <gtk/gtk.h>
#include <math.h>
#include "gtkHelper.h"
#include "serializer.h"
#include "communication.h"

void addClass(GtkWidget *widget, const char* className);
static void init(MenuWindow *obj, bool isA, PipesPtr pipePtr);

MenuWindow* menuWindowCreate(bool isA, PipesPtr pipePtr){
    MenuWindow *obj = (MenuWindow*)malloc(sizeof(MenuWindow));
    init(obj, isA, pipePtr);
    return obj;
}

void menuWindowShow(MenuWindow *menuWindow){
    gtk_widget_show_all(GTK_WIDGET(menuWindow->window));
    gtk_main();
}

void menuWindowDestroy(MenuWindow *menuWindow){
    g_source_remove(menuWindow->handlePacketsTimerId);
    if(menuWindow->gameState != NULL) /// jezeli nie zamknieto to trzeba zamknac
        gtk_widget_hide(GTK_WIDGET(menuWindow->window));
    free(menuWindow);
}


/// jezeli zamknieto okno
static void windowClosedHandler(GtkWidget *widget, gpointer data){
    gtk_main_quit();
}


// /// kliknieto nowej gry
static void newGameButtonClicked(GtkButton *btn, gpointer data){
    MenuWindow *state = (MenuWindow*)data;
    state->gameState = gameLogicCreate(20, state->isA); 
    gtk_main_quit();
}

// /// kliknieto nowej gry
static void loadGameButtonClicked(GtkButton *btn, gpointer data){
    MenuWindow *state = (MenuWindow*)data;
    gchar *filename = pickFileToOpen(state->window, "Wybierz zapisaną grę");
    if(filename == NULL)
        return;
    Serializer *serializer = serializerCreateFromFile(filename);
    GameLogic *gameState = deserializeGameLogic(serializer);
    serializerDestroy(serializer);
    state->gameState = gameState;
    gtk_main_quit();
    g_free(filename);
}


/// odbiera pakiety i je obsluguje
static gboolean handlePackets(gpointer data){
    MenuWindow *state = (MenuWindow*)data;
    Vector* packets = communicationReceivePackets(state->pipePtr);
    for(int i = 0; i<vectorSize(packets); i++){
        Packet *packet = vectorGet(packets, i);
        if(packet->type == SavedGameStatePacketType){
            state->gameState = (GameLogic*)packet->data;
            gtk_main_quit();
        }
    }
    vectorDestroyWithItems(packets, packetDestroyVoidPtr);
    return TRUE;
}






static void init(MenuWindow *obj, bool isA, PipesPtr pipePtr){
    obj->isA = isA;
    obj->pipePtr = pipePtr;
    obj->gameState = NULL;

    obj->window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_title(GTK_WINDOW(obj->window), "Gra w kropki by Mateusz Kisiel");
    gtk_container_set_border_width (GTK_CONTAINER (obj->window), 50);


    g_signal_connect(obj->window, "destroy", G_CALLBACK(windowClosedHandler), obj);
    
    gtk_window_set_position(obj->window, GTK_WIN_POS_CENTER_ALWAYS);

    gtk_window_set_resizable(obj->window, FALSE);
    loadCss("styles.css");

    obj->grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(obj->window), obj->grid);
    gtk_widget_set_halign(obj->grid, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(obj->grid, GTK_ALIGN_CENTER);
    gtk_grid_set_row_spacing(GTK_GRID(obj->grid), 30);
    gtk_grid_set_row_homogeneous (GTK_GRID(obj->grid), true);

    obj->newGameBtn = gtk_button_new_with_label("Nowa gra");
    g_signal_connect(obj->newGameBtn, "clicked", G_CALLBACK(newGameButtonClicked), obj);
    addClass(obj->newGameBtn, "menuBtn");

    obj->loadGameBtn = gtk_button_new_with_label("Wczytaj grę");
    g_signal_connect(obj->loadGameBtn, "clicked", G_CALLBACK(loadGameButtonClicked), obj);
    addClass(obj->loadGameBtn, "menuBtn");


    gtk_grid_attach(GTK_GRID(obj->grid), obj->newGameBtn, 0,0,1,1);
    gtk_grid_attach(GTK_GRID(obj->grid), obj->loadGameBtn, 0,1,1,1);


    obj->handlePacketsTimerId = g_timeout_add(100,handlePackets,obj);
}
