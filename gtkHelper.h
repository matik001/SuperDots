#ifndef GTK_HELPER_H
#define GTK_HELPER_H
#include <gtk/gtk.h>

void addClass(GtkWidget *widget, const char* className);
void loadCss(const char *filename);
void showMessage(char *msg, GtkWindow *window);
gchar *pickFileToOpen(GtkWindow *parentWindow, const char* msg);
gchar *pickFileToSave(GtkWindow *parentWindow, const char* msg);

#endif