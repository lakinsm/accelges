/* Generated by dbus-binding-tool; do not edit! */

#include <glib/gtypes.h>
#include <glib/gerror.h>
#include <dbus/dbus-glib.h>

G_BEGIN_DECLS

#ifndef DBUS_GLIB_CLIENT_WRAPPERS_org_openmoko_accelges_Recognizer
#define DBUS_GLIB_CLIENT_WRAPPERS_org_openmoko_accelges_Recognizer

static
#ifdef G_HAVE_INLINE
inline
#endif
gboolean
org_openmoko_accelges_Recognizer_listen (DBusGProxy *proxy, const gboolean IN_enable, GError **error)

{
  return dbus_g_proxy_call (proxy, "Listen", error, G_TYPE_BOOLEAN, IN_enable, G_TYPE_INVALID, G_TYPE_INVALID);
}

typedef void (*org_openmoko_accelges_Recognizer_listen_reply) (DBusGProxy *proxy, GError *error, gpointer userdata);

static void
org_openmoko_accelges_Recognizer_listen_async_callback (DBusGProxy *proxy, DBusGProxyCall *call, void *user_data)
{
  DBusGAsyncData *data = (DBusGAsyncData*) user_data;
  GError *error = NULL;
  dbus_g_proxy_end_call (proxy, call, &error, G_TYPE_INVALID);
  (*(org_openmoko_accelges_Recognizer_listen_reply)data->cb) (proxy, error, data->userdata);
  return;
}

static
#ifdef G_HAVE_INLINE
inline
#endif
DBusGProxyCall*
org_openmoko_accelges_Recognizer_listen_async (DBusGProxy *proxy, const gboolean IN_enable, org_openmoko_accelges_Recognizer_listen_reply callback, gpointer userdata)

{
  DBusGAsyncData *stuff;
  stuff = g_new (DBusGAsyncData, 1);
  stuff->cb = G_CALLBACK (callback);
  stuff->userdata = userdata;
  return dbus_g_proxy_begin_call (proxy, "Listen", org_openmoko_accelges_Recognizer_listen_async_callback, stuff, g_free, G_TYPE_BOOLEAN, IN_enable, G_TYPE_INVALID);
}
#endif /* defined DBUS_GLIB_CLIENT_WRAPPERS_org_openmoko_accelges_Recognizer */

G_END_DECLS
