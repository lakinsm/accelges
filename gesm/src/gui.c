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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <glade/glade.h>
#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>

enum
{
	COLUMN_NAME = 0,
	COLUMN_LEN
};

static void add_to_list(GtkWidget *list, const gchar *str)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model
		(GTK_TREE_VIEW(list)));

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, COLUMN_NAME, str, -1);
}

static int filter_model(struct dirent *entry)
{
	char *p = rindex(entry->d_name, '.');
	return ((p) && (strcmp(p, ".model") == 0));
}

static void init_list(GtkWidget *list)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;
	
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Gesture Name",
		renderer, "text", COLUMN_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
		
	store = gtk_list_store_new(COLUMN_LEN, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

	g_object_unref(store);
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

void 
on_window_destroy (GtkObject *object, gpointer user_data)
{
	gtk_main_quit();
}

void
on_new_toolbutton_clicked (GtkObject *object, gpointer user_data)
{
	printf("new clicked");
}

void
on_train_toolbutton_clicked (GtkObject *object, gpointer user_data)
{
	printf("train clicked");
}

/* graphical user interface */
void
main_gui (int argc, char *argv[], char *dir)
{
	GladeXML *xml;
	GtkWidget *window;
	GtkWidget *treeview;
	GtkWidget *label;
	
	gtk_init(&argc, &argv);
	xml = glade_xml_new(GLADEDIR "/window.glade", 0, 0);
	
	/* get a widget (useful if you want to change something) */
	window = glade_xml_get_widget(xml, "window");
	treeview = glade_xml_get_widget(xml, "treeview");
	label = glade_xml_get_widget(xml, "label");
	
	/* connect signal handlers */
	glade_xml_signal_autoconnect(xml);

	gtk_widget_show (window);
	/* change dir for config dir */
	chdir (dir);
	
	/* load files */
	init_list(treeview);
	refresh_list(treeview, dir);
	
	gtk_main();
}
