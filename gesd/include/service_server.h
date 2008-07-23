#ifndef SERVICE_SERVER_H_
#define SERVICE_SERVER_H_

#include <glib.h>
#include <glib-object.h>

#define APP_SERVICE_NAME           "ro.borza.gesd"
#define APP_PATH_NAME              "/ro/borza/gesd/Ges"
#define APP_INTERFACE_NAME         "ro.borza.gesd.App"
#define GES_TYPE_APPLICATION ( ges_service_get_type () )

G_BEGIN_DECLS

typedef struct _GesApplication {
        GObject object;
} GesApplication;

typedef struct _GesApplicationClass {
        GObjectClass object_class;
         
        void (*recognized) (GesApplication *obj, const gchar *gesture);
          
} GesApplicationClass;

gboolean
gesd_listenfor (GesApplication *obj, const gchar *gesture, GError **error);

G_END_DECLS

#endif /*SERVICE_SERVER_H_*/
