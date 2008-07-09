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
#include <getopt.h>
#include <gtk/gtk.h>
#include <string.h>

#define VERSION "0.1"

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

static void print_version(void);

static void print_usage(char *file_name);

/* graphical user interface */
static void main_ui(int argc, char *argv[], char *dir);

int main(int argc, char *argv[])
{
	char dir[1024];
	
	int long_opt_ind = 0;
	int long_opt_val = 0;
	
	static struct option long_opts[] = {
		{ "dir", required_argument, 0, 'd' },
		{ "version", no_argument, 0, 'v' },
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	dir[0] = '\0';	
	opterr = 0;
	while ((long_opt_val = getopt_long(argc, argv, "d:vh", long_opts, &long_opt_ind)) != -1) 
	{
		switch (long_opt_val)
		{
			case 'd':
				strncpy(dir, optarg, sizeof(dir));
				dir[sizeof(dir) / sizeof(dir[0]) - 1] = '\0';
				break;
			case 'v':
				print_version();
				
				exit(0);
				break;
			case 'h':
			case '?':
				print_usage(argv[0]);
				
				exit(0);
				break;
		}
	}

	if (dir[0] == '\0')
	{
		print_usage(argv[0]);
		exit(1);
	}
	
	/* should be ok here */
	main_ui(argc, argv, dir);

	return 0;	
}

static void print_version(void)
{
	printf("Version: %s\n", VERSION);
}

static void print_usage(char *file_name)
{
	printf("Usage: %s --dir <dir> | --version | --help\n", file_name);
}

/* graphical user interface */
static void main_ui(int argc, char *argv[], char *dir)
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
	
	gtk_box_pack_start(GTK_BOX(vbox), list, TRUE, TRUE, 5);
	
	refresh_list(list, dir);
	
	/* events */
	g_signal_connect_swapped(G_OBJECT(window), "destroy",
		G_CALLBACK(gtk_main_quit), NULL);

	//g_signal_connect(G_OBJECT(quit), "activate",
	//	G_CALLBACK(gtk_main_quit), NULL);
 
	gtk_widget_show_all(window);
	
	gtk_main();
}
