/*
 * Copyright (C) 2008 by Openmoko, Inc.
 * Written by Paul-Valentin Borza <paul@borza.ro>
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "accel.h"
#include "ges.h"

/*
 *
 */
void accel_write_3d(struct seq_3d_t *seq, char *file_name)
{
	FILE *file;
	file = fopen(file_name, "wb");
	if (file == 0)
	{
		perror("fopen");
		return;
	}

	unsigned int seq_len = seq->end - seq->begin + 1;
	fwrite(&seq_len, sizeof(unsigned int), 1, file);
	fwrite(&seq->each, sizeof(struct accel_3d_t), seq_len, file);
	
	fclose(file);
}

void accel_read_3d(struct seq_3d_t *seq, char *file_name)
{
	FILE *file;
	file = fopen(file_name, "rb");
	if (file == 0)
	{
		perror("fopen");
		return;
	}
	
	unsigned int seq_len = 0;
	fread(&seq_len, sizeof(unsigned int), 1, file);
	seq->end = seq_len - 1;
	seq->begin = 0;
	fread(&seq->each, sizeof(struct accel_3d_t), seq_len, file);

	fclose(file);
}

