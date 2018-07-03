#pragma once

#ifdef USE_RPI_SENSE
	#include "rpi_sense.h"
	#define RED_STATE 1
	#define GREEN_STATE 2
	#define YELLOW_STATE 3
#endif

#ifdef USE_RPI
	#include "rpi_io.h"
#else
	#define ON  1
	#define OFF 0
#endif


#define SET_BIT(X,N)	do (X) |= 1 << (N); while(0)
#define CLEAR_BIT(X,N)	do (X) &= ~(1 << (N)); while(0)
#define TEST_BIT(X,N)   ( (X) & (1 << (N)) )

#define ADVANCED_PEDESTRIANS_ON_DEMAND_MODE 1
#define ADVANCED_VEHICLES_ON_DEMAND_MODE 		2


struct led {
	int state; /* ON/OFF */
	int last_state;
	int x,y;   /* "semaphore" on map - just for print */
	int sense_x, sense_y;
	int freq; /* blinking frequency: 0 not blinking,
	             otherwise "state" changes with 1/freq interval */
	unsigned long next_change; /* for blinking */
	int pin; /* pin on RPI */
};
#define LEDS 20

struct sensor {
	int state; /* ON/OFF */
	int x,y;   /* "sensor" on map */
	unsigned long t; /* time-stamp of change in "state" */
	int state_new; /* new state to be processed */
	unsigned long t_release; /* sensor release time - when using input from pipe */
	int pin;  /* pin on RPI */
	int is_long_pressed;
};


struct vehicle_sensor {
	struct sensor* sensor_forward;
	struct sensor* sensor_right;
	struct sensor* sensor_left;

};

#define SENSORS 20
#define VEHICLE_SENSORS 4

#define TEST_STRAIGHT(X) (X).sensor_forward->is_long_pressed
#define TEST_RIGHT(X) (X).sensor_right->is_long_pressed
#define TEST_LEFT(X) (X).sensor_left->is_long_pressed

#define CLEAR_VEHICLE_SENSOR(X) \
	do { \
			(X).sensor_forward->is_long_pressed = 0; \
			(X).sensor_right->is_long_pressed = 0; \
			(X).sensor_left->is_long_pressed = 0; \
		} while(0)



unsigned long get_time(); /* get time in milliseconds */
void delay_T (); /* delay for one "update" interval defined in sem_io.c */

//Custom functions
void semaphore_blinking_yellow();
void semaphore_basic_cycles_update();
void semaphore_advanced_cycles_update(unsigned long* sensors_data, char advanced_mode);


void semaphore_init();
void semaphore_update ();
void semaphore_close ();
void semaphore_print ();

struct led semaphore_get_led ( int id );
void semaphore_set_led ( int id, int state, int freq );

struct sensor* semaphore_get_sensor ( int id );
void semaphore_set_sensor ( int id ); /* acknowledge sensor data */
