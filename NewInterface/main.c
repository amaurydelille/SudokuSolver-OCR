#include <gtk/gtk.h>
#include "../GridReconstruction/main.h"
#include "../ImageTreatment/treatment.h"
#include "../GridDetection/cutter.h"
#include <SDL.h>
#include <SDL_image.h>
#include "../GridSolver/solver.h"

GtkWidget *image;
GtkWindow *window;
GtkFixed *fixed;
guint resize_timeout_id = 0;
GdkPixbuf *pixbuf;
GtkButton *back;
GtkFileChooserButton *select_file;
char *path;
int bw = 1;
int gd = 1;
int r = 1;

// Function to perform image resizing
gboolean resize_image() {
    if (image == NULL)
        return G_SOURCE_REMOVE;
    gint width, height;
    gtk_window_get_size(GTK_WINDOW(window), &width, &height);

    GdkPixbuf *resized_pixbuf = gdk_pixbuf_scale_simple(pixbuf, (gdk_pixbuf_get_width(pixbuf) * (height * 0.75)) /
                                                                gdk_pixbuf_get_height(pixbuf), height * 0.75,
                                                        GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), resized_pixbuf);
    g_object_unref(resized_pixbuf);

    // Reset the timeout ID to 0 after the operation is done
    resize_timeout_id = 0;

    return G_SOURCE_REMOVE;
}


void on_first_treatment() {
    if (image != NULL) {
        if (bw) {
            applyTreatment(path, "sources/treatment.jpg", 0);
            bw = 0;
        }
        GdkPixbuf *new_pixbuf = gdk_pixbuf_new_from_file("sources/treatment.jpg", NULL);
        gtk_image_set_from_pixbuf(GTK_IMAGE(image), new_pixbuf);
        g_object_unref(pixbuf);
        pixbuf = new_pixbuf;
        resize_image();
    }
}

void on_grid() {
    if (image != NULL && !bw) {
        if (gd) {
            SDL_Surface *image = LoadImage(path);
            SDL_Surface *bin_image = LoadImage("sources/treatment.jpg");
            AutoCutter(image, bin_image);
            free(image);
            free(bin_image);
            gd = 0;
        }
        GdkPixbuf *new_pixbuf = gdk_pixbuf_new_from_file("result.png", NULL);
        gtk_image_set_from_pixbuf(GTK_IMAGE(image), new_pixbuf);
        g_object_unref(pixbuf);
        pixbuf = new_pixbuf;
        resize_image();
    }
}

void on_showresult() {
    if (image != NULL) {
        if (bw) {
            on_first_treatment();
        }
        if (gd) {
            on_grid();
        }
        if (r) {
             solveWithNeuralNetwork();
            CreateGrid("sources/NewGrid.jpg", "sources/grid", "sources/grid.result");
            r = 0;
        }
    }
    GdkPixbuf *new_pixbuf = gdk_pixbuf_new_from_file("sources/NewGrid.jpg", NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), new_pixbuf);
    g_object_unref(pixbuf);
    pixbuf = new_pixbuf;
    resize_image();
}

void on_back() {
    if (image == NULL)
        return;
    GdkPixbuf *new_pixbuf = gdk_pixbuf_new_from_file(path, NULL);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), new_pixbuf);
    g_object_unref(pixbuf);
    pixbuf = new_pixbuf;
    resize_image();
}

void update_image_from_file(GtkFileChooserButton *file_chooser) {
    path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file_chooser));
    if (path != NULL) {
        if (image == NULL) {
            pixbuf = gdk_pixbuf_new_from_file(path, NULL);
            image = gtk_image_new_from_pixbuf(pixbuf);
            gtk_fixed_put(GTK_FIXED(fixed), image, 200, 100);
            resize_image();
            gtk_widget_show_all(GTK_WIDGET(window));
        } else {
            GdkPixbuf *new_pixbuf = gdk_pixbuf_new_from_file(path, NULL);
            gtk_image_set_from_pixbuf(GTK_IMAGE(image), new_pixbuf);
            g_object_unref(pixbuf);
            pixbuf = new_pixbuf;
            resize_image();
        }
        bw = 1;
        gd = 1;
        r = 1;
    }
}

gboolean on_window_resize() {

    gint width, height;
    gtk_window_get_size(GTK_WINDOW(window), &width, &height);

    gtk_fixed_move(GTK_FIXED(fixed), GTK_WIDGET(back), width * 0.90, 30);
    gtk_fixed_move(GTK_FIXED(fixed), GTK_WIDGET(select_file), 20, height * 0.75);

    if (image == NULL)
        return FALSE;
    // Clear any previous timeout if it exists
    if (resize_timeout_id > 0)
        g_source_remove(resize_timeout_id);

    // Set a new timeout to perform resizing after a short delay (50 milliseconds)
    resize_timeout_id = g_timeout_add(10, (GSourceFunc) resize_image, NULL);

    return FALSE;
}

// Main function.
int main(int argc, char *argv[]) {
    // Initializes GTK.
    gtk_init(NULL, NULL);

    if (argc == 2) {
        path = argv[1];
        // Load an image and resize it
        pixbuf = gdk_pixbuf_new_from_file(path, NULL);
        GdkPixbuf *resized_pixbuf = gdk_pixbuf_scale_simple(pixbuf, 200, 200, GDK_INTERP_BILINEAR);

        // Create a new image widget with the resized image
        image = gtk_image_new_from_pixbuf(resized_pixbuf);
        g_object_unref(resized_pixbuf); // Release the resized pixbuf
    }

    // Loads the UI description and builds the UI.
    // (Exits if an error occurs.)
    GtkBuilder *builder = gtk_builder_new();
    GError *error = NULL;
    if (gtk_builder_add_from_file(builder, "Interface.glade", &error) == 0) {
        g_printerr("Error loading file: %s\n", error->message);
        g_clear_error(&error);
        return 1;
    }
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "*.jpg");
    gtk_file_filter_add_pattern(filter, "*.jpeg");
    gtk_file_filter_add_pattern(filter, "*.png");


    GdkColor color;
    color.red = 12000;
    color.green = 12000;
    color.blue = 12000;

    // Gets the widgets.
    window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
    gtk_window_set_title(GTK_WINDOW(window), "Sudocrew");
    gtk_widget_modify_bg(GTK_WINDOW(window), GTK_STATE_NORMAL, &color);
    GtkButton *first_treatment = GTK_BUTTON(gtk_builder_get_object(builder, "button1"));
    GtkButton *grid = GTK_BUTTON(gtk_builder_get_object(builder, "button2"));
    GtkButton *resolution = GTK_BUTTON(gtk_builder_get_object(builder, "button3"));
    back = GTK_BUTTON(gtk_builder_get_object(builder, "retour"));
    fixed = GTK_FIXED(gtk_builder_get_object(builder, "fixed1"));
    select_file = GTK_FILE_CHOOSER_BUTTON(gtk_builder_get_object(builder, "fileButton"));
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(select_file), filter);


    if (image != NULL)
        gtk_fixed_put(GTK_FIXED(fixed), image, 200, 100);

    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(css_provider, "styles.css", NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(first_treatment));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    context = gtk_widget_get_style_context(GTK_WIDGET(back));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    context = gtk_widget_get_style_context(GTK_WIDGET(grid));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    context = gtk_widget_get_style_context(GTK_WIDGET(resolution));
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);


    gtk_widget_show_all(GTK_WIDGET(window));

    // Connects signal handlers.
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(first_treatment, "clicked", G_CALLBACK(on_first_treatment), NULL);
    g_signal_connect(grid, "clicked", G_CALLBACK(on_grid), NULL);
    g_signal_connect(back, "clicked", G_CALLBACK(on_back), NULL);
    g_signal_connect(resolution, "clicked", G_CALLBACK(on_showresult), NULL);
    g_signal_connect(window, "configure-event", G_CALLBACK(on_window_resize), NULL);
    g_signal_connect(select_file, "file-set", G_CALLBACK(update_image_from_file), NULL);

    // Runs the main loop.
    gtk_main();
    if (pixbuf != NULL)
        g_object_unref(pixbuf); // Release the original pixbuf
    // Exits.
    return 0;
}

