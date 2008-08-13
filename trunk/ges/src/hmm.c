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
//#define NDEBUG

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gauss.h"
#include "hmm.h"
#include "math.h"

/*
 * 
 */
double hmm_b(struct gauss_mix_3d_t *gauss_mix, struct sample_3d_t sample)
{
	double result = gauss_mix_prob_den_3d(gauss_mix, sample);
	//if (result < 1.0e-30)
	//	result = 1.0e-30;
	return result;
}

/*
 * rabiner's
 */
double hmm_forward_scale_alpha(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len, double scale[], double **alpha)
{	
	double forward;
	double scale_sum;
	int i, j, t;
		
	for (i = 0; i < hmm->state_len; i++)
	{
		alpha[0][i] = hmm->initial_prob[i] * hmm_b(&hmm->output_prob[i], sample[0]);
		assert(!isnan(alpha[0][i]));
	}
	
	/* scaling 92a, begin initial scale */
	scale_sum = 0.0;
	for (i = 0; i < hmm->state_len; i++)
	{
		scale_sum += alpha[0][i];
	}
	scale[0] = 1.0 / scale_sum;
	for (i = 0; i < hmm->state_len; i++)
	{
		alpha[0][i] *= scale[0];
		assert(!isnan(alpha[0][i]));
	}
	/* end initial scale */
	
	for (t = 1; t < sample_len; t++)
	{
		for (j = 0; j < hmm->state_len; j++)
		{
			double sum = 0.0;
			for (i = 0; i < hmm->state_len; i++)
			{
				sum += alpha[t - 1][i] * hmm->trans_prob[i][j];
			}
			alpha[t][j] = sum * hmm_b(&hmm->output_prob[j], sample[t]);
			//printf("%e * %e\n", sum, hmm_b(&hmm->output_prob[j], sample[t]));
			assert(!isnan(alpha[t][j]));
		}
		
		/* once computed, begin scale at each time frame */
		/* needed to prevent underflow */
		scale_sum = 0.0;
		for (i = 0; i < hmm->state_len; i++)
		{
			scale_sum += alpha[t][i];
		}
		scale[t] = 1.0 / scale_sum;
		for (i = 0; i < hmm->state_len; i++)
		{
			alpha[t][i] *= scale[t];
			assert(!isnan(alpha[t][i]));
		}
		/* end scale at each time frame */
	}
	
	/* use this when you don't know the final state */
	forward = 0.0;
	for (t = 0; t < sample_len; t++)
	{
		forward += log(scale[t]);
	}
	/* without scaling */
	//return forward;
	/* with scaling */
	return log(alpha[sample_len - 1][hmm->state_len - 1]) - forward;
	//return -1 * forward;
	/* but, when it's required to end in the final state, use this */
	//return alpha[sample_len - 1][hmm->final_state];
}

/*
 * 
 */
double hmm_forward_scale(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len, double scale[])
{
	//float alpha[sample_len][hmm->state_len];
	double **alpha;
	int t;
	alpha = (double **)malloc(sample_len * sizeof(double *));
	for (t = 0; t < sample_len; t++)
	{
		alpha[t] = (double *)malloc(hmm->state_len * sizeof(double));
	}
	/* cast from array of array to pointer of pointer */
	double forward = hmm_forward_scale_alpha(hmm, sample, sample_len, scale, alpha);
	
	for (t = 0; t < sample_len; t++)
	{
		free(alpha[t]);
	}
	free(alpha);
	
	return forward;
}

/*
 * made with Rabiner's formulas
 */
double hmm_forward(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len)
{
	double scale[sample_len];
	return hmm_forward_scale(hmm, sample, sample_len, scale);
}

/*
 * 
 */
void hmm_backward_scale_beta(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len, double scale[], double **beta)
{
	//float beta[sample_len][hmm->state_len];
	int i, j, t;
	for (i = 0; i < hmm->state_len; i++)
	{
		beta[sample_len - 1][i] = 1.0; /* in rabiner */
		//beta[sample_len - 1][i] = 1.0 / hmm->state_len; /* slp */
		assert(!isnan(beta[sample_len - 1][i]));
	}
	
	for (i = 0; i < hmm->state_len; i++)
	{
		beta[sample_len - 1][i] *= scale[sample_len - 1];
		assert(!isnan(beta[sample_len - 1][i]));
	}
	
	/* when you use unsigned int t, modify to t + 1 > 0 */
	for (t = sample_len - 2; t > -1; t--)
	{
		for (i = 0; i < hmm->state_len; i++)
		{
			double sum = 0.0;
			for (j = 0; j < hmm->state_len; j++)
			{
				sum += hmm->trans_prob[i][j] * hmm_b(&hmm->output_prob[j], sample[t + 1]) * beta[t + 1][j];
			}
			beta[t][i] = sum;
			assert(!isnan(beta[t][i]));
		}
		
		for (i = 0; i < hmm->state_len; i++)
		{
			beta[t][i] *= scale[t];
			assert(!isnan(beta[t][i]));
		}
	}
}

/*
 * 
 */
double hmm_viterbi(struct hmm_3d_t *hmm, struct sample_3d_t sample[], unsigned int sample_len, unsigned int decoded[])
{
	//int decoded[sample_len];
	double delta[sample_len][hmm->state_len];
	int psi[sample_len][hmm->state_len];
	int i, j, t;
	
	for (i = 0; i < hmm->state_len; i++)
	{
		delta[0][i] = log(hmm->initial_prob[i]) + log(hmm_b(&hmm->output_prob[i], sample[0]));
		psi[0][i] = 0;
	}
	
	for (t = 1; t < sample_len; t++)
	{
		for (j = 0; j < hmm->state_len; j++)
		{
			double max = delta[t - 1][0] + log(hmm->trans_prob[0][j]);
			int argmax = 0;
			
			for (i = 0; i < hmm->state_len; i++)
			{
				if (max < delta[t - 1][i] + log(hmm->trans_prob[i][j]))
				{
					max = delta[t - 1][i] + log(hmm->trans_prob[i][j]);
					argmax = i;
				}
			}
			
			delta[t][j] = max + log(hmm_b(&hmm->output_prob[j], sample[t]));
			psi[t][j] = argmax;
		}
	}
	
	double max = delta[sample_len - 1][0];
	int argmax = 0;
	for (i = 0; i < hmm->state_len; i++)
	{
		if (max < delta[sample_len - 1][i])
		{
			max = delta[sample_len - 1][i];
			argmax = i;
		}
	}
	
	decoded[sample_len - 1] = argmax;
	for (t = sample_len - 2; t > -1; t--)
	{
		decoded[t] = psi[t + 1][decoded[t + 1]];
	}
	
	//printf("Decoded sequence:\n");
	//for (t = 0; t < sample_len; t++)
	//{
	//	printf("%d ", decoded[t]);
	//}
	//printf("\n");
	
	/* used to discard models that don't reach their final state */
	/*if (decoded[sample_len - 1] == hmm->state_len - 1)
	{
		*reached_final_state = 1;
	}
	else
	{
		*reached_final_state = 0;
	}*/
	
	return max;
}

/*
 * 
 */
void hmm_baum_welch(struct hmm_3d_t *hmm, struct hmm_3d_t *hmm_est, struct sample_3d_t sample[], unsigned int sample_len)
{
	double xi[sample_len][hmm->state_len][hmm->state_len];
	double scale[sample_len];
	double prob;
	int i, j, k, t;
	
	double **alpha;
	alpha = (double **)malloc(sample_len * sizeof(double *));
	for (t = 0; t < sample_len; t++)
	{
		alpha[t] = (double *)malloc(hmm->state_len * sizeof(double));
	}
	
	double **beta;
	beta = (double **)malloc(sample_len * sizeof(double *));
	for (t = 0; t < sample_len; t++)
	{
		beta[t] = (double *)malloc(hmm->state_len * sizeof(double));
	}
	
	double ***gamma;
	gamma = (double ***)malloc(sample_len * sizeof(double **));
	for (t = 0; t < sample_len; t++)
	{
		gamma[t] = (double **)malloc(hmm->state_len * sizeof(double *));
		for (j = 0; j < hmm->state_len; j++)
		{
			gamma[t][j] = (double *)malloc(hmm->output_prob[j].mix_len * sizeof(double));
		}
	}
	
	prob = hmm_forward_scale_alpha(hmm, sample, sample_len, scale, alpha);
	hmm_backward_scale_beta(hmm, sample, sample_len, scale, beta);
	
	printf("ALPHA and BETA\n");	
	for (t = 0; t < sample_len; t++)
	{
		printf("%d:\t[", t);
		for (i = 0; i < hmm->state_len; i++)
		{
			printf("%e\t", alpha[t][i]);
		}
		printf("] [");
		for (i = 0; i < hmm->state_len; i++)
		{
			printf("%e\t", beta[t][i]);
		}
		printf("]\n");
	}
	
	/* probability of taking the transition from state i to state j at time t */
	for (t = 0; t < sample_len - 1; t++)
	{
		for (i = 0; i < hmm->state_len; i++)
		{
			for (j = 0; j < hmm->state_len; j++)
			{
				xi[t][i][j] = alpha[t][i] *
					hmm->trans_prob[i][j] * hmm_b(&hmm->output_prob[j], sample[t + 1]) *
					beta[t + 1][j];
			}
		}
		double sum = 0.0;
		for (i = 0; i < hmm->state_len; i++)
		{
			for (j = 0; j < hmm->state_len; j++)
			{
				sum += xi[t][i][j];
			}
		}
		
		for (i = 0; i < hmm->state_len; i++)
		{
			for (j = 0; j < hmm->state_len; j++)
			{
				xi[t][i][j] /= sum;
				assert(!isnan(xi[t][i][j]));
			}
		}
	}
	
	for (t = 0; t < sample_len; t++)
	{
		double sum = 0.0;
		for (j = 0; j < hmm->state_len; j++)
		{
			sum += alpha[t][j] * beta[t][j];
		}
		
		for (j = 0; j < hmm->state_len; j++)
		{	
			for (k = 0;  k < hmm->output_prob[j].mix_len; k++)
			{
				gamma[t][j][k] = alpha[t][j] * beta[t][j] *
					hmm->output_prob[j].weight[k] *
					gauss_prob_den_3d(&hmm->output_prob[j].each[k], sample[t]) /
					(sum * hmm_b(&hmm->output_prob[j], sample[t]));
				//assert(!isnan(gamma[t][j][k]));
				if (isnan(gamma[t][j][k]))
				{
					printf("ERROR: %e - %e\n", gauss_prob_den_3d(&hmm->output_prob[j].each[k], sample[t]), hmm_b(&hmm->output_prob[j], sample[t])); 
				}
			}
		}
	}
	
	/* estimation begins */
	/* transition probabilities */
	for (i = 0; i < hmm->state_len; i++)
	{
		for (j = 0; j < hmm->state_len; j++)
		{
			double sum = 0.0;
			for (t = 0; t < sample_len - 1; t++)
			{
				sum += xi[t][i][j];
			}
			double sum2 = 0.0;
			for (t = 0; t < sample_len - 1; t++)
			{
				for (k = 0; k < hmm->state_len; k++)
				{
					sum2 += xi[t][i][k];
				}
			}
			hmm_est->trans_prob[i][j] = sum / sum2;
			assert(!isnan(hmm_est->trans_prob[i][j]));
		}
	}
	
	/* initial state probabilities */
	for (i = 0; i < hmm->state_len; i++)
	{
		double sum = 0.0;
		for (j = 0; j < hmm->state_len; j++)
		{
			sum += xi[0][i][j];
		}
		hmm_est->initial_prob[i] = sum;
		assert(!isnan(hmm->initial_prob[i]));
	}
	
	/* weight coefficients */
	for (j = 0; j < hmm->state_len; j++)
	{
		double sum = 0.0;
		for (k = 0; k < hmm->output_prob[j].mix_len; k++)
		{
			for (t = 0; t < sample_len; t++)
			{
				sum += gamma[t][j][k];
			}
		}
		
		for (k = 0; k < hmm->output_prob[j].mix_len; k++)
		{
			double sum2 = 0.0;
			for (t = 0; t < sample_len; t++)
			{
				sum2 += gamma[t][j][k];
			}
			hmm_est->output_prob[j].weight[k] = sum2 / sum;
			assert(!isnan(hmm_est->output_prob[j].weight[k]));
		}
	}
	
	/* means */
	for (j = 0; j < hmm->state_len; j++)
	{
		for (k = 0; k < hmm->output_prob[j].mix_len; k++)
		{
			double sum2 = 0.0;
			for (t = 0; t < sample_len; t++)
			{
				sum2 += gamma[t][j][k];
			}
			int l;
			for (l = 0; l < 3; l++)
			{
				double sum = 0.0;
				for (t = 1; t < sample_len; t++)
				{
					sum += gamma[t][j][k] * sample[t].val[l]; 
				}
				hmm_est->output_prob[j].each[k].mean[l] = sum / sum2;
				assert(!isnan(hmm_est->output_prob[j].each[k].mean[l]));
			}
		}
	}
	
	for (j = 0; j < hmm->state_len; j++)
	{
		for (k = 0; k < hmm->output_prob[j].mix_len; k++)
		{
			double sum2 = 0.0;
			for (t = 1; t < sample_len; t++)
			{
				sum2 += gamma[t][j][k];
			}
			int l1;//, l2;
			for (l1 = 0; l1 < 3; l1++)
			{
				//for (l2 = 0; l2 < 3; l2++)
				//{
					double sum = 0.0;
					for (t = 1; t < sample_len; t++)
					{
						/* mean from estimation or original? */
						sum += gamma[t][j][k] *
							(sample[t].val[l1] - hmm->output_prob[j].each[k].mean[l1]) *
							(sample[t].val[l1] - hmm->output_prob[j].each[k].mean[l1]);
							//(sample[t].val[l1] - hmm_est->output_prob[j].each[k].mean[l1]) *
							//(sample[t].val[l2] - hmm_est->output_prob[j].each[k].mean[l2]);
					}
					hmm_est->output_prob[j].each[k].covar[l1][l1] = sum / sum2;
					/* correct covariance */
					//if (hmm_est->output_prob[j].each[k].covar[l1][l2] <= 0.0)
					//{
					//	hmm_est->output_prob[j].each[k].covar[l1][l2] = 0.03;
					//}
					assert(!isnan(hmm_est->output_prob[j].each[k].covar[l1][l1]));
				//}
			}
		}
	}
	
	/*
	double repair_sum = 0.0;
	for (i = 0; i < hmm_est->state_len; i++)
	{
		if (hmm_est->initial_prob[i] <  0.0001)
		{
			hmm_est->initial_prob[i] = 0.0001;
		}
		repair_sum += hmm_est->initial_prob[i];
	}
	// normalize initial probs
	for (i = 0; i < hmm_est->state_len; i++)
	{
		hmm_est->initial_prob[i] /= repair_sum;
	}
	
	for (i = 0; i < hmm_est->state_len; i++)
	{
		repair_sum = 0.0;
		for (j = 0; j < hmm_est->state_len; j++)
		{
			if (hmm_est->trans_prob[i][j] < 0.0001)
			{
				hmm_est->trans_prob[i][j] = 0.0001;
			}
			repair_sum += hmm_est->trans_prob[i][j];
		}
		for (j = 0; j < hmm_est->state_len; j++)
		{
			hmm_est->trans_prob[i][j] /= repair_sum;
		}
	}
	*/
	
	/* normalize covariance diagonal */
	for (i = 0; i < hmm_est->state_len; i++)
	{
		for (k = 0; k < hmm_est->output_prob[i].mix_len; k++)
		{
			int l;
			for (l = 0; l < 3; l++)
			{
				if (hmm_est->output_prob[i].each[k].covar[l][l] < 0.01)
				{
					hmm_est->output_prob[i].each[k].covar[l][l] = 0.01;
				}
			}
		}
	}
	
	/* start dispose */
	for (t = 0; t < sample_len; t++)
	{
		free(alpha[t]);
	}
	free(alpha);
	
	for (t = 0; t < sample_len; t++)
	{
		free(beta[t]);
	}
	free(beta);
	
	for (t = 0; t < sample_len; t++)
	{
		for (j = 0; j < hmm->state_len; j++)
		{
			free(gamma[t][j]);
		}
		free(gamma[t]);
	}
	free(gamma);
}

/*
 * 
 */
int hmm_write_3d(struct hmm_3d_t *hmm, char *file_name)
{
	FILE *file;
	file = fopen(file_name, "wb");
	if (file == 0)
	{
		perror("fopen");
		return -1;
	}
	
	fwrite(&hmm->state_len, sizeof(unsigned int), 1, file);
	fwrite(hmm->initial_prob, sizeof(double), hmm->state_len, file);
	
	int i;
	for (i = 0; i < hmm->state_len; i++)
	{
		fwrite(hmm->trans_prob[i], sizeof(double), hmm->state_len, file);
	}
	
	for (i = 0; i < hmm->state_len; i++)
	{
		unsigned int mix_len = hmm->output_prob[i].mix_len; 
		fwrite(&hmm->output_prob[i].mix_len, sizeof(unsigned int), 1, file);
		fwrite(hmm->output_prob[i].weight, sizeof(double), mix_len, file);
		fwrite(hmm->output_prob[i].each, sizeof(struct gauss_3d_t), mix_len, file);
	}
	
	fclose(file);
	
	return 0;
}

/*
 * 
 */
int hmm_read_3d(struct hmm_3d_t *hmm, char *file_name)
{
	FILE *file;
	file = fopen(file_name, "rb");
	if (file == 0)
	{
		perror("fopen");
		return -1;
	}
	
	fread(&hmm->state_len, sizeof(unsigned int), 1, file);
	hmm->initial_prob = (double *)malloc(hmm->state_len * sizeof(double));
	fread(hmm->initial_prob, sizeof(double), hmm->state_len, file);

	hmm->trans_prob = (double **)malloc(hmm->state_len * sizeof(double *));
	int i;
	for (i = 0; i < hmm->state_len; i++)
	{
		hmm->trans_prob[i] = (double *)malloc(hmm->state_len * sizeof(double));
		fread(hmm->trans_prob[i], sizeof(double), hmm->state_len, file);
	}
	
	hmm->output_prob = (gauss_mix_3d_t *)malloc(hmm->state_len * sizeof(gauss_mix_3d_t));
	
	for (i = 0; i < hmm->state_len; i++)
	{ 
		fread(&hmm->output_prob[i].mix_len, sizeof(unsigned int), 1, file);
		unsigned int mix_len = hmm->output_prob[i].mix_len;
		gauss_mix_create_3d(&hmm->output_prob[i], mix_len);
		fread(hmm->output_prob[i].weight, sizeof(double), mix_len, file);
		fread(hmm->output_prob[i].each, sizeof(struct gauss_3d_t), mix_len, file);
	}
		
	fclose(file);
	
	return 0;
}

/*
 * 
 */
void hmm_create_3d(struct hmm_3d_t *hmm, unsigned int state_len)
{
	hmm->state_len = state_len;
	hmm->initial_prob = (double *)malloc(hmm->state_len * sizeof(double));
	hmm->trans_prob = (double **)malloc(hmm->state_len * sizeof(double *));
	
	int i;
	for (i = 0; i < hmm->state_len; i++)
	{
		hmm->trans_prob[i] = (double *)malloc(hmm->state_len * sizeof(double));
	}
	
	hmm->output_prob = (gauss_mix_3d_t *)malloc(hmm->state_len * sizeof(gauss_mix_3d_t));

	int j;
	for (i = 0; i < state_len; i++)
	{
		hmm->initial_prob[i] = 0.0;
		for (j = 0; j < state_len; j++) {
			hmm->trans_prob[i][j] = 0.0;
		}
	}
}

/*
 * 
 */
void hmm_delete_3d(struct hmm_3d_t *hmm)
{
	free(hmm->initial_prob);
	int i;
	for (i = 0; i < hmm->state_len; i++)
	{
		free(hmm->trans_prob[i]);
	}
	free(hmm->trans_prob);
	for (i = 0; i < hmm->state_len; i++)
	{
		gauss_mix_delete_3d(&hmm->output_prob[i]);
	}
	free(hmm->output_prob);
}

/*
 * 
 */
void hmm_uniform_3d(struct hmm_3d_t *hmm)
{
	float d = 1.0 / hmm->state_len;
	int i, j;
	
	for (i = 0; i < hmm->state_len; i++)
	{
		hmm->initial_prob[i] = d;
	}
	for (i = 0; i < hmm->state_len; i++)
	{
		for (j = 0; j < hmm->state_len; j++)
		{
			hmm->trans_prob[i][j] = d;
		}
	}
}

/*
 * left-right model
 */
void hmm_left_right_3d(struct hmm_3d_t *hmm)
{
	int i, j;
	//float sum = 1.0 + 0.0001 * (hmm->state_len - 1);
	
	hmm->initial_prob[0] = 1.0;// / sum;
	for (i = 1; i < hmm->state_len; i++)
	{
		hmm->initial_prob[i] = 0.0;//001 / sum;
	}
	
	for (i = 0; i < hmm->state_len; i++)
	{
		for (j = 0; j < hmm->state_len; j++)
		{
			hmm->trans_prob[i][j] = 0.0;//001 / sum;
		}
		
		//float d = 1.0 / (hmm->state_len - i);
		float d = 1.0;
		if (i < hmm->state_len - 1)
		{
			d /= 2;
			hmm->trans_prob[i][i + 1] = d;
		}
		hmm->trans_prob[i][i] = d; 
		
		//float sum = 1.0 + i * 0.0001;
		//for (j = i; j < hmm->state_len; j++)
		//{
		//	hmm->trans_prob[i][j] = d;// / sum;
		//}
		
	}
}

void hmm_print_3d(struct hmm_3d_t *hmm)
{
	printf("States: %d\n", hmm->state_len);
	int i, j;
	printf("Initial state distribution:\n");
	for (i = 0; i < hmm->state_len; i++)
	{
		printf("%f\t", hmm->initial_prob[i]);
	}
	printf("\n");
	printf("State transition distribution:\n");
	for (i = 0; i < hmm->state_len; i++)
	{
		for (j = 0; j < hmm->state_len; j++)
		{
			printf("%f\t", hmm->trans_prob[i][j]);
		}
		printf("\n");
	}
	printf("States:\n");
	for (i = 0; i < hmm->state_len; i++)
	{
		printf("Gaussian mixture for state %d:\n", i);
		gauss_mix_print_3d(&hmm->output_prob[i]);
	}
}

void hmm_copy_3d(struct hmm_3d_t *hmm_dest, struct hmm_3d_t *hmm_src)
{
	int i, j;
	if (hmm_dest->state_len != hmm_src->state_len)
	{
		return;
	}
	
	for (i = 0; i < hmm_src->state_len; i++)
	{
		hmm_dest->initial_prob[i] = hmm_src->initial_prob[i];
	}
	
	for (i = 0; i < hmm_src->state_len; i++)
	{
		for (j = 0; j < hmm_src->state_len; j++)
		{
			hmm_dest->trans_prob[i][j] = hmm_src->trans_prob[i][j];
		}
	}
	
	for (i = 0; i < hmm_src->state_len; i++)
	{
		gauss_mix_copy_3d(&hmm_dest->output_prob[i], &hmm_src->output_prob[i]);
	}
}

int hmm_write_gnu_plot_3d(hmm_3d_t *hmm, char *file_name)
{
	FILE *file;
	file = fopen(file_name, "w");
	if (file == 0)
	{
		perror("fopen");
		return -1;
	}
	
	
	int i;
	for (i = 0; i < hmm->state_len; i++)
	{
		char gnu[1024] = "\0";
		strcpy(gnu, file_name);
		char state[100] = "\0";
		sprintf(state, "_state_%d", i);
		strcat(gnu, state);
		strcat(gnu, ".gnu");
		gauss_mix_write_gnuplot_3d(&hmm->output_prob[i], gnu);
		fprintf(file, "load \"%s\"\n", gnu);
	}
	
	fclose(file);

	return 0;
}

