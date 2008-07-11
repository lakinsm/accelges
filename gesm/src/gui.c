/*
 * Copyright (C) 2008 by OpenMoko, Inc.
 * Written by Paul-Valentin Borza <gestures@borza.ro>
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

#include <dirent.h>
#include <gtk/gtk.h>
#include <string.h>

enum
{
	COLUMN_NAME = 0,
	COLUMN_STATUS,
	COLUMN_LEN
};

static void add_to_list(GtkWidget *list, const gchar *str)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model
		(GTK_TREE_VIEW(list)));

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, COLUMN_NAME, str, COLUMN_STATUS, "Trained", -1);
}

static int filter_model(struct dirent *entry)
{
	char *p = rindex(entry->d_name, '.');
	return ((p) && (strcmp(p, ".hmm") == 0));
}

static void refresh_list(GtkWidget *list, char *dir)
{
	struct dirent **names;
	int names_len;
	
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

/* graphical user interface */
void main_gui(int argc, char *argv[], char *dir)
{
	/* declarations */
	GtkWidget *window;
	GtkWidget *notebook;
	GtkWidget *vbox;
	
	GtkWidget *toolbar;
	GtkToolItem *new;
	GtkToolItem *train;
	GtkToolItem *view;
	
	GtkWidget *list;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;
		
	/* initialization */
	gtk_init(&argc, &argv);
	
	/* widow */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Gestures Manager");
	g_signal_connect_swapped(G_OBJECT(window), "destroy",
		G_CALLBACK(gtk_main_quit), NULL);
	
	/* notebook */
	notebook = gtk_notebook_new ();
	gtk_container_add(GTK_CONTAINER (window), notebook);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_BOTTOM);
	
	/* navigation */
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox,
		gtk_image_new_from_stock(GTK_STOCK_INDEX, GTK_ICON_SIZE_LARGE_TOOLBAR));
	gtk_container_child_set(GTK_CONTAINER(notebook), vbox, "tab-expand",
		TRUE, NULL);

	/* toolbar */
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_TEXT);
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);
	
	/* new */
	new = gtk_tool_button_new(NULL, "New");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new, -1);
	 // g_signal_connect (data->dial_button, "clicked", 
      //              G_CALLBACK (dial_contact_clicked_cb), data);
	
	/* train */
	train = gtk_tool_button_new(NULL, "Train");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), train, -1);
	
	/* view */
	view = gtk_tool_button_new(NULL, "View");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), view, -1);
	
	/* list */
	
	list = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), TRUE);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Gesture Name",
		renderer, "text", COLUMN_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
	
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Status",
		renderer, "text", COLUMN_STATUS, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
	
	store = gtk_list_store_new(COLUMN_LEN, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

	g_object_unref(store);
	
	gtk_box_pack_start(GTK_BOX(vbox), list, TRUE, TRUE, 0);
	
	refresh_list(list, dir);
	
	/* events */
	g_signal_connect_swapped(G_OBJECT(window), "destroy",
		G_CALLBACK(gtk_main_quit), NULL);

	//g_signal_connect(G_OBJECT(quit), "activate",
	//	G_CALLBACK(gtk_main_quit), NULL);
 
	gtk_widget_show_all(window);
	
	gtk_main();
}
