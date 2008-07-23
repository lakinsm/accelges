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

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <glib-object.h>

#include "ges.h"
#include "gesd.h"
#include "accelneo.h"
#include "accelwii.h"
#include "service_server.h"
#include "service.h"

#define VERSION "0.1"
G_DEFINE_TYPE( GesApplication, ges_service, G_TYPE_OBJECT );

enum device used_device;

static wii_t wii;
static neo_t neo;

static struct ges_3d_t ges;
static struct config_t config;
static char config_file_name[1024];

/* print welcome message */
static void print_header(void);
/* print all command line options */
static void print_usage(char *file_name);
/* print current version */
static void print_version(void);
/* parse options given at command line */
static unsigned char parse_options(int argc, char **argv);
/* catches the termination signal and closes the Wii */
static void handle_signal(int signal);
/* catches the termination signal and closes the Wii */
static void neo_handle_signal(int signal);

/* this handler is called whenever the Wii sends acceleration reports */
void handle_accel(unsigned char pressed, struct accel_3d_t accel)
{
	/* further call the recognizer */
	ges_process_3d(&ges, accel);
	if (pressed)
	{
		printf("%+f\t%+f\t%+f\n", accel.val[0], accel.val[1], accel.val[2]);
	}
}

/*
 * function called by the recognizer once the recognition is done 
 */
void handle_reco(char *reco)
{
	char cmd[1024];
	cmd[0] = '\0';
	//strcat(cmd, "dcop amarok player ");
	/* reco is specified in the call/call.ges file */
	/* reco might be play, pause, next, prev, or etc. */
	//strcat(cmd, reco);
	strcpy(cmd, reco);
	printf("Run: %s\n", cmd);
	system(cmd); // or execl ???
	printf("Returned.\n"); 
}

static guint sig;

static void ges_service_class_init ( GesApplicationClass *ges_class )
{
	 sig =
    g_signal_new ("recognized", 
                  G_TYPE_FROM_CLASS (ges_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (GesApplicationClass, recognized),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 
                  1, G_TYPE_STRING);
	
        dbus_g_object_type_install_info ( G_TYPE_FROM_CLASS ( ges_class ), &dbus_glib_gesd_object_info );
}

static void ges_service_init ( GesApplication *ges_object )
{
}

gboolean
gesd_listenfor (GesApplication *obj, const gchar *gesture, GError **error)
{
	system("echo received > /home/paul/openmoko/recv.txt");
	return TRUE;
}

int dbus_comm(void)
{
	DBusGConnection *connection;
  	DBusGProxy *proxy;
  	GError *error = NULL;
  	guint32 ret;
  GesApplication *app;
  	
	  /* initialise type system */
  g_type_init ();
	
	  /* Try and setup our DBus service */
  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL)
  {
    g_warning ("Failed to make a connection to the session bus: %s", 
               error->message);
    g_error_free (error);
    return 1;
  }
  
    proxy = dbus_g_proxy_new_for_name (connection, 
                                     DBUS_SERVICE_DBUS,
                                     DBUS_PATH_DBUS, 
                                     DBUS_INTERFACE_DBUS);
    if (!org_freedesktop_DBus_request_name (proxy,
                                          APP_SERVICE_NAME,
                                          0, &ret, &error))
  {
    /* Error requesting the name */
    g_warning ("There was an error requesting the name: %s\n",error->message);
    g_error_free (error);

    return 1;
  }
  if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
  {

    dbus_g_connection_unref (connection);

    return 1;
  }
  printf("so far so good\n");
  
	app = g_object_new ( GES_TYPE_APPLICATION, NULL );
        dbus_g_object_type_install_info ( GES_TYPE_APPLICATION, &dbus_glib_gesd_object_info );

        dbus_g_connection_register_g_object ( connection, APP_PATH_NAME, G_OBJECT( app ) );


  
  system("echo started auto > /home/paul/openmoko/recv.txt");
  return 0;
}

/*
 * do not run in terminal: sudo hidd --search
 */
int main(int argc, char **argv)
{
	print_header();
	
	if (dbus_comm() > 0)
		return 1;
	
	/* get the configuration file name */
	if (!parse_options(argc, argv))
	{
		exit(1);
	}
	
	if (!ges_load_config(&config, config_file_name))
	{
		exit(2);
	}
	printf("Configuration loaded.\n");
	ges_create_3d(&ges);
	ges_read_3d(&ges, &config);
	/* assign callbacks for recognition and acceleration */
	ges.handle_reco = handle_reco;
	if (used_device == dev_wii)
	{
		wii.handle_recv = handle_accel;
	
		printf("Searching... (Press 1 and 2 on the Wii)\n");
		if (wii_search(&wii, 5) < 0)
		{
			fprintf(stderr, "Could not find the Wii.\n");
			ges_delete_3d(&ges);
			exit(1);
		}
		printf("Found.\n");
	
		if (wii_connect(&wii) < 0)
		{
			fprintf(stderr, "Could not connect to the Wii.\n");
			ges_delete_3d(&ges);
			exit(1);
		}
		printf("Connected.\n");
	
		/* catch terminate signal to stop the read thread and close sockets */
		signal(SIGINT, handle_signal);
		signal(SIGTERM, handle_signal);
	
		wii_set_leds(&wii, 0, 0, 0, 1);
		wii_talk(&wii); /* will enter loop and never get out */
	}
	else if (used_device == dev_neo)
	{
		if (neo_open(&neo, neo_accel2))
		{
			neo.handle_recv = handle_accel;
			
			signal(SIGINT, neo_handle_signal);
			signal(SIGTERM, neo_handle_signal);
		
			neo_begin_read(&neo);
		}
	}
	
	/* won't reach this point, will be terminated with signal */
	ges_delete_3d(&ges);
	
	exit(0);
}

/* 
 * print welcome message
 */
static void print_header(void)
{
	printf("gesphone: (C) 2008 OpenMoko Inc. Paul-Valentin Borza\n"
		"This program is free software under the terms of the GNU General Public License.\n\n");
}

/*
 * print all command line options 
 */
static void print_usage(char *file_name)
{
	printf("Usage: %s --wii | --neo --load <configuration> | --version | --help\n", file_name);
}

/*
 * print current version 
 */
static void print_version(void)
{
	printf("Version: %s\n", VERSION);
}

/*
 * parse options given at command line
 */
static unsigned char parse_options(int argc, char **argv)
{
	int long_opt_ind = 0;
	int long_opt_val = 0;
	
	static struct option long_opts[] = {
		{ "wii", no_argument, 0, 'w' },
		{ "neo", no_argument, 0, 'n' },
		{ "version", no_argument, 0, 'v' },
		{ "load", required_argument, 0, 'l' },
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};
	
	config_file_name[0] = '\0';
	
	used_device = dev_none;
	
	/* don't display errors to stderr */
	opterr = 0;
	while ((long_opt_val = getopt_long(argc, argv, "wnvl:h", long_opts, &long_opt_ind)) != -1) 
	{
		switch (long_opt_val)
		{
			case 'w':
				used_device = dev_wii;
				
				break;
			case 'n':
				used_device = dev_neo;
				
				break;
			case 'v':
				print_version();
				
				exit(0);
				break;
			case 'l':
				strncpy(config_file_name, optarg, sizeof(config_file_name));
				config_file_name[sizeof(config_file_name) / sizeof(config_file_name[0]) - 1] = '\0';
				break;
			case 'h':
			case '?':
				print_usage(argv[0]);
				
				exit(0);
				break;
		}
	}
	
	if ((used_device == dev_none) || (config_file_name[0] == '\0'))
	{
		print_usage(argv[0]);
		return 0;
	}
	
	return 1;
}

/*
 * catches the termination signal and disposes the Wii
 */
static void handle_signal(int signal)
{
	switch (signal)
	{
		case SIGINT:
		case SIGTERM:
			wii_set_leds(&wii, 1, 0, 0, 0);
			wii_disconnect(&wii);
			printf("Disconnected.\n");
			ges_delete_3d(&ges);
			fflush(stdout);
			fflush(stderr);
			exit(0);
			break;
		default:
			break;
	}
}

/*
 * catches the termination signal and disposes the Neo
 */
static void neo_handle_signal(int signal)
{
	switch (signal)
	{
		case SIGINT:
		case SIGTERM:
			neo_close(&neo);
			printf("Closed device.\n");
			ges_delete_3d(&ges);
			fflush(stdout);
			fflush(stderr);
			exit(0);
			break;
		default:
			break;
	}
}
