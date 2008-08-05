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
#include <string.h>
#include <unistd.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <glib.h>
#include "service.h"

#define DBUS_SERVICE_NAME "org.openmoko.accelges"
#define DBUS_RECOGNIZER_PATH "/org/openmoko/accelges/Recognizer"
#define DBUS_RECOGNIZER_NAME "org.openmoko.accelges.Recognizer"

//#define DBUS_OPHONED_NAME "org.freesmartphone.ophoned"
//#define DBUS_GSM_DEVICE_PATH "/org/freesmartphone/GSM/Device"
#define DBUS_GSM_CALL_NAME "org.freesmartphone.GSM.Call"

static DBusHandlerResult signal_filter 
      (DBusConnection *connection, DBusMessage *message, void *user_data);

static void power_up_screen(void)
{
	system("echo 0 > bl_power");
}

void call_status_cb(DBusGProxy *proxy, int *index,
	const char *state, GHashTable *properties, gpointer user_data)
{
	printf("Call status: %s\n", state);
}
/*
 * 
 */
void recognized_cb(DBusGProxy *proxy, const char *id, 
	gpointer user_data)
{
	printf("Received: %s\n", id);
	fflush(stdout);
	if (strcmp(id, "screen_zzp") == 0) {
		/* sits on the table, screen up */
		system("xrandr -o normal");
	} else if (strcmp(id, "screen_zzn") == 0) {
		/* sits on the table, screen down */
		system("xrandr -o normal");
	} else if (strcmp(id, "screen_zpz") == 0) {
		/* hold vertically, with the hole up (speaker is down) */
		system("xrandr -o inverted");
	} else if (strcmp(id, "screen_znz") == 0) {
		/* hold vertically, with the hole down (speaker is up) */
		system("xrandr -o normal");
	} else if (strcmp(id, "screen_pzz") == 0) {
		/* sits on the table, and the side with the usb is down (you can't see it) */
		system("xrandr -o left");
	} else if (strcmp(id, "screen_nzz") == 0) {
		/* sits on the table, and the side with the usb is up (you can see it) */
		system("xrandr -o right");
	} else if (strcmp(id, "screen_npp") == 0) {
		/* sits horizontally, tilted at 45 deg, screen faces user, usb is up */
		system("xrandr -o right");
	} else if (strcmp(id, "screen_nnp") == 0) {
		/* vertical position, at 45 deg, usb is in the right */
		system("xrandr -o normal");
	} else if (strcmp(id, "screen_pnp") == 0) {
		/* sits horizontally, tilted at 45 deg, screen faces user, usb is down */
		system("xrandr -o left");
	} else if (strcmp(id, "screen_ppp") == 0) {
		/* vertical position, at 45 deg, usb is in the left */
		system("xrandr -o inverted");
	} else if (strcmp(id, "left") == 0) {
		system("gst-launch filesrc location=/etc/accelges/neo2/left.mp3 "
			"! mad ! audioconvert ! alsasink");
	} else if (strcmp(id, "right") == 0) {
		system("gst-launch filesrc location=/etc/accelges/neo2/right.mp3 "
			"! mad ! audioconvert ! alsasink");
	} else if (strcmp(id, "up") == 0) {
		system("gst-launch filesrc location=/etc/accelges/neo2/up.mp3 "
			"! mad ! audioconvert ! alsasink");
	} else if (strcmp(id, "down") == 0) {
		system("gst-launch filesrc location=/etc/accelges/neo2/down.mp3 "
			"! mad ! audioconvert ! alsasink");
	} else if (strcmp(id, "hor_circle") == 0) {
		system("gst-launch filesrc location=/etc/accelges/neo2/hor_circle.mp3 "
			"! mad ! audioconvert ! alsasink");
	} else if (strcmp(id, "ver_circle") == 0) {
		system("gst-launch filesrc location=/etc/accelges/neo2/ver_circle.mp3 "
			"! mad ! audioconvert ! alsasink");
	} else {
		printf("Gesture '%s'\n", id);
		fflush(stdout);
	}
}

/*
 * print welcome message
 */
static void print_header(void)
{
	printf("gesl: (C) 2008 OpenMoko Inc. Paul-Valentin Borza <paul@borza.ro>\n"
		"This program is free software under the terms of the GNU General Public License.\n\n");
	fflush(stdout);
}


static DBusHandlerResult
signal_filter (DBusConnection *connection, DBusMessage *message, void *user_data)
{
	//printf("Received from interface '%s', member '%s'\n", 
	//	dbus_message_get_interface(message), dbus_message_get_member(message));
	//fflush(stdout);
	
	/* User data is the event loop we are running in */
	GMainLoop *loop = user_data;
	/* A signal from the bus saying we are about to be disconnected */
	if (dbus_message_is_signal 
		(message, "org.freedesktop.Local", "Disconnected")) {
			/* Tell the main loop to quit */
			g_main_loop_quit (loop);
			/* We have handled this message, don't pass it on */
			return DBUS_HANDLER_RESULT_HANDLED;
	}
	/* A Ping signal on the com.burtonini.dbus.Signal interface */
	else if (dbus_message_is_signal (message, DBUS_GSM_CALL_NAME, "CallStatus")) {
		//printf("CALL STATUS RECEIVED\n");
		//fflush(stdout);
		DBusMessageIter iter;
		DBusError error;
		int index = 0;
		const char *status;
	  dbus_error_init (&error);

		//dbus_message_iter_init (&message, &iter);
		//while ((dbus_message_iter_get_arg_type (&iter)) != DBUS_TYPE_INVALID)
		//{
				//if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_INT32)
				//{
					//dbus_message_iter_get_basic(&iter, &index);
				//}	
					//dbus_message_iter_next (&iter);
				//if (dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
				//{
					//dbus_message_iter_get_basic(&iter, &status);
				//}
				//dbus_message_iter_next (&iter);
		//}
		//printf("%d, %s\n", index, status);
	  /*
		if (dbus_message_get_args 
	  	(message, &error, DBUS_TYPE_INT32, &index, DBUS_TYPE_STRING, &status, DBUS_TYPE_INVALID)) {
		  	g_print("Status received: %s\n", status);
				dbus_free (status);
		} else {
			g_print("Ping received, but error getting message: %s\n", error.message);
			dbus_error_free (&error);
		}
		*/
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

/*
 * 
 */
int main (int argc, char **argv)
{
	GMainLoop *loop = 0;
	DBusGConnection *conn = 0;
	DBusConnection *conn2 = 0;
	GError *error = 0;
	DBusError error2;
	DBusGProxy *proxy;
	DBusGProxy *proxy2;

	print_header();
		
	g_type_init ();
	
	loop = g_main_loop_new (0, FALSE);

	conn = dbus_g_bus_get (DBUS_BUS_SYSTEM, &error);
	if (conn == 0) {
		g_error("%s", error->message);
		g_error_free(error);
		exit(1);
	}
	
	proxy = dbus_g_proxy_new_for_name_owner(conn,
		DBUS_SERVICE_NAME, DBUS_RECOGNIZER_PATH, DBUS_RECOGNIZER_NAME,
		&error);
	
	if (proxy == 0) {
		g_error("%s", error->message);
		g_error_free(error);
		exit(2);
	}
	
//	proxy2 = dbus_g_proxy_new_for_name_owner(conn,
//		DBUS_OPHONED_NAME, DBUS_GSM_DEVICE_PATH, DBUS_GSM_DEVICE_CALL_NAME, &error);

//	if (proxy2 == 0) {
//		g_error("%s", error->message);
//		g_error_free(error);
//		exit(3);
//	}

	if (!org_openmoko_accelges_Recognizer_listen (proxy, TRUE, &error)) {
		g_error("%s", error->message);
		g_error_free(error);
		exit(3);
	}

	/* Add the signal to the proxy */
	dbus_g_proxy_add_signal(proxy, "Recognized", 
		G_TYPE_STRING,  G_TYPE_INVALID);
	
	/* Connect the signal handler to the proxy */
	dbus_g_proxy_connect_signal(proxy, "Recognized",
		G_CALLBACK(recognized_cb), conn, 0);
	
//	dbus_g_proxy_add_signal(proxy2, "CallStatus",
//		G_TYPE_INT, G_TYPE_STRING, dbus_g_type_get_map ("GHashTable", G_TYPE_STRING, G_TYPE_VALUE), G_TYPE_INVALID);

//	dbus_g_proxy_connect_signal(proxy2, "CallStatus",
//		G_CALLBACK(call_status_cb), conn, 0);
	
	dbus_error_init(&error2);
	conn2 = dbus_bus_get(DBUS_BUS_SYSTEM, &error2);
	if (!conn2)
	{
		g_warning ("Failed to connect to the D-BUS daemon: %s", error2.message);
		dbus_error_free (&error2);
	}
	dbus_connection_setup_with_g_main (conn2, NULL);

	/* listening to messages from all objects as no path is specified */
	dbus_bus_add_match (conn2, "type='signal', interface='" DBUS_GSM_CALL_NAME "'", &error2);
	dbus_connection_add_filter (conn2, signal_filter, loop, 0);


	g_print("Listening for signals on: '%s'\n", DBUS_SERVICE_NAME);
	
	g_main_loop_run(loop);
	
	return 0;
}
