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

#include <gtk/gtk.h>

enum
{
	COLUMN_NAME = 0,
	COLUMN_STATUS,
	N_COLUMNS
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

static void init_menubar(GtkWidget *box)
{
	GtkWidget *menubar;
	GtkWidget *filemenu;
	GtkWidget *file;
	GtkWidget *new;
	GtkWidget *train;
	GtkWidget *restore;
	GtkWidget *sep;
	GtkWidget *properties;
	GtkWidget *sep2;
	GtkWidget *quit;
	GtkWidget *viewmenu;
	GtkWidget *view;
	GtkWidget *classes;
	GtkWidget *models;
	GtkWidget *helpmenu;
	GtkWidget *help;
	GtkWidget *about;

	menubar = gtk_menu_bar_new();
	filemenu = gtk_menu_new();
	viewmenu = gtk_menu_new();
	helpmenu = gtk_menu_new();
	
	file = gtk_menu_item_new_with_label("File");
	new = gtk_menu_item_new_with_label("New...");
	train = gtk_menu_item_new_with_label("Train...");
	restore = gtk_menu_item_new_with_label("Restore Defaults");
	sep = gtk_separator_menu_item_new();
	properties = gtk_menu_item_new_with_label("Properties");
	sep2 = gtk_separator_menu_item_new();
	quit = gtk_menu_item_new_with_label("Quit");
	view = gtk_menu_item_new_with_label("View");
	classes = gtk_menu_item_new_with_label("Classes");
	models = gtk_menu_item_new_with_label("Models");
	help = gtk_menu_item_new_with_label("Help");
	about = gtk_menu_item_new_with_label("About...");
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), new);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), train);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), restore);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), properties);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep2);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), viewmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(viewmenu), classes);
	gtk_menu_shell_append(GTK_MENU_SHELL(viewmenu), models);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), view);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);
	
	gtk_box_pack_start(GTK_BOX(box), menubar, FALSE, FALSE, 3);	
}

static void init_list(GtkWidget *box)
{
	GtkWidget *list;
	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;

	list = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), TRUE);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Name",
		renderer, "text", COLUMN_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
	
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Status",
		renderer, "text", COLUMN_STATUS, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
	
	store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

	g_object_unref(store);
	
	gtk_box_pack_start(GTK_BOX(box), list, TRUE, TRUE, 5);
	
	  add_to_list(list, "Aliens");
  add_to_list(list, "Leon");
  add_to_list(list, "Capote");
  add_to_list(list, "Saving private Ryan");
  add_to_list(list, "Der Untergang");
	
}

static void init_toolbar(GtkWidget *box)
{
	GtkWidget *toolbar;
	GtkToolItem *new;
	GtkToolItem *train;
	GtkToolItem *view;
	
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_TEXT);
	
	new = gtk_tool_button_new(NULL, "New");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new, -1);
	train = gtk_tool_button_new(NULL, "Train");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), train, -1);
	view = gtk_tool_button_new(NULL, "View");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), view, -1);
	
	gtk_box_pack_start(GTK_BOX(box), toolbar, FALSE, FALSE, 4);
}

/* graphical user interface */
static void main_ui(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	main_ui(argc, argv);
		
	return 0;	
}

/* graphical user interface */
static void main_ui(int argc, char *argv[])
{
	/* declarations */
	GtkWidget *window;
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
	
	/* objects */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "Gestures Manager");
	
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* toolbar */
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_TEXT);
	
	new = gtk_tool_button_new(NULL, "New");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new, -1);
	train = gtk_tool_button_new(NULL, "Train");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), train, -1);
	view = gtk_tool_button_new(NULL, "View");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), view, -1);
	
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 4);
	
	/* list */
	list = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), TRUE);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Name",
		renderer, "text", COLUMN_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
	
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Status",
		renderer, "text", COLUMN_STATUS, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);
	
	store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

	g_object_unref(store);
	
	gtk_box_pack_start(GTK_BOX(vbox), list, TRUE, TRUE, 5);
	
	add_to_list(list, "Aliens");
	add_to_list(list, "Leon");
	add_to_list(list, "Capote");
	add_to_list(list, "Saving private Ryan");
	add_to_list(list, "Der Untergang");

	/* events */
	g_signal_connect_swapped(G_OBJECT(window), "destroy",
		G_CALLBACK(gtk_main_quit), NULL);

	//g_signal_connect(G_OBJECT(quit), "activate",
	//	G_CALLBACK(gtk_main_quit), NULL);
 
	gtk_widget_show_all(window);
	
	gtk_main();
}
