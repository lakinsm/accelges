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

#include "service.h"

#define DBUS_SERVICE_NAME "org.openmoko.accelges"
#define DBUS_RECOGNIZER_PATH "/org/openmoko/accelges/Recognizer"
#define DBUS_RECOGNIZER_NAME "org.openmoko.accelges.Recognizer"

#define DBUS_OPHONED_NAME "org.freesmartphone.ophoned"
#define DBUS_GSM_DEVICE_PATH "/org/freesmartphone/GSM/Device"
#define DBUS_GSM_DEVICE_CALL_NAME "org.freesmartphone.GSM.Device.Call"

static void power_up_screen(void)
{
	system("echo 0 > bl_power");
}

void call_status_cb(DBusGProxy *proxy, int index,
	char *status, GObject *properties, gpointer user_data)
{
	printf("Status: %s\n", status);
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

/*
 * 
 */
int main (int argc, char **argv)
{
	GMainLoop *loop = 0;
	DBusGConnection *conn = 0;
	GError *error = 0;
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
	
	proxy2 = dbus_g_proxy_new_for_name_owner(conn,
		DBUS_OPHONED_NAME, DBUS_GSM_DEVICE_PATH, DBUS_GSM_DEVICE_CALL_NAME, &error);

	if (proxy2 == 0) {
		g_error("%s", error->message);
		g_error_free(error);
		exit(3);
	}

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
	
	dbus_g_proxy_add_signal(proxy2, "CallStatus",
		G_TYPE_INT, G_TYPE_STRING, G_TYPE_OBJECT, G_TYPE_INVALID);

	dbus_g_proxy_connect_signal(proxy2, "CallStatus",
		G_CALLBACK(call_status_cb), conn, 0);

	g_print("Listening for signals on: '%s'\n", DBUS_SERVICE_NAME);
	
	g_main_loop_run(loop);
	
	return 0;
}
