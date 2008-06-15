/*
 * Copyright (C) 2008 by OpenMoko, Inc.
 * Written by Paul-Valentin Borza <gestures@borza.ro>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gauss.h"
#include "ges.h"
#include "gesapp.h"
#include "hmm.h"
#include "model.h"
#include "wii.h"

#define VERSION "0.1"

/* wii */
static struct wii_t wii;
/* sequence */
static struct seq_3d_t seq;
static char file_name[1024];
static unsigned char is_pressed;

struct gauss_mix_3d_t gauss_mix;
struct gauss_mix_3d_t gauss_mix_est;

struct hmm_3d_t hmm;
struct hmm_3d_t hmm_est;

/* print welcome message */
static void print_header(void);
/* print all command line options */
static void print_usage(char *file_name);
/* print current version */
static void print_version(void);
/* parse options given at command line */
static char parse_options(int argc, char **argv);
/* catches the termination signal and closes the Wii */
static void class_handle_signal(int signal);
/* catches the termination signal and closes the Wii */
static void model_handle_signal(int signal);

static void show_handle_signal(int signal);

char prev_state[4] = "000";
float prev_val[3] = { 0.0, 0.0, 0.0};
unsigned int count = 1;

void show_handle_accel(unsigned char pressed, struct accel_3d_t accel)
{
	if (pressed)
	{
		printf("%+f\t%+f\t%+f\t", accel.val[0], accel.val[1], accel.val[2]);
		char state[4] = "\0";
		
		if (accel.val[0] > 0.4)
			strcat(state, "+");
		else if (accel.val[0] < -0.4)
			strcat(state, "-");
		else
			strcat(state, "0");
		
		if (accel.val[1] > 0.4)
			strcat(state, "+");
		else if (accel.val[1] < -0.4)
			strcat(state, "-");
		else
			strcat(state, "0");
		
		if (accel.val[2] > 0.4)
			strcat(state, "+");
		else if (accel.val[2] < -0.4)
			strcat(state, "-");
		else
			strcat(state, "0");
		
		printf("%s\t", state);
		
		if (strcmp(state, prev_state) != 0)
		{
			printf("%+f\t%+f\t%+f\n", 
				prev_val[0] / count, 
				prev_val[1] / count, 
				prev_val[2] / count);
			prev_val[0] = accel.val[0];
			prev_val[1] = accel.val[1];
			prev_val[2] = accel.val[2];
			count = 1;
		} else {
			prev_val[0] += accel.val[0];
			prev_val[1] += accel.val[1];
			prev_val[2] += accel.val[2];
			count++;
			printf("\n");
		}
			
		strcpy(prev_state, state);
		is_pressed = 1;
	}
	
	if ((pressed) && (is_pressed))
	{
		seq.each[seq.index] = accel;
		seq.index = seq.index + 1;
	}
	else if ((!pressed) && (is_pressed)) /* released */
	{
		printf("%+f\t%+f\t%+f\t", accel.val[0], accel.val[1], accel.val[2]);
		char state[4] = "\0";
		
		if (accel.val[0] > 0.4)
			strcat(state, "+");
		else if (accel.val[0] < -0.4)
			strcat(state, "-");
		else
			strcat(state, "0");
		
		if (accel.val[1] > 0.4)
			strcat(state, "+");
		else if (accel.val[1] < -0.4)
			strcat(state, "-");
		else
			strcat(state, "0");
		
		if (accel.val[2] > 0.4)
			strcat(state, "+");
		else if (accel.val[2] < -0.4)
			strcat(state, "-");
		else
			strcat(state, "0");
		
		printf("%s\t", state);
		
		printf("%+f\t%+f\t%+f\n", 
				prev_val[0] / count, 
				prev_val[1] / count, 
				prev_val[2] / count);
		strcpy(prev_state, "000");
		prev_val[0] = 0.0;
		prev_val[1] = 0.0;
		prev_val[2] = 0.0;
		count = 1;
		
		//unsigned int seq_len = seq.index - 1;
		printf("End of push/release.\n");
		
		seq.index = 0;
		is_pressed = 0;
	}
}

/* this handler is called whenever the Wii sends acceleration reports */
void class_handle_accel(unsigned char pressed, struct accel_3d_t accel)
{
	if (pressed)
	{
		is_pressed = 1;
	}
	
	if ((pressed) && (is_pressed))
	{
		seq.each[seq.index] = accel;
		seq.index = seq.index + 1;
	}
	else if ((!pressed) && (is_pressed))
	{
		unsigned int feature_len = seq.index - 2;
		struct sample_3d_t feature[feature_len];
		int i;
		
		for (i = 1; i < seq.index - 1; i++)
		{
			ges_fea_3d(&seq, i, i-1, &feature[i - 1]);
			//printf("%f\t%f\t%f\n", feature[i-1].val[0], feature[i-1].val[1], feature[i-1].val[2]);
		}
		
		gauss_mix_den_est_3d(&gauss_mix, &gauss_mix_est, feature, feature_len);
		gauss_mix_print_3d(&gauss_mix_est);
		//gauss_mix_print_3d(&gauss_mix);
		/* copies gauss_mix_est to gauss_mix (need to re-arrange the params to be consistent */
		gauss_mix_copy_3d(&gauss_mix, &gauss_mix_est);
		
		seq.index = 0;
		is_pressed = 0;
	}
}

void model_handle_accel(unsigned char pressed, struct accel_3d_t accel)
{
	if (pressed)
	{
		is_pressed = 1;
	}
	
	if ((pressed) && (is_pressed))
	{
		seq.each[seq.index] = accel;
		seq.index = seq.index + 1;
	}
	else if ((!pressed) && (is_pressed))
	{	
		hmm_baum_welch(&hmm, &hmm_est, seq.each, seq.index - 1);
		hmm_copy_3d(&hmm, &hmm_est);
		hmm_print_3d(&hmm);
		//gauss_mix_print_3d(&gauss_mix);
		/* copies gauss_mix_est to gauss_mix (need to re-arrange the params to be consistent */
		//gauss_mix_copy_3d(&gauss_mix, &gauss_mix_est);
		
		seq.index = 0;
		is_pressed = 0;
	}
}

 /*
 * do not run in terminal: sudo hidd --search
 */
int main(int argc, char **argv)
{
	unsigned int mix_len;
	unsigned int state_len;
	
	print_header();
	
	/* get the configuration file name */
	switch (parse_options(argc, argv))
	{
		case 'b': /* new-class */
			printf("Class %s\n", file_name);
			printf("Number of mixtures: ");
			scanf("%d", &mix_len);
			gauss_mix_create_3d(&gauss_mix, mix_len);
			gauss_mix_rand_3d(&gauss_mix);
			gauss_mix_print_3d(&gauss_mix); 
			gauss_mix_write_3d(&gauss_mix, file_name);
			gauss_mix_delete_3d(&gauss_mix);
			
			exit(0);
			break;
		case 'c': /* view-class */
			printf("Class: %s\n", file_name);
			if (gauss_mix_read_3d(&gauss_mix, file_name) != 0)
			{
				exit(1);
			}
			gauss_mix_print_3d(&gauss_mix);
			gauss_mix_delete_3d(&gauss_mix);
			
			exit(0);
			break;
		case 'd': /* train-class */
			printf("Class: %s\n", file_name);
			if (gauss_mix_read_3d(&gauss_mix, file_name) != 0)
			{
				exit(1);
			}
			gauss_mix_print_3d(&gauss_mix);
			gauss_mix_create_3d(&gauss_mix_est, gauss_mix.mix_len);
			
			seq.index = 0;
			is_pressed = 0;
			wii.handle_accel = class_handle_accel;
	
			printf("Searching... (Press 1 and 2 on the Wii)\n");
			if (wii_search(&wii, 5) < 0)
			{
				fprintf(stderr, "Could not find the Wii.\n");
				exit(1);
			}
			printf("Found.\n");
	
			if (wii_connect(&wii) < 0)
			{
				fprintf(stderr, "Could not connect to the Wii.\n");
				exit(1);
			}
			printf("Connected. (Press and hold A then make gesture. Release when done.)\n");
	
			/* catch terminate signal to stop the read thread and close sockets */
			signal(SIGINT, class_handle_signal);
			signal(SIGTERM, class_handle_signal);
	
			wii_set_leds(&wii, 0, 0, 0, 1);
			wii_talk(&wii); /* will enter loop and never get out */
			
			break;
		case 'l': /* new-model */
			printf("Model %s\n", file_name);
			printf("Number of states: ");
			scanf("%d", &state_len);
			hmm_create_3d(&hmm, state_len);
			hmm_left_right_3d(&hmm);
			/* don't use uniform initial estimates, use a left-right Bakis model */
			//hmm_uniform_3d(&hmm);
			int i;
			for (i = 0; i < state_len; i++)
			{
				int mix_len = 1;
				int k;
				printf("State: %d\n", i);
				printf("Hard-coded with 1 mixture\n");
				gauss_mix_create_3d(&hmm.output_prob[i], mix_len);
				gauss_mix_rand_3d(&hmm.output_prob[i]);
				
				/* override random values */
				for (k = 0; k < mix_len; k++)
				{
					float val;
					printf("Mean for x: ");
					scanf("%f", &val);
					hmm.output_prob[i].each[k].mean[0] = val;
					printf("Mean for y: ");
					scanf("%f", &val);
					hmm.output_prob[i].each[k].mean[1] = val;
					printf("Mean for z: ");
					scanf("%f", &val);
					hmm.output_prob[i].each[k].mean[2] = val;
					
					printf("Covariance for x: ");
					scanf("%f", &val);
					hmm.output_prob[i].each[k].covar[0][0] = val;
					hmm.output_prob[i].each[k].covar[0][1] = 0.0;
					hmm.output_prob[i].each[k].covar[0][2] = 0.0;
					
					printf("Covariance for y: ");
					scanf("%f", &val);
					hmm.output_prob[i].each[k].covar[1][0] = 0.0;
					hmm.output_prob[i].each[k].covar[1][1] = val;
					hmm.output_prob[i].each[k].covar[1][2] = 0.0;
					
					printf("Covariance for z: ");
					scanf("%f", &val);
					hmm.output_prob[i].each[k].covar[2][0] = 0.0;
					hmm.output_prob[i].each[k].covar[2][1] = 0.0;
					hmm.output_prob[i].each[k].covar[2][2] = val;
				}
			}
			hmm_print_3d(&hmm);
			hmm_write_3d(&hmm, file_name);
			
			exit(0);
			break;
		case 'm': /* view-model */
			printf("Model %s\n", file_name);
			if (hmm_read_3d(&hmm, file_name) != 0)
			{
				exit(1);
			}
			hmm_print_3d(&hmm);
			hmm_delete_3d(&hmm);
			
			break;
		case 'n': /* train-model */
			printf("Model: %s\n", file_name);
			if (hmm_read_3d(&hmm, file_name) != 0)
			{
				exit(1);
			}
			hmm_print_3d(&hmm);
			hmm_create_3d(&hmm_est, hmm.state_len);
			for (i = 0; i < hmm_est.state_len; i++)
			{
				gauss_mix_create_3d(&hmm_est.output_prob[i], hmm.output_prob[i].mix_len);
			}
			//hmm_print_3d(&hmm_est);
			
			seq.index = 0;
			is_pressed = 0;
			wii.handle_accel = model_handle_accel;
	
			printf("Searching... (Press 1 and 2 on the Wii)\n");
			if (wii_search(&wii, 5) < 0)
			{
				fprintf(stderr, "Could not find the Wii.\n");
				exit(1);
			}
			printf("Found.\n");
	
			if (wii_connect(&wii) < 0)
			{
				fprintf(stderr, "Could not connect to the Wii.\n");
				exit(1);
			}
			printf("Connected. (Press and hold A then make gesture. Release when done.)\n");
	
			/* catch terminate signal to stop the read thread and close sockets */
			signal(SIGINT, model_handle_signal);
			signal(SIGTERM, model_handle_signal);
	
			wii_set_leds(&wii, 0, 0, 0, 1);
			wii_talk(&wii); /* will enter loop and never get out */
			
			break;
		case 's': /* show-accel */
	
			wii.handle_accel = show_handle_accel;
	
			printf("Searching... (Press 1 and 2 on the Wii)\n");
			if (wii_search(&wii, 5) < 0)
			{
				fprintf(stderr, "Could not find the Wii.\n");
				exit(1);
			}
			printf("Found.\n");
	
			if (wii_connect(&wii) < 0)
			{
				fprintf(stderr, "Could not connect to the Wii.\n");
				exit(1);
			}
			printf("Connected. (Press and hold A to see acceleration)\n");
	
			/* catch terminate signal to stop the read thread and close sockets */
			signal(SIGINT, show_handle_signal);
			signal(SIGTERM, show_handle_signal);
	
			wii_set_leds(&wii, 0, 0, 0, 1);
			wii_talk(&wii); /* will enter loop and never get out */
			
			break;
		default:

			exit(1);
			break;
	}

	/* won't reach this point, will be terminated with signal */
	exit(0);
}

/* 
 * print welcome message
 */
static void print_header(void)
{
	printf("gesapp: (C) 2008 OpenMoko Inc.\n"
		"This program is free software under the terms of the GNU General Public License.\n\n");
}

/*
 * print all command line options 
 */
static void print_usage(char *file_name)
{
	if (!file_name)
	{
		return;
	}
	
	printf("Usage: %s [OPTIONS] | --version | --help \n", file_name);
	printf("Options:\n");
	printf("  Class: --new-class <class> | --view-class <class> | --train-class <class>\n");
	printf("  Model: --new-model <model> | --view-model <model> | --train-model <model>\n");
	printf("  Misc: --show-accel\n");
}

/*
 * print current version 
 */
static void print_version(void)
{
	printf("Version: %s\n", VERSION);
}

/*
 * parse options given at command line
 */
static char parse_options(int argc, char **argv)
{
	int long_opt_ind = 0;
	int long_opt_val = 0;
	
	static struct option long_opts[] = {
		{ "version", no_argument, 0, 'v' },
		{ "new-class", required_argument, 0, 'b' },
		{ "view-class", required_argument, 0, 'c' },
		{ "train-class", required_argument, 0, 'd' },
		{ "new-model", required_argument, 0, 'l' },
		{ "view-model",  required_argument, 0, 'm' },
		{ "train-model", required_argument, 0, 'n' },
		{ "show-accel", no_argument, 0, 's' },
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};
	
	/* don't display errors to stderr */
	opterr = 0;
	while ((long_opt_val = getopt_long(argc, argv, "vb:c:d:l:m:n:sh", long_opts, &long_opt_ind)) != -1) 
	{
		switch (long_opt_val)
		{
			case 'v': /* version */
				print_version();
				
				exit(0);
				break;
			case 'b': /* new-class */
				strncpy(file_name, optarg, sizeof(file_name));
				file_name[sizeof(file_name) / sizeof(file_name[0]) - 1] = '\0';
				
				return 'b';
				break;
			case 'c': /* view-class */
				strncpy(file_name, optarg, sizeof(file_name));
				file_name[sizeof(file_name) / sizeof(file_name[0]) - 1] = '\0';
				
				return 'c';
				break;
			case 'd': /* train-class */
				strncpy(file_name, optarg, sizeof(file_name));
				file_name[sizeof(file_name) / sizeof(file_name[0]) - 1] = '\0';
				
				return 'd';
				break;
			case 'l': /* new-model */
				strncpy(file_name, optarg, sizeof(file_name));
				file_name[sizeof(file_name) / sizeof(file_name[0]) - 1] = '\0';
				
				return 'l';
				break;
			case 'm': /* view-model */
				strncpy(file_name, optarg, sizeof(file_name));
				file_name[sizeof(file_name) / sizeof(file_name[0]) - 1] = '\0';

				return 'm';
				break;
			case 'n': /* train-model */
				strncpy(file_name, optarg, sizeof(file_name));
				file_name[sizeof(file_name) / sizeof(file_name[0]) - 1] = '\0';

				return 'n';
				break;
			case 's':
				return 's';
				break;
			case 'h':
			case '?':
				print_usage(argv[0]);
				
				exit(0);
				break;
			default:
				break;
		}
	}
		
	return 1;
}

/*
 * catches the termination signal and disposes the Wii
 */
static void class_handle_signal(int signal)
{
	switch (signal)
	{
		case SIGINT:
		case SIGTERM:
			wii_set_leds(&wii, 1, 0, 0, 0);
			wii_disconnect(&wii);
			/* when finished with training the class, save it to file */
			gauss_mix_write_3d(&gauss_mix, file_name);
			
			printf("Disconnected.\n");
			fflush(stdout);
			fflush(stderr);
			exit(0);
			break;
		default:
			break;
	}
}

static void model_handle_signal(int signal)
{
	switch (signal)
	{
		case SIGINT:
		case SIGTERM:
			wii_set_leds(&wii, 1, 0, 0, 0);
			wii_disconnect(&wii);
			/* when finished with training the class, save it to file */
			hmm_write_3d(&hmm, file_name);
			
			printf("Disconnected.\n");
			fflush(stdout);
			fflush(stderr);
			exit(0);
			break;
		default:
			break;
	}
}

static void show_handle_signal(int signal)
{
	switch (signal)
	{
		case SIGINT:
		case SIGTERM:
			wii_set_leds(&wii, 1, 0, 0, 0);
			wii_disconnect(&wii);
			
			printf("Disconnected.\n");
			fflush(stdout);
			fflush(stderr);
			exit(0);
			break;
		default:
			break;
	}
}
