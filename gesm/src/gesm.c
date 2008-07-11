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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "accelneo.h"
#include "accelwii.h"
#include "ges.h"
#include "gesm.h"
#include "gui.h"

#define VERSION "0.1"

static struct neo_t neo;
static struct wii_t wii;
static struct seq_3d_t seq;
static unsigned char is_pressed;

struct gauss_mix_3d_t gauss_mix;
struct gauss_mix_3d_t gauss_mix_est;

static char file[1024];

/* */
static void print_header(void);
/* */
static void print_version(void);
/* */
static void print_usage(void);
/* */
static void wii_signal_cb(int signal);
/* */
static void neo_signal_cb(int signal);
/* */
void cmd_accel_cb(unsigned char pressed, struct accel_3d_t accel);
/* */
void cmd_new_class_cb(unsigned char pressed, struct accel_3d_t accel);
/* */
void cmd_new_model_cb(unsigned char pressed, struct accel_3d_t accel);

/*
 * 
 */
int main(int argc, char *argv[])
{
	enum device dev = dev_none;
	char cmd = '_';
	char dir[1024];
	char *p;
	
	int long_opt_ind = 0;
	int long_opt_val = 0;
	
	static struct option long_opts[] = {
		{ "wii1", no_argument, 0, 'w' },
		{ "neo2", no_argument, 0, 'q' },
		{ "neo3", no_argument, 0, 'z' },
		{ "dir", required_argument, 0, 'd' },
		{ "gui", no_argument, 0, 'g' },
		{ "accel", no_argument, 0, 'a' },
		{ "new", required_argument, 0, 'n' },
		{ "view", required_argument, 0, 'o' },
		{ "train", required_argument, 0, 'e' },
		{ "version", no_argument, 0, 'v' },
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	print_header();
	
	dir[0] = '\0';
	file[0] = '\0';
	opterr = 0;
	while ((long_opt_val = getopt_long(argc, argv, "wqzd:gan:o:e:vh", long_opts, &long_opt_ind)) != -1) 
	{
		switch (long_opt_val)
		{
			case 'w': /* --wii1 */
				dev = (dev == dev_none) ? dev_wii1 : dev;
				break;
			case 'q': /* --neo2 */
				dev = (dev == dev_none) ? dev_neo2 : dev;
				break;
			case 'z': /* --neo3 */
				dev = (dev == dev_none) ? dev_neo3 : dev;
				break;
			case 'd': /* --dir */
				strncpy(dir, optarg, sizeof(dir));
				dir[sizeof(dir) / sizeof(dir[0]) - 1] = '\0';
				break;
			case 'n': /* --new */
			case 'o': /* --view */
			case 'e': /* --train */
				strncpy(file, optarg, sizeof(file));
				file[sizeof(file) / sizeof(file[0]) - 1] = '\0';
			case 'g': /* --gui */
			case 'a': /* --accel */
				cmd = long_opt_val;
				break;
			case 'v': /* --version */
				print_version();
				
				exit(0);
				break;
			case 'h': /* --help */
			case '?':
				print_usage();
				
				exit(0);
				break;
		}
	}
	
	/* minimum requirements */
	if ((dev == dev_none) || (dir[0] == '\0') || (cmd == '_'))
	{
		print_usage();
		exit(1);
	}
	/* should be ok here */
	
	/* device handshake */
	switch (dev)
	{
		case dev_wii1: /* --wii1 */
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
			wii_set_leds(&wii, 0, 0, 0, 1);
			
			signal(SIGINT, wii_signal_cb);
			signal(SIGTERM, wii_signal_cb);
			break;
		case dev_neo2: /* --neo2 */
			if (!neo_open(&neo, neo_accel2)) {
				fprintf(stderr, "Could not open top accelerometer.\n");
				exit(1);
			}
			printf("Opened.\n");
			
			signal(SIGINT, neo_signal_cb);
			signal(SIGTERM, neo_signal_cb);
			break;
		case dev_neo3: /* --neo3 */
			if (!neo_open(&neo, neo_accel3)) {
				fprintf(stderr, "Could not open bottom accelerometer.\n");
				exit(1);
			}
			printf("Opened.\n");
			
			signal(SIGINT, neo_signal_cb);
			signal(SIGTERM, neo_signal_cb);
			break;
		case dev_none:
			exit(1);
			break;
	}
	
	chdir(dir);
	p = rindex(file, '.');
	switch (cmd)
	{
		case 'g': /* --gui */
			main_gui(argc, argv, dir);
			break;
		case 'a': /* --accel */
			wii.handle_recv = cmd_accel_cb;
			neo.handle_recv = cmd_accel_cb;
			break;
		case 'n': /* --new */
			if ((p) && (strcmp(p, ".class") == 0)) { /* class */
				wii.handle_recv = cmd_new_class_cb;
				neo.handle_recv = cmd_new_class_cb;
			} else if ((p) && (strcmp(p, ".model") ==0)) { /* model */
				wii.handle_recv = cmd_new_model_cb;
				neo.handle_recv = cmd_new_model_cb;
			} else { /* unknown */
				fprintf(stderr, "Unrecognized file extension (has to be .class or .model).\n");
				exit(1);
			}
			
			break;
		case 'o': /* --view */
			break;
		case 'e': /* --train */
			break;
	}

	switch (dev)
	{
		case dev_wii1:
			wii_talk(&wii);
			break;
		case dev_neo2:
		case dev_neo3:
			neo_begin_read(&neo);
			break;
		case dev_none:
			exit(1);
			break;
	}
	
	return 0;	
}

/*
 * 
 */
static void print_header(void)
{
	printf("gesm: (C) 2008 OpenMoko Inc. Paul-Valentin Borza www.borza.ro\n"
		"This program is free software under the terms of the GNU General Public License.\n\n");
}

/*
 * 
 */
static void print_version(void)
{
	printf("Version: %s\n", VERSION);
}

/*
 * 
 */
static void print_usage(void)
{	
	printf("Usage: gesm --wii1 --dir DIRECTORY COMMAND\n"
		"   or: gesm --neo2 --dir DIRECTORY COMMAND\n"
		"   or: gesm --neo3 --dir DIRECTORY COMMAND\n"
		"   or: gesm --version\n"
		"   or: gesm --help\n"
		"Commands:\n"
		"   --gui       \tshows a graphical user interface\n"
		"   --accel     \tshows recorded acceleration values\n"
		"   --new   FILE\tcreates a new class (.class) or model (.model)\n"
		"   --view  FILE\tvisualizes a class (.class) or model (.model)\n"
		"   --train FILE\ttrains a class (.class) or model (.model)\n"
		"Remarks:\n"
		"   neo2 refers to the top accelerometer, and\n"
		"   neo3 refers to the bottom accelerometer;\n");
}

/*
 * 
 */
static void wii_signal_cb(int signal)
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

/*
 * 
 */
static void neo_signal_cb(int signal)
{
	switch (signal)
	{
		case SIGINT:
		case SIGTERM:
			neo_close(&neo);
			printf("Closed.\n");

			fflush(stdout);
			fflush(stderr);
			exit(0);
			break;
		default:
			break;
	}
}

/*
 * 
 */
void cmd_accel_cb(unsigned char pressed, struct accel_3d_t accel)
{
	if (pressed)
	{
		printf("%+f\t%+f\t%+f\n", accel.val[0], accel.val[1], accel.val[2]);
	}	
}

/*
 * 
 */
void cmd_new_class_cb(unsigned char pressed, struct accel_3d_t accel)
{
	if (pressed) {
		is_pressed = 1;
	}
	
	if ((pressed) && (is_pressed)) {
		seq.each[seq.index++] = accel;
		printf("%+f\t%+f\t%+f\n", accel.val[0], accel.val[1], accel.val[2]);
	}
	else if ((!pressed) && (is_pressed))
	{
		unsigned int feature_len = seq.index - 2;
		struct sample_3d_t feature[feature_len];
		struct sample_3d_t sum;
		sum.val[0] = 0.0;
		sum.val[1] = 0.0;
		sum.val[2] = 0.0;
		
		/* compute the feature vector for each frame */
		printf("Features:\n");
		int i;
		for (i = 1; i < seq.index - 1; i++) {
			ges_fea_3d(&seq, i, i - 1, &feature[i - 1]);
			sum.val[0] += feature[i - 1].val[0];
			sum.val[1] += feature[i - 1].val[1];
			sum.val[2] += feature[i - 1].val[2];
			printf("%+f\t%+f\t%+f\n", feature[i - 1].val[0], feature[i - 1].val[1], feature[i - 1].val[2]);
		}
		
		/***
		 * NEW CLASS
		 ***/
		/* one mixture */
		gauss_mix_create_3d(&gauss_mix, 1);
		gauss_mix_rand_3d(&gauss_mix);
		gauss_mix.each[0].mean[0] = sum.val[0] / feature_len;
		gauss_mix.each[0].mean[1] = sum.val[1] / feature_len;
		gauss_mix.each[0].mean[2] = sum.val[2] / feature_len;
		gauss_mix_print_3d(&gauss_mix);
		/* check whether we want to commit changes */
		char response;
		printf("Make changes and save file? (Y/N)\n");
		do {
			response = getchar();
			response = toupper(response);
			/* clean the rest of the input */
			while (getchar() != '\n')
				;
		} while ((response != 'Y') && (response != 'N'));
		/* do selection */
		if (response == 'Y') {
			gauss_mix_write_3d(&gauss_mix, file);
			printf("Saved changes to file.\n");
			
			/* don't forget to clean up */
			gauss_mix_delete_3d(&gauss_mix);
			
			/* we're done */
			kill(getpid(), SIGTERM);
		} else if (response == 'N') {
			printf("Discarded changes.\n");
		}
		gauss_mix_delete_3d(&gauss_mix);
		
		/* reset counter */
		seq.index = 0;
		is_pressed = 0;
	}
}

/*
 * 
 */
void cmd_new_model_cb(unsigned char pressed, struct accel_3d_t accel)
{
	if (pressed)
	{
		printf("%+f\t%+f\t%+f\n", accel.val[0], accel.val[1], accel.val[2]);
	}	
}

