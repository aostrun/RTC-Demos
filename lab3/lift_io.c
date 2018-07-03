/* io interface */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
#include "lift_io.h"
#include "lift.h"
//#include "rpi_sense.h"

#define T	10 /* ms, update interval */

/*************************** UTILITY FUNCTIONS ***************************/
/* get time in milliseconds; 0 = program start */
unsigned long get_time ()
{
	static unsigned long t0 = -1;
	unsigned long t;
	struct timespec tp;

	clock_gettime ( CLOCK_MONOTONIC, &tp );
	t = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

	if ( t0 == -1 )
		t0 = t;

	return t - t0;
}
void delay_ms ( unsigned long ms )
{
	struct timespec request;
	request.tv_sec = ms / 1000;
	request.tv_nsec = ( ms % 1000 ) * 1000000;
	clock_nanosleep ( CLOCK_MONOTONIC, 0, &request, NULL );
}
void delay_T ()
{
	delay_ms (T);
}

/*************************** LEDS ***************************/
static struct led led[LEDS] = {
	{28,0,0,0, 	{5, 2, GREEN}},
	{30,0,0,0, 	{4, 2, GREEN}},
	{2,0,0,0,  	{3, 2, GREEN}},
	{3,0,0,0, 	{2, 2, GREEN}},
	{4,0,0,0, 	{1, 2, GREEN}},
	{14,0,0,0, 	{0, 2, GREEN}},
	{15,0,0,0,	{0, 2, GREEN}},
	{8,0,0,0, 	{5, 4, GREEN}},
	{11,0,0,0,	{4, 4, GREEN}},
	{25,0,0,0, 	{3, 4, GREEN}},
	{9,0,0,0, 	{2, 4, GREEN}},
	{29,0,0,0, 	{1, 4, GREEN}},
	{24,0,0,0,	{0, 4, GREEN}},
	{10,0,0,0,	{0, 4, GREEN}}
};

struct led lift_io_get_led ( int id )
{
	return led[id];
}
static void lift_io_update_led ( int id, int state, int freq )
{
	led[id].state = state;
	led[id].freq = freq;
	if ( freq > 0 )
		led[id].next_change = get_time () + 1000/freq;

	#if USE_RPI == 1
	if ( led[id].pin > 0 )
		rpi_output ( led[id].pin, state );
	#endif

	#ifdef USE_RPI_SENSE

		sense_set_pixel(led[id].sense.y, led[id].sense.x, led[id].state );
		if(led[id].sense.y > 3){
			sense_set_pixel(led[id].sense.y + 1, led[id].sense.x, led[id].state );
		}else{
			sense_set_pixel(led[id].sense.y - 1, led[id].sense.x, led[id].state );
		}
	#endif

}
static void lift_io_set_led ( int id, int state, int freq )
{
	if ( freq == 0 || freq != led[id].freq ){
		lift_io_update_led ( id, state, freq );
	}

	/* else: no change: continue blinking */
}
static void update_blinking_leds ()
{
	int i;
	unsigned long now = get_time();
	for ( i = 0; i < LEDS; i++ )
		if ( led[i].freq > 0 && led[i].next_change <= now ){
			#ifdef USE_RPI_SENSE
				lift_io_update_led (
					i,
					led[i].state >= ON ? OFF : led[i].sense.color,
					led[i].freq
				);
			#else
			lift_io_update_led (
				i,
				led[i].state >= ON ? OFF : ON,
				led[i].freq
			);
			#endif
		}

}

/*************************** SENSORS ***************************/
static struct sensor sensor[SENSORS] = {
	{18,0,0,0,0},{17,0,0,0,0},{27,0,0,0,0},{22,0,0,0,0},{23,0,0,0,0},
	{31,0,0,0,0},{7,0,0,0,0}
};

static struct sensor lift_io_get_sensor ( int id )
{
	return sensor[id];
}

static void lift_io_set_sensor ( int id )
{
	if ( sensor[id].state_new != sensor[id].state ) {
		sensor[id].state = sensor[id].state_new;
		sensor[id].t = get_time();
	}
}

/*************************** PIPES ***************************/
/*
 * Simulate button press with the commands:
 * - short press: echo pin_number > pins.txt
 * - long press (1s): echo +pin_number > pins.txt
 * (Pin numbers based on lift board pin-out)
 * Don't create pipe pins.txt outside from this program.
 * Simulation is simple, not resistant to wrong inputs.
*/
static int fd = -1;
#define PIPE_NAME  "pins.txt"

static void pipe_init()
{
	if ( fd == -1 )
		fd = open ( PIPE_NAME, O_RDONLY|O_NONBLOCK );
	if ( fd == -1 ) {
		mkfifo ( PIPE_NAME, 00600 );
		fd = open ( PIPE_NAME, O_RDONLY|O_NONBLOCK );
		if ( fd == -1 ) {
			printf ( "Error creating pipe!\n" );
			exit(1);
		}
	}
}

/*
 * Check if something new arrived in pipe
 * assuming function is called frequently enough that at most one
 * message will be in pipe
 */
static void pipe_update ()
{
	static unsigned int t_last_check = 0;
	struct timeval timeout;
	fd_set fds;
	int lp, rv, i, pin;
	char buf[5];
	unsigned int now = get_time();

	if ( t_last_check > now + 10 )
		return;
	t_last_check = now + 10;

	FD_ZERO ( &fds );
	FD_SET ( fd, &fds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	rv = select ( fd + 1, &fds, NULL, NULL, &timeout );
	if ( rv < 0 ) {
		perror ( "select\n" );
		exit (1);
	}

	if ( rv == 0 )
		return; /* empty */

	if ( read ( fd, buf, 3 ) > 0 ) {
		lp = buf[0] == '+' ? 1 : 0;
		sscanf ( &buf[lp], "%d", &pin );
		for ( i = 0; i < SENSORS; i++ )
			if ( sensor[i].pin == pin ) {
				sensor[i].state_new = ON;
				sensor[i].t_release = now + 100 + lp * 900;
				break;
			}
	}
}

static void pipe_close ()
{
	if ( fd >= 0 ) {
		close(fd);
		unlink (PIPE_NAME);
	}
}

/*************************** IO (RPI) ***************************/
static void io_init()
{
	#ifdef USE_RPI
	int leds[LEDS];
	int sensors[SENSORS];
	int i, n_leds, n_sensors;

	for ( i = 0, n_leds = 0; i < LEDS; i++ ) {
		if ( led[i].pin > 0 ) {
			leds[n_leds] = led[i].pin;
			n_leds++;
		}
	}

	for ( i = 0, n_sensors = 0; i < SENSORS; i++ ) {
		if ( sensor[i].pin > 0 ) {
			sensors[n_sensors] = sensor[i].pin;
			n_sensors++;
		}
	}
	rpi_init ( n_leds, leds, n_sensors, sensors );
	#endif

	#ifdef USE_RPI_SENSE
		sense_init();
	#endif

}

static void io_update()
{
	#ifdef USE_RPI
	int i;
	for ( i = 0; i < SENSORS; i++ )
		if ( sensor[i].pin > 0 )
			sensor[i].state_new = rpi_input (sensor[i].pin);
	#endif



}

static void io_close ()
{
	int i;

	for ( i = 0; i < LEDS; i++ )
		lift_io_set_led ( i, OFF, 0 );
	#if USE_RPI == 1
	rpi_close ();
	#endif
}

/*************************** LIFT ***************************/
void lift_io_init()
{
	pipe_init ();
	io_init ();
}

void lift_io_update ()
{
	int i;
	unsigned int now;

	pipe_update ();
	io_update ();

	now = get_time();
	for ( i = 0; i < SENSORS; i++ ) {
		/* simulate sensor release when pipe is used */
		if ( sensor[i].state == ON && sensor[i].state_new == ON &&
			sensor[i].t_release > sensor[i].t &&
			sensor[i].t_release <= now
		)
			sensor[i].state_new = OFF;
	}

	update_blinking_leds ();
}

void lift_io_close ()
{
	pipe_close ();
	io_close ();
}

/* get button that simulates presses inside lift "i" */
struct sensor lift_io__get_lift_button ( int lift_id )
{
	return lift_io_get_sensor ( lift_id + 5 );
}
void lift_io__set_lift_button ( int lift_id )
{
	lift_io_set_sensor ( lift_id + 5 );
}

/* get button that simulates presses on floor "j" */
struct sensor lift_io__get_floor_button ( int floor_id )
{
	return lift_io_get_sensor ( floor_id );
}
void lift_io__set_floor_button ( int floor_id )
{
	lift_io_set_sensor (floor_id);
}

/* set leds to a state appropriate to with "lift/buttons" */
void lift_io__set_leds ( struct lift *lift, struct buttons *buttons )
{
	int i, j, k, dir_up, dir_down, state[NUM_FLOOR], freq[NUM_FLOOR];

	/*
	 * in simulation board there are leds for direction and position
	 * only buttons isn't used
	 */

	for ( i = 0; i < NUM_LIFT; i++ ) {
		/* direction UP/DOWN/NONE */
		dir_up = dir_down = OFF;
		if ( lift[i].dir == DIR_UP )
			dir_up = ON;
		else if ( lift[i].dir == DIR_DOWN )
			dir_down = ON;

		#ifdef USE_RPI_SENSE
			lift_io_set_led ( i*7 + 5, dir_up == ON ? RED : GREEN, 0 );
		#else
			lift_io_set_led ( i*7 + 5, dir_up, 0 );
			lift_io_set_led ( i*7 + 6, dir_down, 0 );
		#endif


		/* lift position and door state */
		for ( j = 0; j < NUM_FLOOR; j++ )
			state[j] = freq[j] = 0;

		j = ROUND (lift[i].floor);
		switch ( lift[i].door ) {
			case DOOR_OPEN: /* lift is stopped and door is open */
				state[j] = BLUE;
				freq[j] = 5;
				break;
			case DOOR_OPENING: /* lift is stopped and door is opening */
				state[j] = GOLD;
				freq[j] = 10;
				break;
			case DOOR_CLOSING: /* lift is stopped and door is closing */
				state[j] = RED;
				freq[j] = 10;
				break;
			case DOOR_CLOSED: /* door is closed; is lift moving? */
			default:
				if ( lift[i].state == LIFT_STOPPED ) {
					state[j] = GREEN;
					freq[j] = 0;
				}
				else {
					/* moving or halted */
					j = FLOOR(lift[i].floor);
					k = FLOOR(lift[i].floor + 0.99);
					state[j] = state[k] = MAGENTA;
					freq[j] = freq[k] = 20;
				}
		}
		for ( j = 0; j < NUM_FLOOR; j++ ){
			led[i*7 + j].sense.color = state[j];
			lift_io_set_led ( i*7 + j, state[j], freq[j] );
		}

	}
}
