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

#ifndef ACCELSIM_H_
#define ACCELSIM_H_

#include <stdio.h>

#include "ges.h"

#ifndef ACCEL_T
#define ACCEL_T
/* acceleration */
typedef struct accel_3d_t {
	float val[3];
} accel_3d_t;

#endif

#ifndef HANDLE_ACCEL_T
#define HANDLE_ACCEL_T
/* handle for acceleration */
typedef void (* handle_recv_3d_t)(unsigned char pressed, struct accel_3d_t accel);

#endif

typedef struct sim_t {
	struct seq_3d_t seq;
	/* callback for acceleration */
	handle_recv_3d_t handle_recv;
} sim_t;

/* opens the file to simulate from */
unsigned char sim_open(struct sim_t *sim, char *filename);
/* closes the file */
void sim_close(struct sim_t *sim);
/* begins reading accelerometer values from file */
void sim_begin_read(struct sim_t *sim);

#endif /*ACCELSIM_H_*/
