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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>

#include "gesphone.h"
#include "wii.h"

/* cycle that reads reports from the Wii */
static void wii_read(struct wii_t *wii);
/* write a report to the Wii */
static int wii_write(struct wii_t *wii, unsigned char *report, unsigned int report_len);
/* request calibration report */
static void wii_req_cal(struct wii_t *wii);
/* request continuous acceleration reports */
static void wii_req_cont_accel(struct wii_t *wii);

/* 
 * initialize the Wii
 */
void wii_init(struct wii_t * wii)
{
	wii->in_sock = -1;
	wii->out_sock = -1;
	wii->is_calibrated = 0;
	wii->handle_accel = 0;
}

/* 
 * search for the Wii among all bluetooth devices
 */
int wii_search(struct wii_t *wii, int timeout)
{
	int dev_id;	
	int dev_sock;
	int dev_count;
	int i;
	int found = -1;
	
	dev_id = hci_get_route(0);
	if (dev_id < 0)
	{
		perror("hci_get_route");
		return -1;
	}
	
	dev_sock = hci_open_dev(dev_id);
	if (dev_sock < 0)
	{
		perror("hci_open_dev");
		return -1;
	}

	inquiry_info dev_prop_arr[255];
	inquiry_info * dev_prop = dev_prop_arr;
	memset(&dev_prop_arr, 0, sizeof(dev_prop_arr));
	
	dev_count = hci_inquiry(dev_id, timeout, 255, 0, &dev_prop, IREQ_CACHE_FLUSH);
	if (dev_count < 0)
	{
		perror("hci_inquiry");
		return -1;
	}
	
	for (i = 0; i < dev_count; i++)
	{
		char addr[18];
		char name[256];
		
		ba2str(&dev_prop[i].bdaddr, addr);
		memset(name, 0, sizeof(name));
		
		if (hci_read_remote_name(dev_sock, &dev_prop[i].bdaddr, sizeof(name), name, 0) < 0)
			strcpy(name, "[unknown]");
		printf("%s %s\n", addr, name);
			
		if ((dev_prop[i].dev_class[0] == WII_DEV_CLASS_0) &&
			(dev_prop[i].dev_class[1] == WII_DEV_CLASS_1) &&
			(dev_prop[i].dev_class[2] == WII_DEV_CLASS_2))
		{
			wii->bdaddr = dev_prop[i].bdaddr;
			found = 0;
		}
	}
	
	close(dev_sock);
	
	return found;
}

/* 
 * connect to the Wii and open sockets
 */
int wii_connect(struct wii_t *wii)
{
	struct sockaddr_l2 addr;
	
	if (!wii)
		return -1;
		
	addr.l2_family = AF_BLUETOOTH;
	addr.l2_bdaddr = wii->bdaddr;

	/* get output socket */		
	wii->out_sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (wii->out_sock < 0)
	{
		perror("socket");
		return -1;
	}
	
	addr.l2_psm = htobs(WII_OUTPUT_CHANNEL);
	
	/* connect to output socket */
	if (connect(wii->out_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("connect");
		return -1;
	}
	
	/* get input socket */
	wii->in_sock = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (wii->in_sock < 0)
	{
		perror("socket");
		close(wii->out_sock);
		return -1;
	}
	
	addr.l2_psm = htobs(WII_INPUT_CHANNEL);
	
	/* connect to input socket */
	if (connect(wii->in_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("connect");
		close(wii->out_sock);
		return -1;
	}
	
	return 0;
}

/*
 * disconnect the Wii and close sockets 
 */
void wii_disconnect(struct wii_t *wii)
{
	if (!wii)
		return;
	
	close(wii->in_sock);
	close(wii->out_sock);
	
	wii_init(wii);
}

/* 
 * turn leds 1, 2, 3, or 4 on, or off
 */
void wii_set_leds(struct wii_t *wii, int led1, int led2, int led3, int led4)
{
	unsigned int report_len = 3;
	unsigned char report[report_len];
	
	report[0] = WII_SET_REPORT | WII_BT_OUTPUT;
	report[1] = WM_CMD_LED;
	report[2] = (led1 ? WII_LED_1 : WII_LED_NONE) | (led2 ? WII_LED_2 : WII_LED_NONE) |
		(led3 ? WII_LED_3 : WII_LED_NONE) | (led4 ? WII_LED_4 : WII_LED_NONE);
		
	wii_write(wii, report, report_len);
}

/*
 * cycle that reads reports from the Wii
 */
static void wii_read(struct wii_t *wii)
{
	const unsigned int report_len = 23;
	unsigned char report[report_len];
	
	if (!wii)
		return;
	
	while (1)
	{
		if (read(wii->in_sock, report, report_len) < 0)
		{
			perror("read");
			continue;
		}

		switch (report[1])
		{
			case WII_RPT_BTN_ACC:
				/* we need to first read the calibration data */
				if (!wii->is_calibrated)
					break;
			
				struct accel_3d_t accel;	
				/* calibration data was read in a previous iteration and we can continue */
				
				accel.val[0] = ((float)(report[4] - wii->cal_zero.val_x)) / ((float)(wii->cal_one.val_x - wii->cal_zero.val_x));
				accel.val[1] = ((float)(report[5] - wii->cal_zero.val_y)) / ((float)(wii->cal_one.val_y - wii->cal_zero.val_y));
				accel.val[2] = ((float)(report[6] - wii->cal_zero.val_z)) / ((float)(wii->cal_one.val_z - wii->cal_zero.val_z));
				
				/* make callback to the function that handles accelertion */
				wii->handle_accel(accel);
				
				break;
			case WII_RPT_READ:
				/* read calibration data */
				wii->cal_zero.val_x = report[7];
				wii->cal_zero.val_y = report[8];
				wii->cal_zero.val_z = report[9];
				
				wii->cal_one.val_x = report[11];
				wii->cal_one.val_y = report[12];
				wii->cal_one.val_z = report[13];
				
				/* we've read the calibration and we're done here */
				wii->is_calibrated = 1;
				return;
				
				break;
			default:
				break;
		}
	}
	
	return;
}

/*
 * request calibration and continuous acceleration reports
 */
void wii_talk(wii_t *wii)
{	
	if (!wii)
		return;
	
	/* first, read the calibration */
	printf("Requesting calibration... ");
	wii_req_cal(wii);
	wii_read(wii);
	printf("done.\n");
	
	/* second, read the continuous acceleration */
	printf("Reading accelerometer...\n");
	wii_req_cont_accel(wii);
	/* won't get out of the loop until the program is terminated */
	wii_read(wii);
}

/* 
 * write a report to the Wii 
 */
static int wii_write(struct wii_t *wii, unsigned char *report, unsigned int report_len)
{
	if (!wii)
		return -1;
		
	return write(wii->out_sock, report, report_len);	
}

/*
 * request calibration report 
 */
static void wii_req_cal(struct wii_t *wii)
{
	unsigned int report_len = 8;
	unsigned char report[report_len];
	
	if (!wii)
		return;
	
	report[0] = WII_SET_REPORT | WII_BT_OUTPUT;
	report[1] = WM_CMD_READ_DATA;
	*(int *)(report + 2) = htonl(WII_MEM_OFFSET_CAL);
	*(short *)(report + 6) = htons(7);
	
	wii_write(wii, report, report_len);
}

/* 
 * request continuous acceleration reports 
 */
static void wii_req_cont_accel(struct wii_t *wii)
{
	unsigned int report_len = 4;
	unsigned char report[report_len];
	
	if (!wii)
		return;
	
	report[0] = WII_SET_REPORT | WII_BT_OUTPUT;
	report[1] = WM_CMD_REPORT_TYPE;
	report[2] = WII_CONTINUOUS_ON;
	report[3] = WII_RPT_BTN_ACC;
	
	wii_write(wii, report, report_len);
}
