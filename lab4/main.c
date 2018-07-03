/* LAB1 template usage example */
#include <signal.h>
#include <stdlib.h>

#include "lift.h"
#include "lift_io.h"
#include "debug_print.h"

void termination_handler ( int signum )
{
	lift_io_close ();
	exit (1);
}
void set_termination_handler ()
{
	struct sigaction action;

	action.sa_handler = termination_handler;
	sigemptyset ( &action.sa_mask );
	action.sa_flags = 0;
	sigaction ( SIGINT, &action, NULL );
	sigaction ( SIGHUP, &action, NULL );
	sigaction ( SIGTERM, &action, NULL );
}

int main ( int argc, char *argv[] )
{
	int i, j = 0;
	int button_cnt[NUM_LIFT]; /* count number of presses */
	unsigned long button_lr[NUM_LIFT]; /* last release of lift button */
	struct lift lift[NUM_LIFT] = {
		{
			.floor = 0,
			.dir = DIR_NONE,
			.door = DOOR_CLOSED,
			.state = LIFT_STOPPED,
			.t_next_change = 0
		},
		{
			.floor = 3.5,
			.dir = DIR_DOWN,
			.door = DOOR_CLOSED,
			.state = LIFT_MOVING,
			.t_next_change = 0
		}
	};
	struct buttons buttons = {
		.in = {{1,0,0,0,0},{0,0,1,1,0}},
		.open = 0,
		.stop = 0,
		.cont = 0,
		.out = {{0,0},{0,0},{0,0},{0,0},{0,0}}
	};

	lift_io_init();
	set_termination_handler ();

	for ( i = 0; i < NUM_LIFT; i++ ) {
		button_cnt[i] = 0;
		button_lr[i] = get_time();
	}

	while (1) {
		unsigned long now = get_time();

		lift_io_update();

		/* get inputs from buttons */
		for ( i = 0; i < NUM_LIFT; i++ ) {
			struct sensor sensor = lift_io__get_lift_button (i);

			if ( sensor.state == OFF && sensor.state_new == ON ) {
				/* button just pressed */
				button_cnt[i]++;
				lift_io__set_lift_button (i);
			}
			else if ( sensor.state == ON && sensor.state_new == OFF ) {
				/* button just released */
				button_lr[i] = now;
				lift_io__set_lift_button (i);
			}
			else if ( sensor.state == OFF && button_cnt[i] > 0 &&
				button_lr[i] + RELEASE_TIMEOUT < now )
			{
				/* request for floor button_cnt[i] is complete */
				buttons.in[i][button_cnt[i]-1] = 1;
				button_cnt[i] = 0;
			}
		}
		for ( j = 0; j < NUM_FLOOR; j++ ) {
			struct sensor sensor = lift_io__get_floor_button (j);

			if ( sensor.state == OFF && sensor.state_new == ON ) {
				/* button press */
				/* do nothing - wait for release */
				lift_io__set_floor_button (j);
			}
			else if ( sensor.state == ON && sensor.state_new == OFF ) {
				/* button release */
				/* long or short? */
				if ( now - sensor.t < SHORT_PRESS_TIMEOUT ) {
					/* short press */
					buttons.out[j][1] = 1;
				}
				else {
					/* long press */
					buttons.out[j][0] = 1;
				}
				update_floor_stats(j);
				lift_io__set_floor_button (j);
			}
		}

		lift_process ( lift, &buttons );

		if(ENABLE_DBG_PRINT == 0)
			lift_print ( "LIFT MODEL", lift, &buttons );
		lift_io__set_leds ( lift, &buttons );
		#ifdef USE_RPI_SENSE
			sense_update();
		#endif
		delay_T ();
	}

	lift_io_close ();

	return 0 ;
}
