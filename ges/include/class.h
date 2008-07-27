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

#ifndef CLASS_H_
#define CLASS_H_

#include "gauss.h"

typedef struct class_2c_t {
	double prior_prob[2];
	struct gauss_mix_3d_t each[2];
} class_2c_t;

/* return the index of the maximum probability between the two classes */
unsigned int class_max_2c(struct class_2c_t *class, struct sample_3d_t sample);

unsigned int class_max_uc(struct gauss_mix_3d_t gauss_mix[], unsigned int gauss_mix_len, struct sample_3d_t sample);

#endif /*CLASS_H_*/
