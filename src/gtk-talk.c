// #include <string.h>
// #include <math.h>
// #include <gtk/gtk.h>
// #include "fifo.h"
// // kompilacja pod linuxem z lin-fifo.c a pod windowsem z win-fifo.c
// // linux:   gcc -std=c99 -Wall -o gtk-talk gtk-talk.c lin-fifo.c `pkg-config gtk+-3.0 --cflags --libs`
// // windows: gcc -std=c99 -Wall -o gtk-talk gtk-talk.c win-fifo.c `pkg-config gtk+-3.0 --cflags --libs`

// #define MAKS_DL_TEKSTU 10000

// static GtkWidget *window, *bufor;
// static char *moj_id, *twoj_id;

// static void przekaz_tekst( GtkWidget *widget,GtkWidget *text);
// static gboolean pobierz_tekst(gpointer data);
// static void zakoncz(GtkWidget *widget, gpointer data);


// int main(int argc,char *argv[])
// {

//     gtk_init(&argc, &argv);

//     gchar naglowek[31];
//     sprintf(naglowek,"Hej %c, tu %c, porozmawiajmy :-)",twoj_id[0],moj_id[0]);
//     window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//     gtk_window_set_title(GTK_WINDOW(window),naglowek);
//     g_signal_connect(G_OBJECT(window), "destroy",G_CALLBACK(zakoncz), NULL);
//     gtk_container_set_border_width(GTK_CONTAINER(window), 10);

//     GtkWidget *grid = gtk_grid_new();
//     gtk_grid_set_row_spacing(GTK_GRID(grid), 1);
//     gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
//     gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
//     gtk_container_add(GTK_CONTAINER(window), grid);

//     bufor = (GtkWidget *)gtk_text_buffer_new (NULL);
//     GtkWidget *text_view = gtk_text_view_new_with_buffer (GTK_TEXT_BUFFER(bufor));
//     gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
//     gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
//     gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view), FALSE);

//     GtkWidget *scrolled_window = gtk_scrolled_window_new (NULL, NULL);
//     gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC,
//                                   GTK_POLICY_AUTOMATIC);
//     gtk_container_add (GTK_CONTAINER (scrolled_window), text_view);
//     gtk_container_set_border_width (GTK_CONTAINER(scrolled_window), 1);
//     gtk_grid_attach(GTK_GRID(grid), scrolled_window, 0, 0, 60, 20);

//     GtkWidget *text = gtk_entry_new();
//     gtk_entry_set_max_length(GTK_ENTRY(text), MAKS_DL_TEKSTU);
//     gtk_entry_set_text(GTK_ENTRY(text), "");
//     g_signal_connect(G_OBJECT(text), "activate",G_CALLBACK(przekaz_tekst),(gpointer) text);
//     gtk_grid_attach(GTK_GRID(grid), text, 0, 20, 60, 1);

//     GtkWidget *button=gtk_button_new_with_label("koniec");
//     g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(zakoncz), NULL);
//     gtk_grid_attach(GTK_GRID(grid), button, 25, 21, 10, 1);

//     g_timeout_add(100,pobierz_tekst,NULL);

//     gtk_widget_show_all(window);
//     gtk_widget_grab_focus(text);
//     gtk_main();
//     return 0;
// }

// static void przekaz_tekst( GtkWidget *widget,GtkWidget *text)
// {
//     gchar wejscie[MAKS_DL_TEKSTU+5];

//     sendStringToPipe(potoki, gtk_entry_get_text (GTK_ENTRY (text)));

//     strcpy(wejscie,moj_id);
//     strcpy(wejscie+strlen(wejscie),gtk_entry_get_text (GTK_ENTRY (text)));
//     strcat(wejscie,"\n");

//     gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER(bufor),wejscie,-1);
//     gtk_entry_set_text(GTK_ENTRY(text), "");

// }

// static gboolean pobierz_tekst(gpointer data)
// {
//   gchar wejscie[MAKS_DL_TEKSTU+5];

//   strcpy(wejscie,twoj_id);
//   if (getStringFromPipe(potoki,wejscie+strlen(wejscie),MAKS_DL_TEKSTU)) {
//       strcat(wejscie,"\n");
//       gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER(bufor),wejscie,-1);
//   }
//   return TRUE;
// }


