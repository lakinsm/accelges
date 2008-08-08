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

//#include <float.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "class.h"
#include "gauss.h"
#include "ges.h"
#include "hmm.h"
#include "model.h"

/* parse one line in the configuration file */
static unsigned char parse_line(struct config_t *config, char *line);
/* recognize frames */
static void recognize(struct ges_3d_t *ges, struct accel_3d_t accel[], unsigned int accel_len);

/*
 * create feature vector
 */
unsigned char ges_fea_3d(struct seq_3d_t *seq, unsigned int seq_index, unsigned int seq_prev_index, struct sample_3d_t *sample)
{
	if ((!seq) || (!sample))
	{
		return 0;
	}
	
	if (seq_index > 0)
	{		
		/* magnitude at time indicated by index */
		sample->val[0] = sqrt(
			pow(seq->each[seq_index].val[0], 2) +
			pow(seq->each[seq_index].val[1], 2) +
			pow(seq->each[seq_index].val[2], 2));
		/* magnitude at time indicated by prev_index */
		sample->val[1] = sqrt(
			pow(seq->each[seq_prev_index].val[0], 2) +
			pow(seq->each[seq_prev_index].val[1], 2) +
			pow(seq->each[seq_prev_index].val[2], 2));
		/* delta magnitude between time t and t - 1 */
		sample->val[2] = sqrt(
			pow(seq->each[seq_index].val[0] - seq->each[seq_prev_index].val[0], 2) +
			pow(seq->each[seq_index].val[1] - seq->each[seq_prev_index].val[1], 2) +
			pow(seq->each[seq_index].val[2] - seq->each[seq_prev_index].val[2], 2));		
		
		return 1;
	}
	else
	{
		sample->val[0] = 0.0;
		sample->val[1] = 0.0;
		sample->val[2] = 0.0;
		
		return 0;
	}
}

/*
 * process accelerometer values
 */
void ges_process_3d(struct ges_3d_t *ges, struct accel_3d_t accel)
{
	//printf("just a test\n");
	/* increment and save current frame (uses a circular list) */
	unsigned int prev_index = ges->seq.index;
	ges->seq.index = (ges->seq.index + 1) % FRAME_LEN;
	ges->seq.each[ges->seq.index] = accel;
	
	/* extract feature vector */
	sample_3d_t feature;
	ges_fea_3d(&ges->seq, ges->seq.index, prev_index, &feature);
	
	//printf("%f\t%f\t%f\t%s\n", feature.val[0], feature.val[1], feature.val[2],
	//	(class_max_2c(&ges->endpoint, feature) == 1) ? "DYNAMIC" : "STATIC");
	
	/* motion has index 1 and noise has index 0 */
	if (class_max_2c(&ges->endpoint, feature) == 1)
	{
		/* we want motion */
		if (!ges->detected)
		{
			ges->seq.begin = ges->seq.index;
			ges->detected = 1;
			ges->seq.till_end = FRAME_AFTER;

			//ges->prev_class_time = 0;
			//ges->prev_class_change = 0;
		}
		if (ges->seq.till_end < FRAME_AFTER)
		{
			ges->seq.till_end = FRAME_AFTER;
		}
	}
	else
	{
		/* classified as noise and it doesn't follow after a gesture */
		if (!ges->detected)
		{
			struct sample_3d_t sample;
			sample.val[0] = accel.val[0];
			sample.val[1] = accel.val[1];
			sample.val[2] = accel.val[2];
			int i = class_max_uc(ges->class, ges->class_len, sample);
			if (ges->prev_class_ind != i)
			{
				ges->prev_class_ind = i;
				ges->prev_class_time = 0;
				ges->prev_class_change = 1;
			} else {
				if (ges->prev_class_change) {
					ges->prev_class_time++;
				}
				if (ges->prev_class_time > CLASS_TIME) {
					/* screen orientation changed */
					//printf("%s\n", ges->class_cmd[i]);
					//fflush(stdout);
					ges->prev_class_time = 0;
					ges->prev_class_change = 0;
					ges->handle_reco(ges->class_cmd[i]);
				}
			}
		}
		
		/* classified as noise, and the gesture just finished */
		if (ges->detected)
		{
			//printf("detected with size: %d\n", ges->seq.index - ges->seq.begin + 1);
			if (ges->seq.till_end > 0)
				ges->seq.till_end--;
		}
		if (ges->seq.till_end == 0)
		{
			ges->seq.end = ges->seq.index;
			ges->detected = 0;
			ges->seq.till_end = FRAME_AFTER;
		
			printf("detected with size: %d\n", ges->seq.end - ges->seq.begin + 1 - FRAME_AFTER);
				 
			/* case when begin < end */
			if (ges->seq.begin < ges->seq.end)
			{	
				if (ges->seq.end - ges->seq.begin > FRAME_DIF + FRAME_AFTER)
				{
					int before = 0;
					if (ges->seq.begin > FRAME_BEFORE)
					{
						before = FRAME_BEFORE;
					}
					
					unsigned int frame_len = ges->seq.end - ges->seq.begin + 1;
					struct accel_3d_t accels[before + frame_len];
					memcpy(&accels[before], &ges->seq.each[ges->seq.begin], frame_len * sizeof(sample_3d_t));
					
					if (before > 0)
					{
						memcpy(&accels[0], &ges->seq.each[ges->seq.begin - before], before * sizeof(sample_3d_t));
					}
					recognize(ges, accels, before + frame_len);
				}
			}
			else /* case when begin > end */
			{
				if (FRAME_LEN - ges->seq.begin + ges->seq.end > FRAME_DIF + FRAME_AFTER)
				{
					int before = 0;
					if (ges->seq.begin - ges->seq.end > FRAME_BEFORE)
					{
						before = FRAME_BEFORE;
					}
					unsigned int frame_len_end = FRAME_LEN - ges->seq.begin;
					unsigned int frame_len_begin = ges->seq.end + 1;
					unsigned int frame_len = frame_len_end + frame_len_begin;
					struct accel_3d_t accels[before + frame_len];
					memcpy(&accels[before], &ges->seq.each[ges->seq.begin], frame_len_end * sizeof(sample_3d_t));
					memcpy(&accels[before + frame_len_end], &ges->seq.each[0], frame_len_begin * sizeof(accel_3d_t));
					
					if (before > 0)
					{
						memcpy(&accels[0], &ges->seq.each[ges->seq.begin - before], before * sizeof(sample_3d_t));
					}
					
					recognize(ges, accels, before + frame_len); 
				}
			}
		}
	}
}

/* 
 * allocate memory for class mixtures 
 */
void ges_create_3d(struct ges_3d_t *ges)
{
	ges->seq.index = 0;
	ges->detected = 0;
	ges->seq.till_end = FRAME_AFTER;
	ges->prev_class_ind = -1;
	ges->prev_class_change = 1;
	ges->prev_class_time = 0;
	/* do not create here, will be created during reading from files */
	//gauss_mix_create_3d(&ges->endpoint.each[0], 1);
	//gauss_mix_create_3d(&ges->endpoint.each[1], 1);
}

/* 
 * de-allocate memory for class mixtures 
 */
void ges_delete_3d(struct ges_3d_t *ges)
{
	gauss_mix_delete_3d(&ges->endpoint.each[0]);
	gauss_mix_delete_3d(&ges->endpoint.each[1]);
}

/* 
 * read ges structure from files
 */
void ges_read_3d(struct ges_3d_t *ges, struct config_t *config)
{
	int i;
	ges->endpoint.prior_prob[0] = config->sclassp;
	gauss_mix_read_3d(&ges->endpoint.each[0], config->sclass_file);
	//printf("%s\n", config->sclass_file);
	//gauss_mix_print_3d(&ges->endpoint.each[0]);
	ges->endpoint.prior_prob[1] = config->dclassp;
	gauss_mix_read_3d(&ges->endpoint.each[1], config->dclass_file);
	//gauss_mix_print_3d(&ges->endpoint.each[1]);
	
	ges->class_len = config->class_len;
	for (i = 0; i < ges->class_len; i++)
	{
		gauss_mix_read_3d(&ges->class[i], config->class_file[i]);
		strcpy(ges->class_cmd[i], config->class_id[i]);
		printf("%s: %s\n", config->class_file[i], ges->class_cmd[i]);
	}
	
	
	ges->model_len = config->model_len;	
	for (i = 0; i < ges->model_len; i++)
	{
		hmm_read_3d(&ges->model[i], config->model_file[i]);
		strcpy(ges->model_cmd[i], config->model_id[i]);
		printf("%s: %s\n", config->model_file[i], ges->model_cmd[i]); 
	}
}

/* 
 * write ges structure to file
 */
void ges_write_3d(struct ges_3d_t *ges, struct config_t *config)
{
	gauss_mix_write_3d(&ges->endpoint.each[0], config->sclass_file);
	gauss_mix_write_3d(&ges->endpoint.each[1], config->dclass_file);
}

/*
 * load configuration file
 */
unsigned char ges_load_config(struct config_t *config, char *file_name)
{
	if (!file_name)
	{
		return 0;
	}
	/* change working directory to be able to read files from same dir as config file */
	char path_copy[1024];
	strncpy(path_copy, file_name, sizeof(path_copy));
	path_copy[1023] = '\0';
	/* pass a copy because dirname will modify it */
	chdir(dirname(path_copy));
	system("echo `pwd`");
	
	FILE *file;
	char line[1024];
	unsigned int line_index;
	
	file = fopen(basename(file_name), "r");
	if (!file)
	{
		perror("fopen");
		return 0;
	}
	
	config->class_len = 0;
	config->model_len = 0;
	
	line_index = 0;
	while (fgets(line, sizeof(line), file))
	{
		line_index++;
		if (!parse_line(config, line))
		{
			fprintf(stderr, "Unknown syntax in configuration file at line %d\n", line_index);
			return 0;
		}
	}
	
	fclose(file);
	
	return 1;
}

/* 
 * isolated recognition in continuous mode
 * only one gesture is detected in the classified frames
 */
static void recognize(struct ges_3d_t *ges, struct accel_3d_t accel[], unsigned int accel_len)
{
	if (ges->model_len < 2)
		return;
		
	//printf("Received %d frames.\n", accel_len);
	//printf("Processing gesture... (TO-DO: gesture recognition)\n");
	int i;
	/*
	for (i = 0; i < accel_len; i++)
	{
		printf("%d:\t%+f\t%+f\t%+f\n", i, accel[i].val[0], accel[i].val[1], accel[i].val[2]);
	}
	// */
	unsigned int decoded_states[accel_len];	
	double vals[ges->model_len];
	unsigned char pruned[ges->model_len];
	memset(pruned, 0, sizeof(pruned));
	
	//unsigned char reached[ges->model_len];
	//unsigned char max_reached_final_state = 0;
	//double max = hmm_viterbi(&ges->model[0], accel, accel_len, decoded_states);
	
	//if (decoded_states[accel_len - 1] == ges->model[0].state_len - 1)
	//{
	//	max_reached_final_state = 1;
	//}
	//else
	//{
	//	max_reached_final_state = 0;
	//}
	
	int argmax = 0;
	//printf("Recognition results:\n");
	for (i = 0; i < ges->model_len; i++)
	{
		//float possib_max = hmm_forward(&ges->model[i], accel, accel_len);
		//unsigned char possib_max_reached_final_state = 0; 
		//double possib_max = hmm_viterbi(&ges->model[i], accel, accel_len, decoded_states);
		vals[i] = hmm_viterbi(&ges->model[i], accel, accel_len, decoded_states);
		printf("%s (has 0..%d states):\n", ges->model_cmd[i], ges->model[i].state_len - 1);
		int t;
		for (t = 0; t < accel_len; t++)
		{
			printf("%d", decoded_states[t]);
		}
		printf("\n");
	
		/* prune if the last state is not the final state */
		//if (0) {
			if (decoded_states[accel_len - 1] != ges->model[i].state_len - 1)
			{
				pruned[i] = 1;
			}
		//}
		/* prunning by state duration! */
		//if (!pruned[i])
		if (0)
		{
			/* prune if state duration contraint not met */
			double state_prob[ges->model[i].state_len];
			unsigned int state_duration[ges->model[i].state_len];
			memset(state_duration, 0, sizeof(state_duration));
			int j;
			for (j = 0; j < accel_len; j++)
			{
				state_duration[decoded_states[j]]++;
			}
			for (j = 0; j < ges->model[i].state_len; j++)
			{
				state_prob[j] = (double)state_duration[j] / accel_len;
				//printf("state prob %d: %f\n", j, state_prob[j]);
			}
			double mean = 1.0 / ges->model[i].state_len;
			for (j = 0; j < ges->model[i].state_len; j++)
			{
				/* don't use explicit values for the state duration */
				//if (fabs(state_prob[j] - mean) > 0.4)
				if (state_duration[j] < 2)
				{
					pruned[i] = 1;
					break;
				}
			}
		}
				
	//if (decoded_states[accel_len - 1] == ges->model[i].state_len - 1)
	//{
	//	possib_max_reached_final_state = 1;
	//}
	//else
	//{
	//	possib_max_reached_final_state = 0;
	//}


		//vals[i] = possib_max;
		//reached[i] = possib_max_reached_final_state;
		/*
		if (!max_reached_final_state)
		{
			max = possib_max;
			max_reached_final_state = possib_max_reached_final_state;
			argmax = i;
		}
		printf("%s: %e\n", ges->model_cmd[i], possib_max);
		if (possib_max_reached_final_state)
		{ 
			if (max < possib_max)
			{
				max = possib_max;
				max_reached_final_state = possib_max_reached_final_state;
				argmax = i;
			}
		}
		*/
	}
	
	for (i = 0; i < ges->model_len; i++)
	{
		printf("%s (%s):\t%f\n", ges->model_cmd[i], pruned[i] ? "pruned" : "unpruned", vals[i]);
	}
	fflush(stdout);

	double max = 0.0;
	i = 0;
	while ((i < ges->model_len) && pruned[i])
	{
		i++;
	}
	if (i < ges->model_len)
	{
		argmax = i;
		max = vals[i];
		int j;
		for (j = i + 1; j < ges->model_len; j++)
		{
			if ((!pruned[j]) && (max < vals[j]))
			{
				max = vals[j];
				argmax = j;	
			}
		}
		/* now that we definitely have a max, call the action */
		ges->handle_reco(ges->model_cmd[argmax]);
	}
	else
	{
		//printf("No match.\n");
	}
	
	//double prev_max = -1.0e+07;
	//for (i = 0; i < ges->model_len; i++)
	//{
	//	if (i != argmax)
	//	{
	//		if (reached[i])
	//		{
	//			if (prev_max < vals[i])
	//			{
	//				prev_max = vals[i];
	//			}
	//		}
	//	}
	//}
	
	//printf("prev_max: %e\n", prev_max);
	
	//if (max_reached_final_state)
	//{
	//	printf("difference: %f\n", fabs(max - prev_max)); 
	//	if (fabs(max - prev_max) > 20)
	//	{
	//		/* call handler once the recognition is done */
	//		ges->handle_reco(ges->model_cmd[argmax]);
	//	}
	//	else
	//	{
	//		printf("Sorry, didn't get that...\n");
	//	}
	//}
	//else
	//{
	//	printf("Sorry, didn't get that...\n");
	//}
}

/* 
 * parse one line in the configuration file
 */
static unsigned char parse_line(struct config_t *config, char *line)
{
	if ((!config) || (!line))
	{
		return 0;
	}
		
	/* comment */
	if ((line[0] == '#') || (line[0] == ';'))
	{
		return 1;
	}
	else
	{
		char cmd_name[1024];
		unsigned int cmd_len = strcspn(line, "\t\n") / sizeof(char);
		strncpy(cmd_name, line, cmd_len * sizeof(char));
		cmd_name[cmd_len] = '\0';
		/* no command on this line */
		if (cmd_name[0] == '\0')
		{
			return 1; /* return success */
		}

		if (strcmp(cmd_name, "sclassp") == 0) {
			unsigned int no_param_len = strspn(&line[cmd_len], "\t\n") / sizeof(char);
			char *params = &line[cmd_len + no_param_len];
			char param_name[1024];
			unsigned int param_len = strcspn(params, "\t\n") / sizeof(char);
			strncpy(param_name, params, param_len);
			param_name[param_len] = '\0';
			
			if (param_name[0] == '\0')
			{
				return 0; /* same as above */
			}
			config->sclassp = atof(param_name);		
		} else if (strcmp(cmd_name, "dclassp") == 0) {
			unsigned int no_param_len = strspn(&line[cmd_len], "\t\n") / sizeof(char);
			char *params = &line[cmd_len + no_param_len];
			char param_name[1024];
			unsigned int param_len = strcspn(params, "\t\n") / sizeof(char);
			strncpy(param_name, params, param_len);
			param_name[param_len] = '\0';
			
			if (param_name[0] == '\0')
			{
				return 0; /* same as above */
			}
			config->dclassp = atof(param_name);		
		} else if (strcmp(cmd_name, "sclass") == 0) {
			unsigned int no_param_len = strspn(&line[cmd_len], "\t\n") / sizeof(char);
			char *params = &line[cmd_len + no_param_len];
			char param_name[1024];
			unsigned int param_len = strcspn(params, "\t\n") / sizeof(char);
			strncpy(param_name, params, param_len);
			param_name[param_len] = '\0';
			
			if (param_name[0] == '\0')
			{
				return 0; /* must have param, so return failure */
			}
			
			strcpy(config->sclass_file, param_name);
		}
		else if (strcmp(cmd_name, "dclass") == 0) {
			unsigned int no_param_len = strspn(&line[cmd_len], "\t\n") / sizeof(char);
			char *params = &line[cmd_len + no_param_len];
			char param_name[1024];
			unsigned int param_len = strcspn(params, "\t\n") / sizeof(char);
			strncpy(param_name, params, param_len);
			param_name[param_len] = '\0';
			
			if (param_name[0] == '\0')
			{
				return 0; /* same as above */
			}
			
			strcpy(config->dclass_file, param_name);	
		}
		else if (strcmp(cmd_name, "model") == 0)
		{
			// TODO: parse hmm line
			unsigned int no_param_len = strspn(&line[cmd_len], "\t\n") / sizeof(char);
			char *params = &line[cmd_len + no_param_len];
			char param_name_1[1024];
			unsigned int param_len_1 = strcspn(params, "\t\n") / sizeof(char);
			strncpy(param_name_1, params, param_len_1);
			// BUG HERE and also below
			param_name_1[param_len_1] = '\0';
			
			if (param_name_1[0] == '\0')
			{
				return 0; /* must have param, so return failure */
			}
			
			unsigned int no_param_len_2 = strspn(&line[cmd_len + no_param_len + param_len_1], " \t\n") / sizeof(char);
			char *params_2 = &line[cmd_len + no_param_len + param_len_1 + no_param_len_2];
			char param_name_2[1024];
			unsigned int param_len_2 = strcspn(params_2, "\t\n") / sizeof(char);
			// check length of param_len_2 with size of param_name_2!!!
			strncpy(param_name_2, params_2, param_len_2);
			param_name_2[param_len_2] = '\0';
			
			
			/* model command that is returned when detected */
			strcpy(config->model_id[config->model_len], param_name_1);
			/* model file name to read data from */
			strcpy(config->model_file[config->model_len], param_name_2);

			//printf("arg %s and %s.\n", param_name_1, param_name_2);
			config->model_len++;
		}
		else if (strcmp(cmd_name, "class") == 0)
		{
			// TODO: parse hmm line
			unsigned int no_param_len = strspn(&line[cmd_len], "\t\n") / sizeof(char);
			char *params = &line[cmd_len + no_param_len];
			char param_name_1[1024];
			unsigned int param_len_1 = strcspn(params, "\t\n") / sizeof(char);
			strncpy(param_name_1, params, param_len_1);
			// BUG HERE and also below
			param_name_1[param_len_1] = '\0';
			
			if (param_name_1[0] == '\0')
			{
				return 0; /* must have param, so return failure */
			}
			
			unsigned int no_param_len_2 = strspn(&line[cmd_len + no_param_len + param_len_1], " \t\n") / sizeof(char);
			char *params_2 = &line[cmd_len + no_param_len + param_len_1 + no_param_len_2];
			char param_name_2[1024];
			unsigned int param_len_2 = strcspn(params_2, "\t\n") / sizeof(char);
			// check length of param_len_2 with size of param_name_2!!!
			strncpy(param_name_2, params_2, param_len_2);
			param_name_2[param_len_2] = '\0';
			
			
			/* model command that is returned when detected */
			strcpy(config->class_id[config->class_len], param_name_1);
			/* model file name to read data from */
			strcpy(config->class_file[config->class_len], param_name_2);

			//printf("arg %s and %s.\n", param_name_1, param_name_2);
			config->class_len++;
		}
		else
		{
			return 0;
		}
	}
	// printf("%s", line);
	
	return 1;
}

/* 
 * populate ges structure with manual values
 */
void ges_populate_3d(struct ges_3d_t *ges)
{	
	/* noise */
	gauss_mix_create_3d(&ges->endpoint.each[0], 1);
	ges->endpoint.prior_prob[0] = 0.4;
	ges->endpoint.each[0].weight[0] = 1;
	ges->endpoint.each[0].each[0].mean[0] = 1.0;
	ges->endpoint.each[0].each[0].mean[1] = 1.0;
	ges->endpoint.each[0].each[0].mean[2] = 0.0;
	ges->endpoint.each[0].each[0].covar[0][0] = 0.07;
	ges->endpoint.each[0].each[0].covar[0][1] = 0.0;
	ges->endpoint.each[0].each[0].covar[0][2] = 0.0;
	ges->endpoint.each[0].each[0].covar[1][0] = 0.0;
	ges->endpoint.each[0].each[0].covar[1][1] = 0.07;
	ges->endpoint.each[0].each[0].covar[1][2] = 0.0;
	ges->endpoint.each[0].each[0].covar[2][0] = 0.0;
	ges->endpoint.each[0].each[0].covar[2][1] = 0.0;
	ges->endpoint.each[0].each[0].covar[2][2] = 0.07;
	
	/* motion */
	gauss_mix_create_3d(&ges->endpoint.each[1], 1);
	ges->endpoint.prior_prob[1] = 0.6;
	ges->endpoint.each[1].weight[0] = 1;
	ges->endpoint.each[1].each[0].mean[0] = 2.4;
	ges->endpoint.each[1].each[0].mean[1] = 2.0;
	ges->endpoint.each[1].each[0].mean[2] = 0.7;
	ges->endpoint.each[1].each[0].covar[0][0] = 0.4;
	ges->endpoint.each[1].each[0].covar[0][1] = 0.0;
	ges->endpoint.each[1].each[0].covar[0][2] = 0.0;
	ges->endpoint.each[1].each[0].covar[1][0] = 0.0;
	ges->endpoint.each[1].each[0].covar[1][1] = 0.4;
	ges->endpoint.each[1].each[0].covar[1][2] = 0.0;
	ges->endpoint.each[1].each[0].covar[2][0] = 0.0;
	ges->endpoint.each[1].each[0].covar[2][1] = 0.0;
	ges->endpoint.each[1].each[0].covar[2][2] = 0.4;
}
