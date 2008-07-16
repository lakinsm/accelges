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
static struct class_2c_t endpoint;
static unsigned char is_pressed;
static unsigned char detected;

static struct gauss_mix_3d_t gauss_mix;

/* vars from arguments that are used in callbacks */
static char file[1024];
static unsigned char verbose;
static unsigned char confirm;

static class_process handle_class;
static model_process handle_model;

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
static void cmd_accel_cb(unsigned char pressed, struct accel_3d_t accel);
/* */
static void cmd_class_cb(unsigned char pressed, struct accel_3d_t accel);
/* */
static void cmd_class_new_begin(char *file);
/* */
static void cmd_class_new_cb(struct sample_3d_t feature[], unsigned int feature_len);
/* */
static void cmd_class_new_end(char *file);
/* */
static void cmd_class_train_begin(char *file);
/* */
static void cmd_class_train_cb(struct sample_3d_t feature[], unsigned int feature_len);
/* */
static void cmd_class_train_end(char *file);
/* */
static void cmd_class_view_begin(char *file);
/* */
static void cmd_class_view_end(char *file);
/* */
static void cmd_model_cb(unsigned char pressed, struct accel_3d_t accel);
/* */
static void cmd_model_new_begin(char *file);
/* */
static void cmd_model_new_cb(struct accel_3d_t accels[], unsigned int accel_len);
/* */
static void cmd_model_new_end(char *file);
/* */
static void cmd_model_train_begin(char *file);
/* */
static void cmd_model_train_cb(char *file);
/* */
static void cmd_model_train_end(char *file);
/* */
static void cmd_model_view_begin(char *file);
/* */
static void cmd_model_view_end(char *file);
/* */
static unsigned char do_confirm(void);
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
		{ "verbose", no_argument, 0, 'p' },
		{ "confirm", no_argument, 0, 'c' },
		{ "version", no_argument, 0, 'v' },
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 }
	};

	print_header();
	
	dir[0] = '\0';
	file[0] = '\0';
	verbose = 0;
	confirm = 0;
	opterr = 0;
	while ((long_opt_val = getopt_long(argc, argv, "wqzd:gan:o:e:pcvh", long_opts, &long_opt_ind)) != -1) 
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
			case 'p': /* --verbose */
				verbose = 1;
				break;
			case 'c': /* --confirm */
				confirm = 1;
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
	if ((cmd != 'g') && (cmd != 'o')) /* ! --gui and ! --view */
	{
		switch (dev)
		{
			case dev_wii1: /* --wii1 */
				printf("Searching... (Press 1 and 2 on the Wii)\n");
				if (wii_search(&wii, 5) < 0) {
					fprintf(stderr, "Could not find the Wii.\n");
					exit(1);
				}
				printf("Found.\n");
	
				if (wii_connect(&wii) < 0) {
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
				printf("Connected to top accelerometer.\n");
			
				signal(SIGINT, neo_signal_cb);
				signal(SIGTERM, neo_signal_cb);
				break;
			case dev_neo3: /* --neo3 */
				if (!neo_open(&neo, neo_accel3)) {
					fprintf(stderr, "Could not open bottom accelerometer.\n");
					exit(1);
				}
				printf("Connected to bottom accelerometer.\n");
			
				signal(SIGINT, neo_signal_cb);
				signal(SIGTERM, neo_signal_cb);
				break;
			case dev_none:
				exit(1);
				break;
		}
	}
	
	/* prepare callbacks for requested command */
	switch (cmd)
	{
		case 'g': /* --gui */
			main_gui(argc, argv, dir);
			break;
		case 'a': /* --accel */
			chdir(dir);
			wii.handle_recv = cmd_accel_cb;
			neo.handle_recv = cmd_accel_cb;
			break;
		case 'n': /* --new */
		case 'o': /* --view */
		case 'e': /* --train */
			chdir(dir);
			p = rindex(file, '.');
			if ((p) && (strcmp(p, ".class") == 0)) { /* class */
				if (cmd == 'n') { /* --new class */
					wii.handle_recv = cmd_class_cb;
					neo.handle_recv = cmd_class_cb;
					handle_class = cmd_class_new_cb;
					cmd_class_new_begin(file);
				} else if (cmd == 'o') { /* --view class */
					wii.handle_recv = 0;
					neo.handle_recv = 0;
					handle_class = 0;
					cmd_class_view_begin(file);
				} else if (cmd == 'e') { /* --train class */
					wii.handle_recv = cmd_class_cb;
					neo.handle_recv = cmd_class_cb;
					handle_class = cmd_class_train_cb;
					cmd_class_train_begin(file);
				}
			} else if ((p) && (strcmp(p, ".model") == 0)) { /* model */
				if (cmd == 'n') { /* --new model */
					wii.handle_recv = cmd_new_model_cb;
					neo.handle_recv = cmd_new_model_cb;
				} else if (cmd == 'o') { /* --view model */
					
				} else if (cmd == 'e') { /* --train model */
					wii.handle_recv = cmd_train_model_cb;
					neo.handle_recv = cmd_train_model_cb;
				}
			} else { /* unknown */
				fprintf(stderr, "Unrecognized file extension (has to be .class or .model).\n");
				exit(1);
			}
			
			break;
	}
	
	if ((cmd != 'g') && (cmd != 'o')) /* ! --gui and ! --view */
	{
		/* begin reading accel values */
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
	}
	
	return 0;	
}

/*
 * 
 */
static void print_header(void)
{
	printf("gesm: (C) 2008 Openmoko Inc. Paul-Valentin Borza <paul@borza.ro>\n"
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
	printf("Usage: gesm --wii1 --dir DIRECTORY COMMAND OPTION\n"
		"   or: gesm --neo2 --dir DIRECTORY COMMAND OPTION\n"
		"   or: gesm --neo3 --dir DIRECTORY COMMAND OPTION\n"
		"   or: gesm --version\n"
		"   or: gesm --help\n"
		"Commands:\n"
		"   --gui       \tshows a graphical user interface\n"
		"   --accel     \tshows recorded acceleration values\n"
		"   --new   FILE\tcreates a new class (.class) or model (.model)\n"
		"   --view  FILE\tvisualizes a class (.class) or model (.model)\n"
		"   --train FILE\ttrains a class (.class) or model (.model)\n"
		"Options:\n"
		"   --verbose   \tdisplays additional information\n"
		"   --confirm   \tasks confirmation for save\n"
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
static void cmd_accel_cb(unsigned char pressed, struct accel_3d_t accel)
{
	if (pressed) {
		printf("%+f\t%+f\t%+f\n", accel.val[0], accel.val[1], accel.val[2]);
	}	
}

/*
 * 
 */
static void cmd_class_cb(unsigned char pressed, struct accel_3d_t accel)
{
	if (pressed) {
		is_pressed = 1;
	}
	
	if ((pressed) && (is_pressed)) {
		seq.each[seq.index++] = accel;
		if (verbose) {
			printf("%+f\t%+f\t%+f\n", accel.val[0], accel.val[1], accel.val[2]);
		}
	}
	else if ((!pressed) && (is_pressed))
	{
		unsigned int feature_len = seq.index - 2;
		struct sample_3d_t feature[feature_len];
		
		/* compute the feature vector for each frame */
		int i;
		for (i = 1; i < seq.index - 1; i++) {
			ges_fea_3d(&seq, i, i - 1, &feature[i - 1]);
		}
		
		if (handle_class) {
			handle_class(feature, feature_len);
		}
		
		/* reset counter */
		seq.index = 0;
		is_pressed = 0;
	}
}

/*
 * 
 */
static void cmd_class_new_begin(char *file)
{
	gauss_mix_create_3d(&gauss_mix, 1);
	gauss_mix_rand_3d(&gauss_mix);

	seq.index = 0;
	is_pressed = 0;
}

/*
 * 
 */
static void cmd_class_new_cb(struct sample_3d_t feature[], unsigned int feature_len)
{
	struct sample_3d_t sum;
	sum.val[0] = 0.0;
	sum.val[1] = 0.0;
	sum.val[2] = 0.0;
	
	if (verbose) {
		printf("Features:\n");
	}
	int i;
	for (i = 1; i < seq.index - 1; i++) {
		sum.val[0] += feature[i - 1].val[0];
		sum.val[1] += feature[i - 1].val[1];
		sum.val[2] += feature[i - 1].val[2];
		if (verbose) {
			printf("%+f\t%+f\t%+f\n", feature[i - 1].val[0], feature[i - 1].val[1], feature[i - 1].val[2]);
		}
	}
	/***
	 * NEW CLASS
	 ***/
	gauss_mix.each[0].mean[0] = sum.val[0] / feature_len;
	gauss_mix.each[0].mean[1] = sum.val[1] / feature_len;
	gauss_mix.each[0].mean[2] = sum.val[2] / feature_len;
	if (verbose) {
		gauss_mix_print_3d(&gauss_mix);
	}
	
	if (do_confirm()) {
		cmd_class_new_end(file);
		
		/* we're done */
		kill(getpid(), SIGTERM);
	}
	
	seq.index = 0;
	is_pressed = 0;
}

/*
 * 
 */
static void cmd_class_new_end(char *file)
{
	gauss_mix_write_3d(&gauss_mix, file);
	printf("Done.\n");
	
	gauss_mix_delete_3d(&gauss_mix);
}

/*
 * 
 */
static void cmd_class_train_begin(char *file)
{
	if (gauss_mix_read_3d(&gauss_mix, file) != 0)
	{
		fprintf(stderr, "Could not load class from file.");
		return;
	}
	gauss_mix_print_3d(&gauss_mix);

	seq.index = 0;
	is_pressed = 0;
}

/*
 * 
 */
static void cmd_class_train_cb(struct sample_3d_t feature[], unsigned int feature_len)
{
	struct gauss_mix_3d_t gauss_mix_est;
	gauss_mix_create_3d(&gauss_mix_est, gauss_mix.mix_len);
	gauss_mix_den_est_3d(&gauss_mix, &gauss_mix_est, feature, feature_len);
	if (verbose) {
		gauss_mix_print_3d(&gauss_mix_est);
	}

	if (do_confirm()) {
		gauss_mix_copy_3d(&gauss_mix, &gauss_mix_est);
		gauss_mix_delete_3d(&gauss_mix_est);
		cmd_class_train_end(file);
		
		/* we're done */
		kill(getpid(), SIGTERM);
	}
	gauss_mix_delete_3d(&gauss_mix_est);
	
	seq.index = 0;
	is_pressed = 0;
}

/*
 * 
 */
static void cmd_class_train_end(char *file)
{
	gauss_mix_write_3d(&gauss_mix, file);
	printf("Done.\n");
	
	gauss_mix_delete_3d(&gauss_mix);	
}

/*
 * 
 */
static void cmd_class_view_begin(char *file)
{
	if (gauss_mix_read_3d(&gauss_mix, file) != 0)
	{
		fprintf(stderr, "Could not load class from file.");
		return;
	}
	gauss_mix_print_3d(&gauss_mix);
	
	cmd_class_view_end(file);
}

/*
 * 
 */
static void cmd_class_view_end(char *file)
{
	gauss_mix_delete_3d(&gauss_mix);
}

/*
 * 
 */
static void cmd_new_model_cb(unsigned char pressed, struct accel_3d_t accel)
{
	/* increment and save current frame (uses a circular list) */
	unsigned int prev_index = seq.index;
	seq.index = (seq.index + 1) % FRAME_LEN;
	seq.each[seq.index] = accel;
	
	/* extract feature vector */
	sample_3d_t feature;
	ges_fea_3d(&seq, seq.index, prev_index, &feature);
	
	/* motion has index 1 and noise has index 0 */
	if (class_max_2c(&endpoint, feature) == 1)
	{
		/* we want motion */
		if (!detected)
		{
			seq.begin = seq.index;
			detected = 1;
			seq.till_end = FRAME_AFTER;
		}
	}
	else
	{
		/* noise detected */
		if (detected)
		{
			//printf("detected with size: %d\n", ges->seq.index - ges->seq.begin + 1);
			if (seq.till_end > 0)
				seq.till_end--;
		}
		if (seq.till_end == 0)
		{
			seq.end = seq.index;
			detected = 0;
			seq.till_end = FRAME_AFTER;
		
			//printf("Gesture detected with size: %d\n", ges->seq.end - ges->seq.begin + 1 - FRAME_AFTER);
				 
			/* case when begin < end */
			if (seq.begin < seq.end)
			{	
				if (seq.end - seq.begin > FRAME_DIF + FRAME_AFTER)
				{
					int before = 0;
					if (seq.begin > FRAME_BEFORE)
					{
						before = FRAME_BEFORE;
					}
					
					unsigned int frame_len = seq.end - seq.begin + 1;
					struct accel_3d_t accels[before + frame_len];
					memcpy(&accels[before], &seq.each[seq.begin], frame_len * sizeof(sample_3d_t));
					
					if (before > 0)
					{
						memcpy(&accels[0], &seq.each[seq.begin - before], before * sizeof(sample_3d_t));
					}
	
					cmd_new_model_process(accels, before + frame_len);
				}
			}
			else /* case when begin > end */
			{
				if (FRAME_LEN - seq.begin + seq.end > FRAME_DIF + FRAME_AFTER)
				{
					int before = 0;
					if (seq.begin - seq.end > FRAME_BEFORE)
					{
						before = FRAME_BEFORE;
					}
					unsigned int frame_len_end = FRAME_LEN - seq.begin;
					unsigned int frame_len_begin = seq.end + 1;
					unsigned int frame_len = frame_len_end + frame_len_begin;
					struct accel_3d_t accels[before + frame_len];
					memcpy(&accels[before], &seq.each[seq.begin], frame_len_end * sizeof(sample_3d_t));
					memcpy(&accels[before + frame_len_end], &seq.each[0], frame_len_begin * sizeof(accel_3d_t));
					
					if (before > 0)
					{
						memcpy(&accels[0], &seq.each[seq.begin - before], before * sizeof(sample_3d_t));
					}
					
					cmd_new_model_process(accels, before + frame_len); 
				}
			}
		}
	}

	//printf("%+f\t%+f\t%+f\n", accel.val[0], accel.val[1], accel.val[2]);	
}

/*
 * 
 */
static void cmd_new_model_process(struct accel_3d_t accels[], unsigned int accel_len)
{
	float prev_val[3] = { 0.0, 0.0, 0.0 };
	int count = 1;
	int i;
	for (i = 0; i < accel_len; i++)
	{	
		if (i % 17 == 0)
		{
			printf("%+f\t%+f\t%+f ***\n", prev_val[0] / count, 
				prev_val[1] / count, prev_val[2] / count);
			prev_val[0] = accels[i].val[0];
			prev_val[1] = accels[i].val[1];
			prev_val[2] = accels[i].val[2];
			count = 1;
		} else {
			prev_val[0] += accels[i].val[0];
			prev_val[1] += accels[i].val[1];
			prev_val[2] += accels[i].val[2];
			count++;
		}
		printf("%+f\t%+f\t%+f\n", accels[i].val[0], accels[i].val[1], accels[i].val[2]);
	}
	
	if (count > 0)
	{
		printf("*** %+f\t%+f\t%+f ***\n", prev_val[0] / count, 
			prev_val[1] / count, prev_val[2] / count);
	}
	
	printf("End of detection.\n");
	fflush(stdout);
}

/*
 * 
 */
static void cmd_train_model_cb(unsigned char pressed, struct accel_3d_t accel)
{
	
}

/*
 * 
 */
static unsigned char do_confirm(void)
{
	char response = 'Y';
	if (confirm) {
		printf("Confirm? (Y/N)\n");
		do {
			response = getchar();
			response = toupper(response);
			/* clean the rest of the input */
			while (getchar() != '\n')
				;
		} while ((response != 'Y') && (response != 'N'));
	}
	return (response == 'Y') ? 1 : 0;
}

