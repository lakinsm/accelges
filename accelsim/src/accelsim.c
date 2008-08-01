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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#include "accel.h"
#include "accelsim.h"

/*
 * opens file to read from
 */
unsigned char sim_open(struct sim_t *sim, char *filename)
{
	accel_read_3d(&sim->seq, filename);
	return 1;

}

/*
 * closes the file
 */
void sim_close(struct sim_t *sim)
{
}

/*
 * begins reading the file
 */
void sim_begin_read(struct sim_t *sim)
{
	int i;
	for (i = sim->seq.begin; i <= sim->seq.end; i++)
	{
		if (sim->handle_recv) {
			sim->handle_recv(1, sim->seq.each[i]);
		}
	}
}

