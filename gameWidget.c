#include <gtk/gtk.h>
#include "gameWidget.h"
#include "gameLogic.h"
#include "mathHelper.h"



static void reRender(Game *game);
static Vector *vectorPointsToVectorDots(Game *game, Vector *points);
static gboolean gtkDrawCallback(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean mouseMovedEventHandler (GtkWidget *widget, GdkEventMotion *event, gpointer data);
static gboolean mousePressEventHandler(GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean mouseLeftEventHandler(GtkWidget *widget, GdkEventCrossing *event, gpointer data);
static void scoreChangedHandler(int playerA, int playerB, void* data);

/// tworzy nowy obiekt widgetu gry
Game *gameCreateFromGameLogic(int width, int height, GameLogic *gameLogic, bool areWePlayerA){
    Game *game = (Game*)malloc(sizeof(Game));

    game->gameLogic = gameLogic;
    game->areWePlayerA = areWePlayerA;
    game->hoverDot = NULL;
    game->width = width;
    game->height = height;
    game->activeBaseDots = vectorCreate();

    game->onBaseCreated = NULL;
    game->onBaseCreatedData = NULL;
    game->onMoveMade = NULL;
    game->onMoveMadeData = NULL;
    game->onScoreChangedData = NULL;
    game->onScoreChanged = NULL;
    game->onEndOfGame = NULL;
    game->onEndOfGameData = NULL;

    gameLogicOnScoreChanged(game->gameLogic, scoreChangedHandler, game);
    
    game->drawingArea = gtk_drawing_area_new();

    gtk_widget_set_size_request(game->drawingArea, width, height);
    g_signal_connect(G_OBJECT(game->drawingArea), "draw", G_CALLBACK(gtkDrawCallback), game);

    gtk_widget_set_events(GTK_WIDGET(game->drawingArea), GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_LEAVE_NOTIFY_MASK);
    g_signal_connect(G_OBJECT(game->drawingArea), "motion_notify_event", (GCallback)mouseMovedEventHandler, game);
    g_signal_connect(G_OBJECT(game->drawingArea), "button_press_event", (GCallback) mousePressEventHandler, game);
    g_signal_connect(G_OBJECT(game->drawingArea), "leave-notify-event", (GCallback) mouseLeftEventHandler, game);


    return game;
}

/// resetowanie gry
void gameReset(Game *game){
    int linesCnt = game->gameLogic->linesCnt;

    gameLogicDestroy(game->gameLogic);
    vectorClear(game->activeBaseDots);
    game->hoverDot = NULL;
    game->gameLogic = gameLogicCreate(linesCnt, true);
    gameLogicOnScoreChanged(game->gameLogic, scoreChangedHandler, game);
    scoreChangedHandler(0, 0, game);

    reRender(game);
}

/// niszczy widget
void gameDestroy(Game *game){
    gameLogicDestroy(game->gameLogic);
    vectorDestroy(game->activeBaseDots);

    free(game);
}

/// wykonuje ruch
/// moze za przeciwnika, ale moze tez za gracza
void gameMakeMove(Game* game, PointInt *p){
    gameLogicMakeMove(game->gameLogic, *p);
    reRender(game);
}

/// tworzy baze 
/// dziala dla obu graczy
/// przyjmuje liste punktow (nie kropek)
void gameMakeBase(Game* game, Vector *pathPoints){
    Vector *path = vectorPointsToVectorDots(game, pathPoints);
    gameLogicAddBase(game->gameLogic, path);
    reRender(game);
    
    vectorDestroy(path);
}

/// konczy ture
/// dziala dla obu graczy
void gameChangeTurn(Game *game){
    gameLogicEndTurn(game->gameLogic);
    vectorClear(game->activeBaseDots);

    bool endOfGame = game->gameLogic->freeSpaces == 0;
    if(endOfGame && game->onEndOfGame != NULL){
        int pointsA = game->gameLogic->pointsA;
        int pointsB = game->gameLogic->pointsB;
        if(!game->areWePlayerA)
            swap(&pointsA, &pointsB);
        game->onEndOfGame(pointsA, pointsB, game->onEndOfGameData);
    }
    /// konczymy dopiero po zakonczeniu tury bo moze chciec jeszcze zrobic jakies bazy
}

void gameOnMoveMade(Game *game, void (*handler)(PointInt p, void* data), void *data){
    game->onMoveMade = handler;
    game->onMoveMadeData = data;
}
void gameOnBaseCreated(Game *game, void (*handler)(Vector *pathPoints, void* data), void *data){
    game->onBaseCreated = handler;
    game->onBaseCreatedData = data;
}

/// dodaje event zmiany wyniku
void gameOnScoreChanged(Game *game, void (*handler)(int mine, 
                            int his, void* data), void *data){
    game->onScoreChangedData = data;
    game->onScoreChanged = handler;
}

/// dodaje event konca gry
void gameOnEndOfGame(Game *game, void (*handler)(int mine, 
                            int his, void* data), void *data){
    game->onEndOfGameData = data;
    game->onEndOfGame = handler;
    
}

/// zwraca czy jest nasza tura
bool gameIsMyTurn(Game *game){
    return gameLogicIsPlayerATurn(game->gameLogic) == game->areWePlayerA;
}

static void scoreChangedHandler(int playerA, int playerB, void* data){
    Game *game = (Game*)data;
    if(game->onScoreChanged != NULL){
        if(!game->areWePlayerA)
            swap(&playerA, &playerB);
        game->onScoreChanged(playerA, playerB, game->onScoreChangedData);
    }
}

/// zamienia liste punktow na liste kropek
static Vector *vectorPointsToVectorDots(Game *game, Vector *points){
    Vector *ret = vectorCreate();
    for(int i = 0; i<vectorSize(points); i++){
        PointInt *p = (PointInt*)vectorGet(points, i);
        vectorPush(ret, &game->gameLogic->dots[p->x][p->y]);
    }
    return ret;
}

/// zamienia liste kropek na liste punktow
static Vector *vectorDotsToVectorPoints(Game *game, Vector *dots){
    Vector *ret = vectorCreate();
    for(int i = 0; i<vectorSize(dots); i++){
        Dot *dot = (Dot*)vectorGet(dots, i);
        vectorPush(ret, pointIntCreate(dot->p.x, dot->p.y));
    }
    return ret;
}

/// zamienia indeks kropki na jego pozycje na ekranie
static PointDouble toPixelPoint(Game *game, PointInt p){
    PointDouble res;
    res.x = p.x*(game->width-2*MARGIN)/(double)(game->gameLogic->linesCnt-1) + MARGIN;
    res.y = p.y*(game->height-2*MARGIN)/(double)(game->gameLogic->linesCnt-1) + MARGIN;
    return res;
}

/// wykonuje redraw calego widgetu
static void reRender(Game *game){
    gtk_widget_queue_draw_area(game->drawingArea, 0,0, game->width, game->height);
}

/// zrezygnowalem z korzystania z tej funkcji, duzo dodatkowych przypadkow musialbym uwzgledniac,
/// a wydajnosc bylaby identyczna, bo renderowanie jest wykonywane rzadko
static void reRenderSingleDot(Game *game, PointInt p){
    const int RESERVE = 2;
    PointDouble pos = toPixelPoint(game, p);
    gtk_widget_queue_draw_area(game->drawingArea, pos.x-DOT_RADIUS-RESERVE,pos.y-DOT_RADIUS-RESERVE, 2*(DOT_RADIUS+RESERVE), 2*(DOT_RADIUS+RESERVE));
}

/// zamienia punkt na ekranie na indeks najblizszej kropki bedacej w odleglosci nie wiekszej niz d 
static PointInt *toArrPoint(Game *game, PointDouble pixelPoint, double d){
    int beforeX = (int)((pixelPoint.x - MARGIN)/((game->width-2*MARGIN)/(double)(game->gameLogic->linesCnt-1)));
    int beforeY = (int)((pixelPoint.y - MARGIN)/((game->height-2*MARGIN)/(double)(game->gameLogic->linesCnt-1)));
    PointInt t[] = {
        pointInt(beforeX, beforeY),
        pointInt(beforeX+1, beforeY),
        pointInt(beforeX, beforeY+1),
        pointInt(beforeX+1, beforeY+1)   
    };

    /// jezeli kilka punktow bedzie w zasiegu ma wziac najblizszy
    PointInt *res = NULL;
    double bestD = DBL_MAX;
    for(int i = 0; i<4; i++){
        if(t[i].x >= game->gameLogic->linesCnt || t[i].y >= game->gameLogic->linesCnt)
            continue;
        PointDouble p1 =  toPixelPoint(game, t[i]);
        
        double dist = distance2(pixelPoint, p1);
        if(dist <= d*d && dist < bestD){
            bestD = dist;
            res = pointIntCreate(t[i].x, t[i].y);
        }
    }
    return res;
}



/// sprawdza czy kroka jest w aktualnie tworzonej sciezce bazy
static bool isDotInActivePath(Game *game, Dot *dot){
    for(int i = 0; i < vectorSize(game->activeBaseDots); i++){
        if(vectorGet(game->activeBaseDots, i) == dot)
            return true;
    }

    return false;
}


/// zwraca true jezeli kropka jest nasza
static bool isDotMine(Game *game, Dot *dot){
    return dot->belongsToA == game->areWePlayerA;
}



/// sprawdza czy kropka moze byc dodana jako kolejna w aktualnie tworzonej sciezce bazy
static bool canBeAddedtoActiveBasePath(Game *game, Dot *dot){
    if(gameIsMyTurn(game)  
                    && dot->exists 
                    && isDotMine(game, dot) 
                    && !dot->isInsideBase){
        if(vectorSize(game->activeBaseDots)==0)
            return true;
    
        Dot *lastDot = (Dot*)vectorGet(game->activeBaseDots, vectorSize(game->activeBaseDots)-1);
        if(abs(lastDot->p.x - dot->p.x) <= 1
            && abs(lastDot->p.y - dot->p.y) <= 1
            && !isDotInActivePath(game, dot)){
            return true;
        }
    }
    return false;
}

/// sprawdza czy mozna stworzyc baze z aktualnej aktywnej
static bool canCreateBase(Game *game){
    if(gameIsMyTurn(game)
                && vectorSize(game->activeBaseDots) > 0){
        Dot *firstDot = (Dot*)vectorGet(game->activeBaseDots, 0);
        Dot *lastDot = (Dot*)vectorGet(game->activeBaseDots, vectorSize(game->activeBaseDots)-1);
        if(abs(lastDot->p.x - firstDot->p.x) <= 1
                && abs(lastDot->p.y - firstDot->p.y) <= 1
                && gameLogicIsAnyOpponentsDotInsideBase(game->gameLogic, game->activeBaseDots)){
            return true;
        }
    } 
    return false;
}


/// zwraca true jezeli moze stworzyc baze z obecnej aktywnej, a kursor myszki
/// jest na pierwszym elemencie sciezki
static bool isAboutToMakeBase(Game *game){
    bool isHoveringFirstDotInPath = vectorSize(game->activeBaseDots) > 0
            && game->hoverDot == vectorGet(game->activeBaseDots, 0);
    return isHoveringFirstDotInPath
            && canCreateBase(game);
}


/// jezeli kusor jest na kropce w aktywnej sciezce to zwracamy ktora to jest w kolejnosci od konca
/// w innym przypadku zwraca 0
/// !!! nie sprawdza czy da sie zrobic baze, zaklada ze sie nie da
static int howManyAreWeAboutToRemove(Game *game){
    if(gameIsMyTurn(game)){
        for(int i = 0; i<vectorSize(game->activeBaseDots); i++){
            if(vectorGet(game->activeBaseDots, i) == game->hoverDot)
                return vectorSize(game->activeBaseDots)-i;
        }
    }
    return 0;
}

/// rysuje siatke
static void drawGrid(Game *game, cairo_t *cr){
    cairo_set_line_width(cr, GRID_LINE_WIDTH);
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb (cr, 0, 0, 0);

    
    for(int i = 0; i<game->gameLogic->linesCnt; i++){
        PointDouble from = toPixelPoint(game, pointInt(i, 0)),
                    to   = toPixelPoint(game, pointInt(i, game->gameLogic->linesCnt-1));

        cairo_move_to (cr, from.x, from.y);
        cairo_line_to (cr, to.x, to.y);
        
    }
    for(int i = 0; i<game->gameLogic->linesCnt; i++){
        PointDouble from = toPixelPoint(game, pointInt(0, i)),
                    to   = toPixelPoint(game, pointInt(game->gameLogic->linesCnt-1, i));

        cairo_move_to (cr, from.x, from.y);
        cairo_line_to (cr, to.x, to.y);

    }
    cairo_stroke (cr);
}

/// rysuje kropki
/// jezeli drawInsideBase true to rysuje tylko kropki wewnatrz baz, 
/// jezeli false to rysuje tylko pozostale
/// chcemy by kropki byly na wierzchu lini baz, ale pod tlem bazy
/// howManyNotToShow - jak duzo kropek zostanie usunietych ze sciezki po kliknieciu myszka
static void drawDots(Game *game, cairo_t *cr, int howManyNotToShow, bool drawInsideBase){
    cairo_set_line_width(cr, 1);
    
    /// rysowanie nieaktywnych
    for(int i = 0; i < vectorSize(game->gameLogic->existingDots); i++){
        Dot *dot = (Dot*)vectorGet(game->gameLogic->existingDots, i);
        
        if(dot->isInsideBase != drawInsideBase)
            continue;

        if(isDotInActivePath(game, dot))
            continue;

        if(dot == game->hoverDot 
            && canBeAddedtoActiveBasePath(game, dot))
            cairo_set_source_rgba (cr, COLOR_HOVER_NEXT_IN_PATH);
        else{
            if(dot->belongsToA)
                cairo_set_source_rgba (cr, COLOR_A_DOTS);
            else
                cairo_set_source_rgba (cr, COLOR_B_DOTS);
        }
        PointDouble point = toPixelPoint(game, dot->p);
        cairo_arc(cr, point.x, point.y, DOT_RADIUS, 0, 2*G_PI);
        cairo_fill(cr);
    }
    if(!drawInsideBase){

        /// rysowanie aktywnych
        for(int i = 0; i<vectorSize(game->activeBaseDots); i++){
            Dot * dot = (Dot*)vectorGet(game->activeBaseDots, i);
            PointDouble point = toPixelPoint(game, dot->p);

            bool shouldShowAsPath = i<vectorSize(game->activeBaseDots)-howManyNotToShow;
            if(shouldShowAsPath)
                cairo_set_source_rgba (cr, COLOR_ACTIVE_BASE_DOTS);    
            else
                cairo_set_source_rgba(cr, COLOR_ACTIVE_BASE_DOTS_TO_BE_REMOVED);
            cairo_arc(cr, point.x, point.y, DOT_RADIUS, 0, 2*G_PI);
            cairo_fill(cr);
        }

        if(game->hoverDot != NULL 
                    && gameIsMyTurn(game)
                    && gameLogicIsMoveLegal(game->gameLogic, game->hoverDot->p)){
            if(game->areWePlayerA)
                cairo_set_source_rgba (cr, COLOR_HOVERED_NEW_A);
            else
                cairo_set_source_rgba (cr, COLOR_HOVERED_NEW_B);

            PointDouble point = toPixelPoint(game, game->hoverDot->p);
            cairo_arc(cr, point.x, point.y, DOT_RADIUS, 0, 2*G_PI);
            cairo_fill(cr);
        }
    }
}

/// rysuje bazy
static void drawBases(Game *game, cairo_t *cr){
    cairo_set_line_width(cr, BASE_LINE_WIDTH);
    
    for(int i = 0; i < vectorSize(game->gameLogic->bases); i++){
        Base *base = (Base*)vectorGet(game->gameLogic->bases, i);
        Dot *previousDot = (Dot*)vectorGet(base->path, vectorSize(base->path)-1);

        if(previousDot->belongsToA)
            cairo_set_source_rgba (cr, COLOR_A_BASE_PATH);
        else
            cairo_set_source_rgba (cr, COLOR_B_BASE_PATH);

        for(int j = 0; j < vectorSize(base->path); j++){
            Dot *dot = (Dot*)vectorGet(base->path, j);
            PointDouble from = toPixelPoint(game,previousDot->p),
                        to = toPixelPoint(game, dot->p);

            if(j == 0)
                cairo_move_to (cr, from.x, from.y);
            cairo_line_to (cr, to.x, to.y);

            previousDot = dot;
        }
        cairo_close_path(cr);
        cairo_stroke_preserve(cr);

        if(previousDot->belongsToA)
            cairo_set_source_rgba (cr, COLOR_A_BASE_BG);
        else
            cairo_set_source_rgba (cr, COLOR_B_BASE_BG);

        cairo_fill(cr);
    }
}

/// rysuje aktywna baze
/// shouldShowNewBase - czy powinno pokazac jak bedzie wygladala nowo utworzona baza
/// howManyNotToShow - jak duzo zostanie usunietych ze sciezki po kliknieciu myszka
static void drawActiveBase(Game *game, cairo_t *cr, bool shouldShowNewBase, int howManyNotToShow){
    if(vectorSize(game->activeBaseDots) == 0)
        return;

    cairo_set_line_width(cr, BASE_LINE_WIDTH);
    
    Dot *previousDot = (Dot*)vectorGet(game->activeBaseDots, 
                        shouldShowNewBase ? vectorSize(game->activeBaseDots) -1 : 0);

    bool wasMoved = false;
    for(int i = shouldShowNewBase ? 0 : 1; i < vectorSize(game->activeBaseDots); i++){
        Dot *dot = (Dot*)vectorGet(game->activeBaseDots, i);
        PointDouble from = toPixelPoint(game,previousDot->p),
                    to = toPixelPoint(game, dot->p);

        bool shouldShowAsPath = i<vectorSize(game->activeBaseDots)-howManyNotToShow;
        if(shouldShowAsPath)
            cairo_set_source_rgba (cr, COLOR_ACTIVE_BASE_PATH);    
        else
            cairo_set_source_rgba(cr, COLOR_ACTIVE_BASE_PATH_TO_BE_REMOVED);

        if(!wasMoved || howManyNotToShow > 0){
            cairo_move_to (cr, from.x, from.y);
            wasMoved = true;
        }
        cairo_line_to (cr, to.x, to.y);
        if(howManyNotToShow > 0){ /// musze rysowac na biezaco, bo sa roznych kolorow
            cairo_stroke(cr);
        }
        previousDot = dot;
    }
    if(shouldShowNewBase){
        cairo_stroke_preserve(cr);
        cairo_set_source_rgba (cr, COLOR_ACTIVE_BASE_BG);
        cairo_fill(cr);
    }
    else if(howManyNotToShow == 0){
        cairo_stroke(cr);

        /// rysowanie linii do potencjalnej kolejnej kropki
        if(vectorSize(game->activeBaseDots) > 0
                    && game->hoverDot != NULL
                    && canBeAddedtoActiveBasePath(game, game->hoverDot)){
            cairo_set_source_rgba (cr, COLOR_HOVER_NEXT_IN_PATH);    

            Dot *last = (Dot*)vectorGet(game->activeBaseDots, vectorSize(game->activeBaseDots)-1);
            PointDouble from = toPixelPoint(game,last->p),
                        to = toPixelPoint(game, game->hoverDot->p);
            cairo_move_to (cr, from.x, from.y);
            cairo_line_to (cr, to.x, to.y);
            cairo_stroke(cr);

        }
    }
}

/// odpowiedzialne za wyswietlenie widgetu
static gboolean gtkDrawCallback(GtkWidget *widget, cairo_t *cr, gpointer data) {
    Game *game = (Game*)data;


    bool shouldShowNewBase = isAboutToMakeBase(game);
    int howManyNotToShowAsPath = shouldShowNewBase ? 0 : howManyAreWeAboutToRemove(game);

    drawGrid(game, cr);
    drawDots(game, cr, howManyNotToShowAsPath, true);
    drawBases(game, cr);
    drawActiveBase(game, cr, shouldShowNewBase, howManyNotToShowAsPath);
    drawDots(game, cr, howManyNotToShowAsPath, false);

    return FALSE;
}

/// Obsluguje zmiane pozycji myszki
static void mouseMoved(Game *game, PointInt mousePos){
    PointInt *pos =  toArrPoint(game,  pointDouble(mousePos.x, mousePos.y), DOT_CLICK_RADIUS);

    Dot *prevHoveredDot = game->hoverDot;
    game->hoverDot = NULL;
    if(pos!=NULL){
        game->hoverDot = &game->gameLogic->dots[pos->x][pos->y];
        free(pos);
    }

    if(prevHoveredDot != game->hoverDot){
        reRender(game);
        // if(prevHoveredDot!=NULL)
        //     reRenderSingleDot(game, prevHoveredDot->p);
        // if(game->hoverDot!=NULL)
        //     reRenderSingleDot(game, game->hoverDot->p);
    }
}

/// gdy zmieniono pozycje myszki
static gboolean mouseMovedEventHandler (GtkWidget *widget, GdkEventMotion *event, gpointer data){
    Game *game = (Game*)data;
    mouseMoved(game, pointInt(event->x, event->y));
    return TRUE;
}


/// gdy kliknieto przycisk myszki
static gboolean mousePressEventHandler(GtkWidget *widget, GdkEventButton *event, gpointer data){
    Game *game = (Game*)data;
    
    mouseMoved(game, pointInt(event->x, event->y));

    if(!gameIsMyTurn(game) || game->hoverDot == NULL)
        return FALSE;

    if (event->button == 1){
        if(gameLogicIsMoveLegal(game->gameLogic, game->hoverDot->p)){
            gameMakeMove(game, &game->hoverDot->p);

            if(game->onMoveMade)
                game->onMoveMade(game->hoverDot->p, game->onMoveMadeData);
        }
        else if(isAboutToMakeBase(game)){
            gameLogicAddBase(game->gameLogic, game->activeBaseDots);

            if(game->onBaseCreated){
                Vector *points = vectorDotsToVectorPoints(game,game->activeBaseDots);
                game->onBaseCreated(points, game->onBaseCreatedData);
                vectorDestroyWithItems(points, free);
            }
            vectorClear(game->activeBaseDots);

            reRender(game); 
        }
        else if(canBeAddedtoActiveBasePath(game, game->hoverDot)){
            vectorPush(game->activeBaseDots, game->hoverDot);
            reRender(game);
        }
        else if(howManyAreWeAboutToRemove(game)>0){
            while(vectorPop(game->activeBaseDots)!=game->hoverDot){}
            reRender(game); 
        }
    }

    return TRUE;
}

/// gdy myszka oposcila widget
static gboolean mouseLeftEventHandler(GtkWidget *widget, GdkEventCrossing *event, gpointer data){
    Game *game = (Game*)data;
    Dot *prevHoveredDot = game->hoverDot;
    game->hoverDot = NULL;
    if(prevHoveredDot!=NULL){
        reRender(game);
    }
    return TRUE;
}


