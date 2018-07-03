#pragma once

#include "lift.h"

//#define USE_RPI

#ifdef USE_RPI
	#include "rpi_io.h"
#else
	#define ON  1
	#define OFF 0
#endif

#ifdef USE_RPI_SENSE
	#include "rpi_sense.h"
#endif

enum {BLACK = 0, RED = 1, GREEN = 2, BLUE = 3, MAGENTA=4, GOLD=5, PINK=6};

struct sense_led {
	int x;
	int y;
	int color;
};

struct led {
	int pin; /* pin on RPI */
	int state; /* ON/OFF */
	int freq; /* blinking frequency: 0 not blinking,
	             otherwise "state" changes with 1/freq interval */
	unsigned long next_change; /* for blinking */
	struct sense_led sense;
};
#define LEDS 14

struct sensor {
	int pin;  /* pin on RPI */
	int state; /* ON/OFF */
	unsigned long t; /* time-stamp of change in "state" */
	int state_new; /* new state to be processed */
	unsigned long t_release; /* sensor release time - when using input from pipe */
};
#define SENSORS 7

unsigned long get_time (); /* get time in milliseconds */
void delay_T (); /* delay for one "update" interval defined in sem_io.c */

void lift_io_init();
void lift_io_update ();
void lift_io_close ();

struct sensor lift_io__get_lift_button ( int lift_id );
void lift_io__set_lift_button ( int lift_id );
struct sensor lift_io__get_floor_button ( int floor_id );
void lift_io__set_floor_button ( int floor_id );
void lift_io__set_leds ( struct lift *lift, struct buttons *buttons );
