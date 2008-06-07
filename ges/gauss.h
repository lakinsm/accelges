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
 
#ifndef GAUSS_H_
#define GAUSS_H_

typedef struct gauss_mix_3d_t {
	unsigned int mix_len; /* number of mixtures */
	float *weight; /* weight coefficients */
	struct gauss_3d_t *single;
} gauss_mix_3d_t;

typedef struct gauss_3d_t {
	float mean[3]; /* mean vector */
	float covar[3][3]; /* covariance matrix */
} gauss_3d_t;

typedef struct sample_3d_t {
	float val[3];
} sample_3d_t;

/* allocate memory for each mixture */
void gauss_mix_create_3d(struct gauss_mix_3d_t *gauss_mix, unsigned int mix_len);
/* de-allocate memory for each mixture */
void gauss_mix_delete_3d(struct gauss_mix_3d_t *gauss_mix);

/* trivariate gaussian probability density function */
float gauss_prob_den_3d(struct gauss_3d_t *gauss, struct sample_3d_t sample);
/* trivariate gaussian discriminant function */
float gauss_disc_3d(struct gauss_3d_t *gauss, struct sample_3d_t sample, float prior_prob);

/* print trivariate gaussian parameters to standard output */
void gauss_print_3d(struct gauss_3d_t *gauss);

/* trivariate gaussian mixture probability density function */
float gauss_mix_prob_den_3d(struct gauss_mix_3d_t *gauss_mix, struct sample_3d_t sample);
/* trivariate gaussian mixture discriminant function */
float gauss_mix_disc_3d(struct gauss_mix_3d_t *gauss_mix, struct sample_3d_t sample, float prior_prob);
/* trivariate gaussian mixture density estimation */
void gauss_mix_den_est_3d(struct gauss_mix_3d_t *gauss_mix, struct gauss_mix_3d_t *gauss_mix_est, struct sample_3d_t sample[], unsigned int sample_len);

/* print trivariate gaussian mixture parameters to standard output */
void gauss_mix_print_3d(struct gauss_mix_3d_t *gauss_mix);

/* write trivariate gaussian mixture to file */
int gauss_mix_write_3d(struct gauss_mix_3d_t *gauss_mix, char *file_name);
/* read trivariate gaussian mixture from file */
int gauss_mix_read_3d(struct gauss_mix_3d_t *gauss_mix, char *file_name);

#endif /*GAUSS_H_*/
