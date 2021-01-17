#ifndef GTK_HELPER_H
#define GTK_HELPER_H
#include <gtk/gtk.h>

/// dodaje css klase do kontrolki 
void addClass(GtkWidget *widget, const char* className);

/// laduje style css
void loadCss(const char *filename);

/// pokazuje wiadomosc
void showMessage(char *msg, GtkWindow *window);

/// wyswietla okno prompta do otworzenia pliku
gchar *pickFileToOpen(GtkWindow *parentWindow, const char* msg);

/// wyswietla okno prompta do zapisu pliku
gchar *pickFileToSave(GtkWindow *parentWindow, const char* msg);

#endif