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

#ifndef WII_H_
#define WII_H_

#include <pthread.h>
#include <bluetooth/bluetooth.h>

#include "gesphone.h"

#define WII_DEV_CLASS_0				0x04
#define WII_DEV_CLASS_1				0x25
#define WII_DEV_CLASS_2				0x00
#define WII_VENDOR_ID				0x057E
#define WII_PRODUCT_ID				0x0306

/* Communication channels */
#define WII_OUTPUT_CHANNEL			0x11
#define WII_INPUT_CHANNEL			0x13

#define WII_SET_REPORT				0x50

#define WM_CMD_LED					0x11
#define WM_CMD_REPORT_TYPE			0x12
#define WM_CMD_RUMBLE				0x13
#define WM_CMD_IR					0x13
#define WM_CMD_CTRL_STATUS			0x15
#define WM_CMD_WRITE_DATA			0x16
#define WM_CMD_READ_DATA			0x17
#define WM_CMD_IR_2					0x1A

#define WII_BT_INPUT				0x01
#define WII_BT_OUTPUT				0x02

#define WII_CONTINUOUS_ON			0x04
#define WII_CONTINUOUS_OFF			0x00

/* led bit masks */
#define WII_LED_NONE				0x00
#define WII_LED_1					0x10
#define WII_LED_2					0x20
#define WII_LED_3					0x40
#define WII_LED_4					0x80

#define WII_MEM_OFFSET_CAL			0x16

#define WII_RPT_BTN_ACC				0x31
#define WII_RPT_READ				0x21

/* calibration */
typedef struct cal_t {
	unsigned char val_x;
	unsigned char val_y;
	unsigned char val_z;
} cal_t;

/* wii remote */
typedef struct wii_t {
	bdaddr_t bdaddr;
	int out_sock;
	int in_sock;
	
	int is_calibrated;
	cal_t cal_zero; /* calibration on zero g */
	cal_t cal_one; /* calibration on one g */
	handle_accel_3d_t handle_accel;	
} wii_t;

/* initialize the Wii */
void wii_init(struct wii_t *wii);
/* search for the Wii among all bluetooth devices */
int wii_search(struct wii_t *wii, int timeout);
/* connect to the Wii and open sockets */
int wii_connect(struct wii_t *wii);
/* disconnect the Wii and close sockets */
void wii_disconnect(struct wii_t *wii);
/* turn leds 1, 2, 3, or 4 on, or off */
void wii_set_leds(struct wii_t *wii, int led1, int led2, int led3, int led4);
/* request calibration and continuous acceleration reports */
void wii_talk(struct wii_t *wii);

#endif /*WII_H_*/
