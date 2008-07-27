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

#ifndef RECOGNIZER_H_
#define RECOGNIZER_H_

#include <glib.h>
#include <glib-object.h>

typedef struct Recognizer
{
	GObject parent;
} Recognizer;

typedef struct RecognizerClass
{
	GObjectClass parent;
	  
	void (*recognized) (Recognizer *recognizer, const gchar *id);
} RecognizerClass;

/* */
GType recognizer_get_type(void);

#define RECOGNIZER_TYPE              (recognizer_get_type ())
#define RECOGNIZER(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), RECOGNIZER_TYPE, Recognizer))
#define RECOGNIZER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), RECOGNIZER_TYPE, RecognizerClass))
#define IS_RECOGNIZER(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), RECOGNIZER_TYPE))
#define IS_RECOGNIZER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), RECOGNIZER_TYPE))
#define RECOGNIZER_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), RECOGNIZER_TYPE, RecognizerClass))

/* */
gboolean listen(Recognizer *recognizer, gboolean enable, GError **error);

#endif /*RECOGNIZER_H_*/
