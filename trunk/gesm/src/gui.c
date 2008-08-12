/*
 * Copyright (C) 2008 by Openmoko, Inc.
 * Written by Paul-Valentin Borza <paul@borza.ro>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <glib.h>

#include "gesm.h"
#include "gui.h"

enum
{
	COLUMN_NAME = 0,
	COLUMN_LEN
};

//static GtkWidget *label = 0;
static GtkWidget *bar = 0;
static GtkWidget *treeview = 0;
static GtkWidget *train_toolbutton = 0;
/* this is the thread that is used for training */
static pthread_t thread;

/* */
static void add_to_list (GtkWidget *list, const gchar *str);
/* */
int filter_model (const struct dirent *entry);
/* */
static void init_list (GtkWidget *list);
/* */
static void refresh_list (GtkWidget *list, char *dir);
/* */
void on_window_destroy (GtkObject *object, gpointer user_data);
/* */
void *train_clicked (void *arg);
/* */
void on_train_toolbutton_clicked (GtkObject *object, gpointer user_data);
/* */
void on_refresh_toolbutton_clicked (GtkObject *object, gpointer user_data);

void update_gui_same_thread(char *msg)
{
	if (bar) {
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR (bar), msg);
	}
}

/* */
void update_gui(char *msg)
{
	if (bar) {
		gdk_threads_enter();  // Protect from gtk main loop
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR (bar), msg);

		gdk_flush();
		gdk_threads_leave();
		/* kinf of ugly way to know that the other thread has been canceled, or has finished processing, but works */
		if ((strcmp(msg, "Disconnected") == 0) ||
			(strcmp(msg, "Closed") == 0)) {
			gtk_widget_set_sensitive(train_toolbutton, TRUE);
		}
	}	
}

/* graphical user interface */
void main_gui (int argc, char *argv[])
{
	GladeXML *xml;
	GtkWidget *window;
	
	g_thread_init(0);
	gdk_threads_init();
	gtk_init(&argc, &argv);
	xml = glade_xml_new(GLADEDIR "/window.glade", 0, 0);
	
	/* get a widget (useful if you want to change something) */
	window = glade_xml_get_widget(xml, "window");
	treeview = glade_xml_get_widget(xml, "treeview");
	//label = glade_xml_get_widget(xml, "label");
	bar = glade_xml_get_widget(xml, "bar");
	train_toolbutton = glade_xml_get_widget(xml, "train_toolbutton");

	/* connect signal handlers */
	glade_xml_signal_autoconnect(xml);

	gtk_widget_show (window);
	/* change dir for config dir */
	chdir (dir);
	
	/* load files */
	init_list(treeview);
	refresh_list(treeview, dir);

	gdk_threads_enter();	
	gtk_main();
	gdk_threads_leave();
}

/*
 * 
 */
static void add_to_list(GtkWidget *list, const gchar *str)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model
		(GTK_TREE_VIEW(list)));
	
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, COLUMN_NAME, str, -1);
}

/*
 * 
 */
int filter_model(const struct dirent *entry)
{
	char *p = rindex(entry->d_name, '.');
	return ((p) && (strcmp(p, ".model") == 0));
}

/*
 * 
 */
static void init_list(GtkWidget *list)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;
	
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Available gestures",
		renderer, "text", COLUMN_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
		
	store = gtk_list_store_new(COLUMN_LEN, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

	g_object_unref(store);
}

/*
 * 
 */
static void refresh_list(GtkWidget *list, char *dir)
{
	struct dirent **names;
	int names_len;
	
	GtkListStore *store;

	store = GTK_LIST_STORE(gtk_tree_view_get_model
		(GTK_TREE_VIEW(list)));
	gtk_list_store_clear(store);
	
	names_len = scandir(dir, &names, filter_model, alphasort);
	if (names_len < 0)
	{
		perror("scandir");
	}
	else
	{
		while (names_len--)
		{
			//printf("%s\n", names[names_len]->d_name);
			add_to_list(list, names[names_len]->d_name);
			free(names[names_len]);
		}
		free(names);
	}
}

/*
 * 
 */
void on_window_destroy (GtkObject *object, gpointer user_data)
{
	gtk_main_quit ();
}

/*
 * 
 */
void *train_clicked (void *arg)
{
	handshake('n'); /* recreate gesture */
	handshake('e'); /* train existing gesture */
	return 0;
}

/*
 * 
 */
void on_train_toolbutton_clicked (GtkObject *object, gpointer user_data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gchar *name;
		gtk_tree_model_get (model, &iter, COLUMN_NAME, &name, -1);

		/* disable train button and enable cancel button */
		gtk_widget_set_sensitive(train_toolbutton, FALSE);
		
		strcpy(file, name);
		pthread_create(&thread, 0, train_clicked, 0);

		//char *result;
		//pthread_join(thread, &result);
		//printf("returned from thread");

		g_free(name);
	}
	else
	{
		update_gui_same_thread ("Please select a model first");
	}
}

/*
 * 
 */
void on_refresh_toolbutton_clicked (GtkObject *object, gpointer user_data)
{
	refresh_list(treeview, dir);
}

