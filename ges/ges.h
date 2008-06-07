/*
 * Copyright (C) 2008 by OpenMoko, Inc.
 * Written by Paul-Valentin Borza <gestures@borza.ro>
 * All Rights Reserved
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
 
#ifndef GES_H_
#define GES_H_

#include "class.h"
#include "gauss.h"

/* should hold a continuous gesture of 5 min (which is a lot) at 100 Hz */
#define FRAME_LEN 30000
#define FRAME_DIF 5

typedef struct config_t {
	char noise_file_name[1024];
	char motion_file_name[1024];
} config_t;

typedef struct accel_3d_t {
	float val[3];
} accel_3d_t;

typedef struct ges_3d_t {
	struct accel_3d_t frame[FRAME_LEN];
	unsigned int index;
	unsigned int begin;
	unsigned int end;
	unsigned char detected;
	struct class_2c_t endpoint;
} ges_t;

/* populate ges structure with manual values */
void ges_populate_3d(struct ges_3d_t *ges);
/* allocate memory for class mixtures */
void ges_create_3d(struct ges_3d_t *ges);
/* de-allocate memory for class mixtures */
void ges_delete_3d(struct ges_3d_t *ges);
/* read ges structure from files */
void ges_read_3d(struct ges_3d_t *ges, struct config_t *config);
/* write ges structure to files */
void ges_write_3d(struct ges_3d_t *ges, struct config_t *config);
/* load configuration file */
unsigned char ges_load_config(struct config_t *config, char *file_name);
/* process accelerometer values */
void ges_process_3d(struct ges_3d_t *ges, struct accel_3d_t accel);

#endif /*GES_H_*/
