/* LAB1 usage example */
#include "sem_io.h"

#include <signal.h>
#include <stdlib.h>

void termination_handler ( int signum )
{
	semaphore_close ();
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

//#define SET_BIT(X,N)	do (X) |= 1 << (N); while(0)
//#define CLEAR_BIT(X,N)	do (X) &= ~(1 << (N)); while(0)
//#define TEST_BIT(X,N)   ( (X) & (1 << (N)) )

int main ( int argc, char *argv[] )
{
	int i;
	unsigned long leds, sensors, freq[LEDS] = {0};

	unsigned long t_last_sensor_activation = -1, t_cur_time;
	unsigned char concurrent_sensors = 0;

	static unsigned long t_treshold_interval = 300;

	unsigned char semaphore_mode = 0;

	semaphore_init();
	set_termination_handler ();

	leds = 0xffffffff; /* all leds ON */
	sensors = 0;
	for ( i = 0; i < LEDS; i++ )
		freq[i] = 0;

	if(argc == 2){
		semaphore_mode = atoi(argv[1]) % 5;
	}

	while (1)
	{
		semaphore_update();
		concurrent_sensors = 0;
		/* TEST: use local "leds" and "sensors" for control */
		for ( i = 0; i < SENSORS; i++ ) {
			struct sensor *sensor = semaphore_get_sensor (i);

			/* do something with this information? */
			/* e.g. increase led blinking rate */
			/* test with: echo P_NE > pins.txt multiple times */
			if ( sensor->state_new == ON && sensor->state == OFF ){
				/* button just pressed */
				t_cur_time = get_time();

				if(t_cur_time <= (t_last_sensor_activation + t_treshold_interval)){
					concurrent_sensors++;
				}else{
					t_last_sensor_activation = t_cur_time;
					concurrent_sensors = 1;
				}

				if(concurrent_sensors == 3){
					//Change semaphore mode
					semaphore_mode = (semaphore_mode + 1) % 5;
				}

				SET_BIT ( leds, i );
				freq[i] = (freq[i] + 1) % 10;
				semaphore_set_sensor (i);
			}
			else if ( sensor->state_new == OFF && sensor->state == ON ){
				/* button just released */

				/* test for press duration */
				/* e.g. if long press increase freq by 5 */
				if ( sensor->t + 800 < get_time() ){
					//printf("long press recorded\n");
					sensor->is_long_pressed = 1;
				}

				semaphore_set_sensor (i);
			}
			/* else no change: button still pressed or not */

			/* update sensor state in local data */
			if ( sensor->state == ON )
				SET_BIT ( sensors, i );
			//else
				//CLEAR_BIT ( sensors, i );
		}


		switch(semaphore_mode){
			case 0:
				semaphore_blinking_yellow();
				break;
			case 1:
				semaphore_basic_cycles_update();
				break;
			case 2:
				semaphore_advanced_cycles_update(&sensors, 0);
				break;
			case 3:
				semaphore_advanced_cycles_update(&sensors, ADVANCED_PEDESTRIANS_ON_DEMAND_MODE);
				break;
			case 4:
				semaphore_advanced_cycles_update(&sensors, ADVANCED_VEHICLES_ON_DEMAND_MODE);
				break;
			default:
				semaphore_blinking_yellow();
		}

		semaphore_print();
		delay_T ();
	}

	semaphore_close ();

	return 0 ;
}
