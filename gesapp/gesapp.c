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
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "gauss.h"
#include "ges.h"
#include "gesapp.h"
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

/* print welcome message */
static void print_header(void);
/* print all command line options */
static void print_usage(char *file_name);
/* print current version */
static void print_version(void);
/* parse options given at command line */
static char parse_cmd(int argc, char **argv);
/* */
void class_handle_accel(unsigned char pressed, struct accel_3d_t accel);
/* catches the termination signal and closes the Wii */
static void class_handle_signal(int signal);

/* this handler is called whenever the Wii sends acceleration reports */
void class_handle_accel(unsigned char pressed, struct accel_3d_t accel)
{
	if (pressed)
	{
		is_pressed = 1;
	}
	
	if ((pressed) && (is_pressed))
	{
		seq.index = (seq.index + 1) % FRAME_LEN;
		seq.each[seq.index] = accel;
	}
	else if ((!pressed) && (is_pressed))
	{
		is_pressed = 0;
		
		struct sample_3d_t accels[seq.index];
		//memcpy(accels, seq.each, sizeof(accels));
		
		int i;
		for (i = 1; i < seq.index; i++)
		{
			sample_3d_t feature;
	/* magnitude at time t (current time) */
	feature.val[0] = sqrt(
		pow(seq.each[i].val[0], 2) +
		pow(seq.each[i].val[1], 2) +
		pow(seq.each[i].val[2], 2));
	/* OPTIMIZE: computes magnitudes twice at t and t + 1*/
	/* magnitude at time t-1 */
	feature.val[1] = sqrt(
		pow(seq.each[i - 1].val[0], 2) +
		pow(seq.each[i - 1].val[1], 2) +
		pow(seq.each[i - 1].val[2], 2));
	/* delta magnitude between time t and t - 1 */
	feature.val[2] = sqrt(
		pow(seq.each[i].val[0] - seq.each[i - 1].val[0], 2) +
		pow(seq.each[i].val[1] - seq.each[i - 1].val[1], 2) +
		pow(seq.each[i].val[2] - seq.each[i - 1].val[2], 2));	

			accels[i] = feature;
		}
		
		gauss_mix_den_est_3d(&gauss_mix, &gauss_mix_est, accels, seq.index);
		gauss_mix_print_3d(&gauss_mix_est);
		/* copies gauss_mix_est to gauss_mix (need to re-arrange the params to be consistent */
		gauss_mix_copy_3d(&gauss_mix_est, &gauss_mix);
		seq.index = 0;
	}
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

 /*
 * do not run in terminal: sudo hidd --search
 */
int main(int argc, char **argv)
{
	unsigned int mix_len;
	print_header();
	
	/* get the configuration file name */
	switch (parse_cmd(argc, argv))
	{
		case 'b': /* new-class */
			printf("Class %s\n", file_name);
			printf("Number of mixtures: ");
			scanf("%d", &mix_len);
			gauss_mix_create_3d(&gauss_mix, mix_len);
			gauss_mix_rand_3d(&gauss_mix);
			gauss_mix_print_3d(&gauss_mix); 
			gauss_mix_write_3d(&gauss_mix, file_name);
			
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
			printf("Connected.\n");
	
			/* catch terminate signal to stop the read thread and close sockets */
			signal(SIGINT, class_handle_signal);
			signal(SIGTERM, class_handle_signal);
	
			wii_set_leds(&wii, 0, 0, 0, 1);
			wii_talk(&wii); /* will enter loop and never get out */
			
			break;
		case 'l': /* new-model */
			
			break;
		case 'm': /* view-model */
		
			break;
		case 'n': /* new-model */
		
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
	printf("\tClass: --new-class <class> | --view-class <class> | --train-class <class>\n");
	printf("\tModel: --new-model <model> | --view-model <model> | --train-model <model>\n");
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
static char parse_cmd(int argc, char **argv)
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
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};
	
	/* don't display errors to stderr */
	opterr = 0;
	while ((long_opt_val = getopt_long(argc, argv, "v1:2:p:q:c:m:h", long_opts, &long_opt_ind)) != -1) 
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
			case 'h':
			case '?':
				print_usage(argv[0]);
				
				exit(0);
				break;
		}
	}
		
	return 1;
}
