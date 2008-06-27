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

#ifndef HMM_H_
#define HMM_H_

#include "gauss.h"

typedef struct hmm_3d_t {
	unsigned int state_len;
	double **trans_prob; /* state transition probability matrix */
	double *initial_prob; /* initial state probability vector */
	gauss_mix_3d_t *output_prob; /* output probability vector */
	unsigned int initial_state;
	unsigned int final_state;	
} hmm_3d_t;

/* */
double hmm_b(struct gauss_mix_3d_t *gauss_mix, struct sample_3d_t sample);
/* */
double hmm_forward_scale_alpha(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len, double scale[], double **alpha);
/* */
double hmm_forward_scale(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len, double scale[]);
/* */
double hmm_forward(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len);
/* */
void hmm_backward_scale_beta(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len, double scale[], double **beta);
/* */
//void hmm_backward_scale(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len, float scale[]);
/* */
/* */
double hmm_viterbi(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len, unsigned int decoded[]);
/* */
void hmm_baum_welch(struct hmm_3d_t *hmm, struct hmm_3d_t *hmm_est, struct sample_3d_t sample[], unsigned int sample_len);
/* */
int hmm_write_3d(struct hmm_3d_t *hmm, char *file_name);
/* */
int hmm_read_3d(struct hmm_3d_t *hmm, char *file_name);

void hmm_create_3d(struct hmm_3d_t *hmm, unsigned int state_len);

void hmm_delete_3d(struct hmm_3d_t *hmm);

void hmm_uniform_3d(struct hmm_3d_t *hmm);

void hmm_left_right_3d(struct hmm_3d_t *hmm);

void hmm_print_3d(struct hmm_3d_t *hmm);

void hmm_copy_3d(struct hmm_3d_t *hmm_dest, struct hmm_3d_t *hmm_src);

int hmm_write_gnu_plot_3d(hmm_3d_t *hmm, char *file_name);

#endif /*HMM_H_*/
