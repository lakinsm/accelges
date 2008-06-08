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

#include <stdio.h>

#include "class.h"
#include "gauss.h"

/*
 * return the index of the maximum probability between the two classes 
 */
unsigned int class_max_2c(struct class_2c_t *class, struct sample_3d_t sample)
{
	//if (!class)
	//{
	//	fprintf(stderr, "Null arguments inside %s at line %d.\n", __FILE__, __LINE__);
	//}
	
 	float prob_0c = gauss_mix_disc_3d(&class->each[0], sample, class->prior_prob[0]);
	float prob_1c = gauss_mix_disc_3d(&class->each[1], sample, class->prior_prob[1]);
	
	if (prob_0c > prob_1c)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
