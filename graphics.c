/*
 * Copyright © 2014 Bart Massey
 * [This program is licensed under the "MIT License"]
 * Please see the file COPYING in the source
 * distribution of this software for license terms.
 */ 

#include <inttypes.h>

#include <cairo.h>
#include <gtk/gtk.h>

#include "duvis.h"

static int display_width, display_height;

static void draw_node(cairo_t *cr, struct entry *e,
                      int x, int y, int width, int height) {

    /* Length of 2**64 - 1, +1 for null */
    char sizeStr[21];

    int txtX = width / 4; 
    int txtY = height / 2;

    /* Copy uint64_t into char buffer */
    sprintf(sizeStr, "%" PRIu64, e->size);

    /* Draw the rectangle container */
    cairo_rectangle(cr, x, y, width, height);
    cairo_stroke(cr);

    /* Draw the label */
    cairo_move_to(cr, txtX, txtY);
    if (e->depth == 0) {
        cairo_show_text(cr, e->components[0]);
        for (int i = 1; i < base_depth; i++) {
            cairo_show_text(cr, "/");
            cairo_show_text(cr, e->components[i]);
        }
    } else {
        cairo_show_text(cr, e->components[e->n_components - 1]);
    }
    cairo_show_text(cr, " (");
    cairo_show_text(cr, sizeStr);
    cairo_show_text(cr, ")");
}

static void draw_tree(cairo_t *cr, struct entry *e) {
    draw_node(cr, e, 0, 0, display_width, display_height);
}

/* Perform the actual drawing of the entries */
static void do_drawing(GtkWidget *widget, cairo_t *cr) {

    /* How much space was the window actually allocated? */
    GtkAllocation *allocation = g_new0 (GtkAllocation, 1);
    gtk_widget_get_allocation(GTK_WIDGET(widget), allocation);
    display_width = allocation->width;
    display_height = allocation->height;

    /* Allocation no longer needed */
    g_free(allocation);
   
    /* Set cairo drawing variables */
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Helvetica",
                           CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 20);
    cairo_set_line_width(cr, 1);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_MITER);
    
    /* Begin drawing the nodes */
    draw_tree(cr, root_entry);
}

/* Call up the cairo functionality */
static gboolean on_draw_event(GtkWidget *widget, cairo_t *cr,
                              gpointer user_data) {
   
    do_drawing(widget, cr);

    return FALSE;
}

/* Determine the size of the window */
static void getSize(GtkWidget *widget,
                    GtkAllocation *allocation, void *data) {
    display_width = allocation->width;
    display_height = allocation->height;
}

/* Initialize the window, drawing surface, and functionality */
int gui(int argv, char **argc) {

    GtkWidget *window;
    GtkWidget *darea;

    /* Initialize GTK, the window, and the drawing surface */
    gtk_init(&argv, &argc); 
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    darea = gtk_drawing_area_new();

    /* Put the drawing surface 'inside' the window */
    gtk_container_add(GTK_CONTAINER(window), darea);

    /* Functionality handling - drawing and exiting */
    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(on_draw_event), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(G_OBJECT(darea), "size-allocate",
                     G_CALLBACK(getSize), NULL);

    /* Default window settings */
    gtk_window_set_title(GTK_WINDOW(window), "Duvis");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 480);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    /* Display the window */
    gtk_widget_show_all(window);
    gtk_main();

    return (0);
}
