#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus-glib-bindings.h>

#include "service.h"

/* Convenience function to print an error and exit */
static void
die (const char *prefix, GError *error) 
{
  g_error("%s: %s", prefix, error->message);
  g_error_free (error);
  exit(1);
}

static GMainLoop *loop = NULL;

/* Signal callback handling routing */
void received_im_msg_cb (DBusGProxy *purple_proxy, const char *id, 
                         gpointer user_data)
{
	//char text[1024];
	//strcpy(text, "echo signaled ");
	//strcpy(text, id);
	//strcpy(text, " >> /home/paul/openmoko/recv.txt");
	//system(text);
	printf("received signal\n");
	printf(id);
	fflush(stdout);
}

int
main (int argc, char **argv)
{
  GError *error = NULL;
  DBusGConnection *connection;
  DBusGProxy *proxy;
  char *s_out = NULL;

  g_type_init ();
  
  loop = g_main_loop_new (NULL, FALSE);

  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL)
    die ("Failed to open connection to bus", error);

  proxy = dbus_g_proxy_new_for_name_owner (connection,
                                           "org.openmoko.gestures",
                                           "/org/openmoko/gestures/Recognizer",
                                           "org.openmoko.gestures.Recognizer",
                                           &error);
  if (proxy == NULL)
    die ("Failed to create proxy for name owner", error);

  if (!org_openmoko_gestures_Recognizer_listen (proxy, TRUE, &error))
    die ("Call to echo failed", error);

/* Add the signal to the proxy */
    dbus_g_proxy_add_signal(proxy, "Recognized", 
			G_TYPE_STRING,  G_TYPE_INVALID);

    /* Connect the signal handler to the proxy */
    dbus_g_proxy_connect_signal(proxy, "Recognized",
                                G_CALLBACK(received_im_msg_cb), connection, NULL);

    /* Main loop */
    g_main_loop_run (loop);

  g_print("Got '%s'\n", s_out);
  g_free (s_out);

  return 0;
}
