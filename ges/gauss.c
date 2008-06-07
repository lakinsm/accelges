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
#include <stdlib.h>
#include <math.h>

#include "gauss.h"

#define MAX_ACCEL_VAL 4

/* matrix determinant of size 3x3 */
static float mat_det_3d(float mat[3][3]);
/* matrix inverse of size 3x3 */
static void mat_inv_3d(float mat[3][3], float mat_inv[3][3]);

/* 
 * allocate memory for each mixture
 */
void gauss_mix_create_3d(struct gauss_mix_3d_t *gauss_mix, unsigned int mix_len)
{
	gauss_mix->mix_len = mix_len;
	gauss_mix->weight = (float *)malloc(mix_len * sizeof(float));
	gauss_mix->single = (struct gauss_3d_t *)malloc(mix_len * sizeof(struct gauss_3d_t));
}

/*
 * de-allocate memory for each mixture
 */
void gauss_mix_delete_3d(struct gauss_mix_3d_t *gauss_mix)
{
	gauss_mix->mix_len = 0;
	free(gauss_mix->weight);
	free(gauss_mix->single);
}

/*
 * trivariate gaussian probability density function
 * Spoken language processing: section 3.1.7.3. formula 3.82. page 93.
 */
float gauss_prob_den_3d(struct gauss_3d_t *gauss, struct sample_3d_t sample)
{
	float mat_det = mat_det_3d(gauss->covar);
	float mat_inv[3][3];
	mat_inv_3d(gauss->covar, mat_inv);
	
	float accel1[3];
	accel1[0] = sample.val[0] - gauss->mean[0];
	accel1[1] = sample.val[1] - gauss->mean[1];
	accel1[2] = sample.val[2] - gauss->mean[2];
	
	/* Mahalanobis distance */
	float mahalanobis_dis =
		(accel1[0] * mat_inv[0][0] + accel1[1] * mat_inv[1][0] + accel1[2] * mat_inv[2][0]) * accel1[0] +
		(accel1[0] * mat_inv[0][1] + accel1[1] * mat_inv[1][1] + accel1[2] * mat_inv[2][1]) * accel1[1] +
		(accel1[0] * mat_inv[0][2] + accel1[1] * mat_inv[1][2] + accel1[2] * mat_inv[2][2]) * accel1[2];
	
	float pdf = powf(M_E, mahalanobis_dis / - 2.0) / (powf(2 * M_PI, 1.5) * sqrtf(mat_det));
	 
	return pdf;
}

/*
 * trivariate gaussian discriminant function
 * Spoken language processing: section 4.2.1. formula 4.18. page 142.
 */
float gauss_disc_3d(struct gauss_3d_t *gauss, struct sample_3d_t sample, float prior_prob)
{
	return logf(gauss_prob_den_3d(gauss, sample)) + logf(prior_prob);
}

/*
 * 
 */
void gauss_rand_3d(struct gauss_3d_t *gauss)
{
	int i, j;
	
	for (i = 0; i < 3; i++)
	{
		gauss->mean[i] = rand() % MAX_ACCEL_VAL;
	}
	
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			gauss->covar[i][j] = rand() % MAX_ACCEL_VAL;
		}
	}
}

/*
 * print trivariate gaussian parameters to standard output
 */
void gauss_print_3d(struct gauss_3d_t *gauss)
{
	int i, j;
	printf("Means:\n");
	for (i = 0; i < 3; i++)
	{
		printf("%f\t", gauss->mean[i]);
	}
	printf("\n");
	printf("Covariances:\n");
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			printf("%f\t", gauss->covar[i][j]);
		}
		printf("\n");
	}
}

/*
 * trivariate gaussian mixture probability density function
 * Spoken language processing: section 3.1.7.3. formula 3.86. page 95
 */
float gauss_mix_prob_den_3d(struct gauss_mix_3d_t *gauss_mix, struct sample_3d_t sample)
{
	float pdf = 0;
	int i;
	
	for (i = 0; i < gauss_mix->mix_len; i++)
		pdf += gauss_mix->weight[i] * gauss_prob_den_3d(&gauss_mix->single[i], sample);
		
	return pdf; 	
}

/*
 * trivariate gaussian mixture discriminant function
 * Spoken language processing: section 4.2.1. formula 4.18. page 142.
 */
float gauss_mix_disc_3d(struct gauss_mix_3d_t *gauss_mix, struct sample_3d_t sample, float prior_prob)
{
	return logf(gauss_mix_prob_den_3d(gauss_mix, sample)) + logf(prior_prob);
}

/*
 * trivariate gaussian mixture density estimation
 * Spoken language processing: section 4.4.3. formula 4.106/7/8. page 175.
 */
void gauss_mix_den_est_3d(struct gauss_mix_3d_t *gauss_mix, struct gauss_mix_3d_t *gauss_mix_est, struct sample_3d_t sample[], unsigned int sample_len)
{
	unsigned int mix_len = gauss_mix->mix_len;
	/* quantities */
	float quan_2d[mix_len][sample_len];
	float quan_1d[mix_len];
	float quan_0d;
	int k, i, p, q;
	
	/* 4.103. */
	for (k = 0; k < mix_len; k++)
	{
		for (i = 0; i < sample_len; i++)
		{
			quan_2d[k][i] = gauss_mix->weight[k] * gauss_prob_den_3d(&gauss_mix->single[k], sample[i]) / gauss_mix_prob_den_3d(gauss_mix, sample[i]);
		}
	}
			
	/* 4.104. */
	for (k = 0; k < mix_len; k++)
	{
		quan_1d[k] = 0.0;
		for (i = 0; i < sample_len; i++)
		{
			quan_1d[k] += quan_2d[k][i];
		} 
	}
	
	/* this is N in 4.106. */
	quan_0d = 0.0;
	for (k = 0; k < mix_len; k++)
	{
		quan_0d += quan_1d[k];
	}
		
	/* 4.106. */
	for (k = 0; k < mix_len; k++)
	{
		gauss_mix_est->weight[k] = quan_1d[k] / quan_0d;
	}
	
	/* 4.107. */	
	for (k = 0; k < mix_len; k++)
	{
		for (p = 0; p < 3; p++)
		{
			float sum = 0.0;
			for (i = 0; i < sample_len; i++)
			{
				sum += quan_2d[k][i] * sample[i].val[p];
			}
			gauss_mix_est->single[k].mean[p] = sum / quan_1d[k];
		}
	}
	
	/* 4.108. */
	for (k = 0; k < mix_len; k++)
	{
		for (p = 0; p < 3; p++)
		{
			for (q = 0; q < 3; q++)
			{
				float sum = 0.0;
				for (i = 0; i < sample_len; i++)
				{
					sum += quan_2d[k][i] * (sample[i].val[p] - gauss_mix->single[k].mean[p]) *
						(sample[i].val[q] - gauss_mix->single[k].mean[q]); 
				}
				gauss_mix_est->single[k].covar[p][q] = sum / quan_1d[k];
			}
		}
	}
	
	/* done */
}

/*
 * 
 */
void gauss_mix_rand_3d(struct gauss_mix_3d_t *gauss_mix)
{
	int i;
	float sum = 0;
	
	for (i = 0; i < gauss_mix->mix_len; i++)
	{
		gauss_mix->weight[i] = rand();
		sum += gauss_mix->weight[i];
	}
	
	/* normalize to interval [0, 1] */
	for (i = 0; i < gauss_mix->mix_len; i++)
	{
		gauss_mix->weight[i] /= sum;
	}
	
	for (i = 0; i < gauss_mix->mix_len; i++)
	{
		gauss_rand_3d(&gauss_mix->single[i]);
	}
}

/*
 * 
 */
void gauss_mix_copy_3d(struct gauss_mix_3d_t *gauss_mix, struct gauss_mix_3d_t *gauss_mix_copy)
{
	int i;
	
	if (gauss_mix->mix_len != gauss_mix_copy->mix_len)
	{
		return;
	}
	
	for (i = 0; i < gauss_mix->mix_len; i++)
	{
		gauss_mix_copy->weight[i] = gauss_mix->weight[i];
		gauss_mix_copy->single[i] = gauss_mix->single[i];
	}
}

/*
 * print trivariate gaussian mixture parameters to standard output 
 */
void gauss_mix_print_3d(struct gauss_mix_3d_t *gauss_mix)
{
	unsigned int mix_len = gauss_mix->mix_len;
	int k;
	printf("Mixtures: %d\n", mix_len);
	printf("Weights:\n");
	for (k = 0; k < mix_len; k++)
	{
		printf("%f\t", gauss_mix->weight[k]);
	}
	printf("\n");
	for (k = 0; k < mix_len; k++)
	{
		printf("For gaussian mixture %d:\n", k);
		gauss_print_3d(&gauss_mix->single[k]);
		printf("\n");
	}
}

/*
 * write trivariate gaussian mixture to file
 */
int gauss_mix_write_3d(struct gauss_mix_3d_t *gauss_mix, char *file_name)
{
	unsigned int mix_len = gauss_mix->mix_len;
	FILE *file;
	file = fopen(file_name, "wb");
	if (file == 0)
	{
		perror("fopen");
		return -1;
	}
	
	fwrite(&gauss_mix->mix_len, sizeof(unsigned int), 1, file);
	fwrite(gauss_mix->weight, sizeof(float), mix_len, file);
	fwrite(gauss_mix->single, sizeof(struct gauss_3d_t), mix_len, file);
	
	fclose(file);
	
	return 0;
}

/*
 * read trivariate gaussian mixture from file
 */
int gauss_mix_read_3d(struct gauss_mix_3d_t *gauss_mix, char *file_name)
{
	unsigned int mix_len;
	FILE *file;
	file = fopen(file_name, "rb");
	if (file == 0)
	{
		perror("fopen");
		return -1;
	}
	
	fread(&gauss_mix->mix_len, sizeof(unsigned int), 1, file);
	mix_len = gauss_mix->mix_len;
	gauss_mix_create_3d(gauss_mix, mix_len); /* create arrays */
	fread(gauss_mix->weight, sizeof(float), mix_len, file);
	fread(gauss_mix->single, sizeof(struct gauss_3d_t), mix_len, file);
	
	fclose(file);
	
	return 0;
}

/*
 * matrix determinant of size 3x3
 * http://mathworld.wolfram.com/Determinant.html
 */
static float mat_det_3d(float mat[3][3])
{		
	float mat_det =
		mat[0][0] * mat[1][1] * mat[2][2] +
		mat[0][1] * mat[1][2] * mat[2][0] +
		mat[0][2] * mat[1][0] * mat[2][1] -
		mat[0][2] * mat[1][1] * mat[2][0] -
		mat[0][1] * mat[1][0] * mat[2][2] -
		mat[0][0] * mat[2][1] * mat[1][2];
	
	return mat_det;
}

/*
 * matrix inverse of size 3x3
 * http://mathworld.wolfram.com/MatrixInverse.html
 */
static void mat_inv_3d(float mat[3][3], float mat_inv[3][3])
{
	float mat_det = mat_det_3d(mat);
	float mat_det_inv = 1 / mat_det;
	
	mat_inv[0][0] = mat_det_inv * (mat[1][1] * mat[2][2] - mat[1][2] * mat[2][1]);
	mat_inv[0][1] = mat_det_inv * (mat[0][2] * mat[2][1] - mat[0][1] * mat[2][2]);
	mat_inv[0][2] = mat_det_inv * (mat[0][1] * mat[1][2] - mat[0][2] * mat[1][1]);
	mat_inv[1][0] = mat_det_inv * (mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2]);
	mat_inv[1][1] = mat_det_inv * (mat[0][0] * mat[2][2] - mat[0][2] * mat[2][0]);
	mat_inv[1][2] = mat_det_inv * (mat[0][2] * mat[1][0] - mat[0][0] * mat[1][2]);
	mat_inv[2][0] = mat_det_inv * (mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]);
	mat_inv[2][1] = mat_det_inv * (mat[0][1] * mat[2][0] - mat[0][0] * mat[2][1]);
	mat_inv[2][2] = mat_det_inv * (mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0]);
}
