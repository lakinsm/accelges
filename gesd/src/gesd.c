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

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <glib-object.h>
#include <pthread.h>

#include "ges.h"
#include "gesd.h"
#include "accelneo.h"
#include "accelwii.h"
#include "recognizer.h"
#include "service.h"

#define VERSION "0.1"
#define DBUS_SERVICE_NAME "org.openmoko.gestures"
#define DBUS_RECOGNIZER_PATH "/org/openmoko/gestures/Recognizer"

/* boilerplate for glib */
G_DEFINE_TYPE(Recognizer, recognizer, G_TYPE_OBJECT)
static GObject *reco;
static guint reco_signal;

/* neo or wii use ges */
static struct neo_t neo;
static struct wii_t wii;
static struct ges_3d_t ges;

/*
 * 
 */
static void recognizer_class_init(RecognizerClass *recognizer_class)
{
	reco_signal = g_signal_new ("recognized",
		G_TYPE_FROM_CLASS (recognizer_class),
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (RecognizerClass, recognized),
		0, 0,
		g_cclosure_marshal_VOID__STRING,
		G_TYPE_NONE, 
		1, G_TYPE_STRING);
	
	dbus_g_object_type_install_info(G_TYPE_FROM_CLASS(recognizer_class),
		&dbus_glib_recognizer_object_info);
}

/*
 * 
 */
static void recognizer_init (Recognizer *recognizer)
{
}

/*
 * will do absolutely nothing
 */
gboolean listen(Recognizer *recognizer, gboolean enable, GError **error)
{
	return TRUE;
}

/*
 * runs on another thread and provides services by dbus
 */
void *main_dbus(void *arg)
{
	GMainLoop *loop = 0;
	DBusGConnection *conn = 0;
	GError *error = 0;
	DBusGProxy *driver_proxy = 0;
	guint32 request_name_ret;
	
	g_type_init();
	loop = g_main_loop_new(0, FALSE);

	conn = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (conn == 0) {
		g_error("%s", error->message);
		g_error_free(error);
		pthread_exit(0);
	}
	
	reco = g_object_new(RECOGNIZER_TYPE, NULL);
	dbus_g_connection_register_g_object(conn,
		DBUS_RECOGNIZER_PATH, reco);
	
	driver_proxy = dbus_g_proxy_new_for_name(conn,
		DBUS_SERVICE_DBUS, DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
	
	if (!org_freedesktop_DBus_request_name(driver_proxy,
		DBUS_SERVICE_NAME, 0, &request_name_ret, &error)) {
		g_error("%s", error->message);
		g_error_free(error);
		pthread_exit(0);
	}
	
	if (request_name_ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		g_error ("Got result code %u from requesting name", request_name_ret);
		exit (1);
	}
	
	g_print("Dbus service running: '%s'\n", DBUS_SERVICE_NAME);
	
	g_main_loop_run (loop);
	
	return 0;
}

/*
 * callback for a recognized gesture
 */
void recognized_cb(char *id)
{
	printf("Recognized: %s\n", id);
	fflush(stdout);
	
	/* call dbus signal */
	g_signal_emit(G_OBJECT(reco), reco_signal, 0, id); 
}

/*
 * print welcome message
 */
static void print_header(void)
{
	printf("gesd: (C) 2008 OpenMoko Inc. Paul-Valentin Borza <paul@borza.ro>\n"
		"This program is free software under the terms of the GNU General Public License.\n\n");
	fflush(stdout);
}

/*
 * print all command line options 
 */
static void print_usage(void)
{
	printf("Usage: gesd --wii  --config FILE\n"
		"   or: gesd --neo2 --config FILE\n"
		"   or: gesd --neo3 --config FILE\n"
		"   or: gesd --version\n"
		"   or: gesd --help\n"
		"Remarks:\n"
		"   neo2 refers to the top accelerometer, and\n"
		"   neo3 refers to the bottom accelerometer;\n");
	fflush(stdout);
}

/*
 * print current version 
 */
static void print_version(void)
{
	printf("Version: %s\n", VERSION);
	fflush(stdout);
}

/*
 * callback for close signal for the Wii
 */
static void wii_signal_cb(int signal)
{
	switch (signal)
	{
		case SIGINT:
		case SIGTERM:
			wii_set_leds(&wii, 1, 0, 0, 0);
			wii_disconnect(&wii);
			printf("Disconnected.\n");
			fflush(stdout);
			/* clean mem */
			ges_delete_3d(&ges);
			
			exit(0);
			break;
		default:
			break;
	}
}

/*
 * callback for close signal for the Neo
 */
static void neo_signal_cb(int signal)
{
	switch (signal)
	{
		case SIGINT:
		case SIGTERM:
			neo_close(&neo);
			printf("Closed.\n");
			fflush(stdout);
			/* clean mem */
			ges_delete_3d(&ges);
			
			exit(0);
			break;
		default:
			break;
	}
}

/*
 * callback for acceleration reports
 */
void received_cb(unsigned char pressed, struct accel_3d_t accel)
{
	/* call recognizer */
	ges_process_3d(&ges, accel);
	
	if (pressed) {
		printf("%+f\t%+f\t%+f\n", accel.val[0], accel.val[1], accel.val[2]);
		fflush(stdout);
	}
}

/*
 * handshake with the devices
 */
unsigned char handshake(enum device dev)
{
	if (dev == dev_none) {
		return 0;
	}
	
	switch (dev)
	{
		case dev_wii: /* --wii */
			printf("Searching... (Press 1 and 2 on the Wii)\n");
			fflush(stdout);
			if (wii_search(&wii, 5) < 0) {
				fprintf(stderr, "Could not find the Wii.\n");
				fflush(stderr);
				return 0;
			}
			printf("Found.\n");
			fflush(stdout);
			
			if (wii_connect(&wii) < 0) {
				fprintf(stderr, "Could not connect to the Wii.\n");
				fflush(stderr);
				return 0;
			}
			printf("Connected.\n");
			fflush(stdout);

			wii_set_leds(&wii, 0, 0, 0, 1);
			
			signal(SIGINT, wii_signal_cb);
			signal(SIGTERM, wii_signal_cb);
			break;
		case dev_neo2: /* --neo2 */
			if (!neo_open(&neo, neo_accel2)) {
				fprintf(stderr, "Could not open top accelerometer.\n");
				fflush(stderr);
				return 0;
			}
			printf("Connected.\n");
			fflush(stdout);
			
			signal(SIGINT, neo_signal_cb);
			signal(SIGTERM, neo_signal_cb);
			break;
		case dev_neo3: /* --neo3 */
			if (!neo_open(&neo, neo_accel3)) {
				fprintf(stderr, "Could not open bottom accelerometer.\n");
				fflush(stderr);
				return 0;
			}
			printf("Connected.\n");
			fflush(stdout);
			
			signal(SIGINT, neo_signal_cb);
			signal(SIGTERM, neo_signal_cb);
			break;
		case dev_none:
			return 0;
			break;
	}
	/* success */
	return 1;
}

/*
 * main entry
 */
int main(int argc, char **argv)
{
	enum device dev_arg;
	char config_arg[512];
	
	/* configuration read from config_arg */
	struct config_t config;
	
	pthread_t dbus_thread;
	
	int long_opt_ind = 0;
	int long_opt_val = 0;
	
	static struct option long_opts[] = {
		{ "wii", no_argument, 0, 'w' },
		{ "neo2", no_argument, 0, 'q' },
		{ "neo3", no_argument, 0, 'z' },
		{ "config", required_argument, 0, 'c' },
		{ "version", no_argument, 0, 'v' },
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};
	
	print_header();
	
	dev_arg = dev_none;
	config_arg[0] = '\0';
	
	/* don't display errors to stderr */
	opterr = 0;
	while ((long_opt_val = getopt_long(argc, argv, "wqzc:vh", long_opts, &long_opt_ind)) != -1) 
	{
		switch (long_opt_val)
		{
			case 'w': /* --wii */
				dev_arg = (dev_arg == dev_none) ? dev_wii : dev_arg;
				break;
			case 'q': /* --neo2 */
				dev_arg = (dev_arg == dev_none) ? dev_neo2 : dev_arg;
				break;
			case 'z': /* --neo3 */
				dev_arg = (dev_arg == dev_none) ? dev_neo3 : dev_arg;
				break;
			case 'c': /* --config */
				strncpy(config_arg, optarg, sizeof(config_arg));
				config_arg[sizeof(config_arg) / sizeof(config_arg[0]) - 1] = '\0';				
				break;
			case 'v': /* --version */
				print_version();
				exit(0);
				break;
			case 'h': /* --help */
			case '?':
				print_usage();
				exit(0);
				break;
		}
	}
	
	/* minimum requirements */
	if ((dev_arg == dev_none) || (config_arg[0] == '\0'))
	{
		print_usage();
		exit(1);
	}		
	
	/* begin thread for dbus service */
	pthread_create(&dbus_thread, 0, main_dbus, 0);
	/* assign descriptors for reading accel values */
	if (!handshake(dev_arg)) {
		exit(2);
	}
	
	/* load configuration */
	if (!ges_load_config(&config, config_arg)) {
		exit(3);
	}
	ges_create_3d(&ges);
	ges_read_3d(&ges, &config);
	printf("Configuration loaded.\n");
	fflush(stdout);
	
	/* assign callbacks for recognition and acceleration */
	ges.handle_reco = recognized_cb;
	
	if (dev_arg == dev_wii) {
		/* assign callback for acceleration */
		wii.handle_recv = received_cb;
		/* begin read loop */
		wii_talk(&wii);
	} else if ((dev_arg == dev_neo2) || (dev_arg == dev_neo3)) {
		/* assign callback for acceleration */
		neo.handle_recv = received_cb;
		/* begin read loop */
		neo_begin_read(&neo);
	} else {
		exit(4);
	}
	
	/* clean mem, if we can reach here */
	ges_delete_3d(&ges);
	
	exit(0);
}
