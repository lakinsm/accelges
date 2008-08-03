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

#define CLASS_TIME 60

/* configuration type */
typedef struct config_t {
	char sclass_file[512];
	char dclass_file[512];
	/* probabilities */
	double sclassp;
	double dclassp;
	/* classes */
	unsigned int class_len; /* < 128 */
	char class_file[128][512];
	char class_id[128][512];
	/* models */
	unsigned int model_len; /* < 128*/
	char model_file[128][512];
	char model_id[128][512];
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
	int till_end; /* consecutive frames till declared end of motion */
} frame_t;

/* gesture type */
typedef struct ges_3d_t {
	struct seq_3d_t seq;
	unsigned char detected;
	struct class_2c_t endpoint;
	
	/* max 100 classes */
	unsigned int class_len;
	struct gauss_mix_3d_t class[128];
	char class_cmd[100][512];
	int prev_class_ind;
	unsigned char prev_class_change;
	int prev_class_time;
	/* max 100 models */
	unsigned int model_len;
	struct hmm_3d_t model[128];
	char model_cmd[100][512];
	
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
