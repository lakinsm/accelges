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
#include "hmm.h"

/* should hold a continuous gesture of 5 min (which is a lot) at 100 Hz */
#define FRAME_LEN 30000
/* 1/5 seconds */
#define FRAME_DIF 20
/* also send to the recognizer some more frames, before the ones classified */
#define FRAME_BEFORE 10
/* FRAME_AFTER is not used in this release, it would have to wait for those frames */
/* also send to the recognizer some more frames, after the ones classified */
#define FRAME_AFTER 10

/* configuration type */
typedef struct config_t {
	char noise_file_name[1024];
	char motion_file_name[1024];
	unsigned int model_len;
	/* max 100 models; TODO: should be dynamically allocated */
	char model_file_name[100][1024];
	char model_cmd[100][1024];
} config_t;

#ifndef ACCEL_T
#define ACCEL_T
/* acceleration type */
typedef struct accel_3d_t {
	float val[3];
} accel_3d_t;
#endif

/* geture recognized callback */
typedef void (* ges_reco_cb)(char *result);

/* sequence type */
typedef struct seq_3d_t {
	struct accel_3d_t each[FRAME_LEN];
	unsigned int index;
	unsigned int begin;
	unsigned int end;
} frame_t;

/* gesture type */
typedef struct ges_3d_t {
	struct seq_3d_t seq;
	unsigned char detected;
	struct class_2c_t endpoint;
	unsigned int model_len;
	/* max 100 models */
	struct hmm_3d_t model[100];
	char model_cmd[100][1024];
	ges_reco_cb handle_reco;
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
/* create feature vector */
unsigned char ges_fea_3d(struct seq_3d_t *seq, unsigned int seq_index, unsigned int seq_prev_index, struct sample_3d_t *sample);

#endif /*GES_H_*/
