#include "gtkHelper.h"

void addClass(GtkWidget *widget, const char* className){
    GtkStyleContext *context;
    context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_class(context, className);
}

void loadCss(const char *filename){
    GtkCssProvider *provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_path (provider, filename, NULL);

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                            GTK_STYLE_PROVIDER(provider),
                            GTK_STYLE_PROVIDER_PRIORITY_USER);   
}

void showMessage(char *msg, GtkWindow *window){
    GtkWidget *dialog;
    dialog=gtk_message_dialog_new (window,GTK_DIALOG_DESTROY_WITH_PARENT,
				   GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,"%s",msg);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

gchar *pickFileToOpen(GtkWindow *parentWindow, const char* msg){
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
    gint res;

    dialog = gtk_file_chooser_dialog_new (msg,
                                        parentWindow,
                                        action,
                                        "Anuluj",
                                        GTK_RESPONSE_CANCEL,
                                        "Otwórz",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);

    gchar *filename = NULL;
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    if (res == GTK_RESPONSE_ACCEPT)
    {
        GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);

        filename = gtk_file_chooser_get_filename (chooser);
    }

    gtk_widget_destroy (dialog);

    return filename;
}

gchar *pickFileToSave(GtkWindow *parentWindow, const char* msg){
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;
    gint res;

    dialog = gtk_file_chooser_dialog_new (msg,
                                        parentWindow,
                                        action,
                                        "Anuluj",
                                        GTK_RESPONSE_CANCEL,
                                        "Zapisz",
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
    chooser = GTK_FILE_CHOOSER (dialog);

    gtk_file_chooser_set_do_overwrite_confirmation (chooser, TRUE);
    // if (user_edited_a_new_document)
    // gtk_file_chooser_set_current_name (chooser,
    //                                     _("Untitled document"));
    // else{
    gtk_file_chooser_set_current_folder(chooser, "save");
    gtk_file_chooser_set_filename (chooser, "Zapis gry");
    //}
    res = gtk_dialog_run (GTK_DIALOG (dialog));
    
    gchar *filename = NULL;

    if (res == GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename (chooser);
    }

    gtk_widget_destroy (dialog);

    return filename;
}

