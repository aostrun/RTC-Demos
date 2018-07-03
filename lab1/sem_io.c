/* LAB1 template example */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "sem_io.h"

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
/*           state x   y s_x s_y f.  n. pin */
/* P_NW */ { OFF, 0,  2,  4, 0, 1,  0,  0, 29 },
/* P_NE */ { OFF, 0,  2, 14, 0, 6,  0,  0, 31  },
/* P_EN */ { OFF, 0,  3, 18, 1, 7,  0,  0, 28  },
/* P_ES */ { OFF, 0,  8, 18, 6, 7,  0,  0, 30  },
/* P_SE */ { OFF, 0, 10, 16, 7, 6,  0,  0, 24  },
/* P_SW */ { OFF, 0, 10,  6, 7, 1,  0,  0, 27  },
/* P_WS */ { OFF, 0,  9,  2, 6, 0,  0,  0, 18  },
/* P_WN */ { OFF, 0,  4,  2, 1, 0,  0,  0, 15  },

/* C_NW */ { OFF, 0,  3,  6, 1, 2,  0,  0,  0  },
/* C_NS */ { OFF, 0,  3,  8, 1, 3,  0,  0, 17  },
/* C_NE */ { OFF, 0,  3, 10, 1, 4,  0,  0,  0  },
/* C_EN */ { OFF, 0,  4, 16, 2, 6,  0,  0,  0  },
/* C_EW */ { OFF, 0,  5, 16, 3, 6,  0,  0, 14  },
/* C_ES */ { OFF, 0,  6, 16, 4, 6,  0,  0,  0  },
/* C_SE */ { OFF, 0,  9, 14, 6, 5,  0,  0,  0  },
/* C_SN */ { OFF, 0,  9, 12, 6, 4,  0,  0,  4  },
/* C_SW */ { OFF, 0,  9, 10, 6, 3,  0,  0,  0  },
/* C_WS */ { OFF, 0,  8,  4, 5, 1,  0,  0,  0  },
/* C_WE */ { OFF, 0,  7,  4, 4, 1,  0,  0, 23  },
/* C_WN */ { OFF, 0,  6,  4, 3, 1,  0,  0,  0  }
};



struct led semaphore_get_led ( int id )
{
	return led[id];
}
static void semaphore_update_led ( int id, int state, int freq )
{
	led[id].state = state;
	led[id].freq = freq;
	if ( freq > 0 )
		led[id].next_change = get_time () + 1000/freq;

	#ifdef USE_RPI
	if ( led[id].pin > 0 )
		rpi_output ( led[id].pin, state );
	#endif

	#ifdef USE_RPI_SENSE

		unsigned char color_state;
		if(state == 0){
			color_state = RED_STATE;
		}else if(state == 1){
			color_state = GREEN_STATE;
		}else if(state == YELLOW_STATE){
			color_state = YELLOW_STATE;
		}else{
			color_state = 0;
		}

		sense_set_pixel(led[id].sense_x, led[id].sense_y, color_state);
		sense_update();
	#endif

}
void semaphore_set_led ( int id, int state, int freq )
{
	if ( freq == 0 || freq != led[id].freq || state != led[id].state)
		semaphore_update_led ( id, state, freq );
	/* else: no change: continue blinking */
}
static void update_blinking_leds ()
{
	int i;
	unsigned long now = get_time();
	for ( i = 0; i < LEDS; i++ )
		if(led[i].freq > 0 && led[i].next_change <= now){
			if(led[i].state >= ON){
				led[i].last_state = led[i].state;
				semaphore_update_led (
					i,
					OFF,
					led[i].freq
				);
			}else{
				semaphore_update_led (
					i,
					led[i].last_state,
					led[i].freq
				);
			}
		}
}

/*************************** SENSORS ***************************/
struct sensor sensor[SENSORS] = {
/* P_NW: sensor for semaphore P_NW is on the other side of the road */
/*           state x   y   t   sn. t. pin */
/* P_NW */ { OFF,  2, 15,  0,  0,  0, 25, 0 },
/* P_NE */ { OFF,  2,  3,  0,  0,  0, 11, 0 },
/* P_EN */ { OFF,  9, 18,  0,  0,  0,  7, 0 },
/* P_ES */ { OFF,  2, 18,  0,  0,  0,  8, 0 },
/* P_SE */ { OFF, 10,  5,  0,  0,  0, 22, 0 },
/* P_SW */ { OFF, 10, 17,  0,  0,  0,  3, 0 },
/* P_WS */ { OFF,  3,  2,  0,  0,  0,  9, 0 },
/* P_WN */ { OFF, 10,  2,  0,  0,  0, 10, 0 },

/* C_NW */ { OFF,  2,  6,  0,  0,  0,  0, 0 },
/* C_NS */ { OFF,  2,  8,  0,  0,  0,  0, 0 },
/* C_NE */ { OFF,  2, 10,  0,  0,  0,  0, 0 },
/* C_EN */ { OFF,  4, 17,  0,  0 , 0,  0, 0 },
/* C_EW */ { OFF,  5, 17,  0,  0,  0,  0, 0 },
/* C_ES */ { OFF,  6, 17,  0,  0,  0,  0, 0 },
/* C_SE */ { OFF, 10, 14,  0,  0,  0,  0, 0 },
/* C_SN */ { OFF, 10, 12,  0,  0,  0,  0, 0 },
/* C_SW */ { OFF, 10, 10,  0,  0,  0,  0, 0 },
/* C_WS */ { OFF,  8,  3,  0,  0,  0,  0, 0 },
/* C_WE */ { OFF,  7,  3,  0,  0,  0,  0, 0 },
/* C_WN */ { OFF,  6,  3,  0,  0,  0,  0, 0 }
};


struct vehicle_sensor vehicle_sensor[VEHICLE_SENSORS] = {
	//	forward,		right,		left
		{&sensor[1], &sensor[6], &sensor[3]},
		{&sensor[3], &sensor[0], &sensor[2]},
		{&sensor[5], &sensor[2], &sensor[4]},
		{&sensor[7], &sensor[4], &sensor[6]}
};

struct sensor* semaphore_get_sensor ( int id )
{
	return &sensor[id];
}

void semaphore_set_sensor ( int id )
{
	if(sensor[id].state_new != sensor[id].state) {
		sensor[id].state = sensor[id].state_new;
		sensor[id].t = get_time();
	}
}

/*************************** PIPES ***************************/
/*
 * Sensor simulation using pipe:
 * - echo sensor_name > pins.txt
 * - multiple almost simultaneous press: echo P_EN > pins.txt && sleep 0.1 && echo P_SW > pins.txt && sleep 0.1 && echo C_WS > pins.txt
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

static char *label[] = {
	"P_NW", "P_NE", "P_EN", "P_ES", "P_SE", "P_SW", "P_WS", "P_WN",
	"C_NW", "C_NS", "C_NE", "C_EN", "C_EW", "C_ES",
	"C_SE", "C_SN", "C_SW", "C_WS", "C_WE", "C_WN"
};

static int get_id ( char *id )
{
	int i;
	for ( i = 0; i < SENSORS; i++ )
		if ( !strncmp ( id, label[i], 4 ) )
			return i;
	return -1;
}

/*
 * Check if something new arrived in pipe
 * assuming function is called frequently enough that at most one message
 * will be in pipe
 */
static void pipe_update ()
{
	static unsigned int t_last_check = 0;
	struct timeval timeout;
	fd_set fds;
	int lp, rv, i;
	char buf[7] = {0};
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

	if ( read ( fd, buf, 7 ) > 0 ) {
		lp = buf[0] == '+' ? 1 : 0;
		i = get_id ( &buf[lp] );
		if ( i >= 0 && i < SENSORS ) {
			sensor[i].state_new = ON;
			sensor[i].t_release = now + 100 + lp * 900;
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
		semaphore_set_led ( i, OFF, 0 );
	#ifdef USE_RPI
	rpi_close ();
	#endif
}

/*************************** SEMAPHORE ***************************/
void semaphore_init()
{
	pipe_init ();
	io_init ();

	#ifdef USE_RPI_SENSE
		sense_init();
	#endif

}

void semaphore_update ()
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

void semaphore_close ()
{
	pipe_close ();
	io_close ();
}


/*************************** SEMAPHORE MAP ***************************/
/* semaphore map with all led and sensors in ON state */
#define MAP_SIZE 13
static char sem_map[MAP_SIZE][22] = {
	/* 012345678901234567890 */
	  "+++++ N N N   +++++++", /* 0 */
	  "+++++ W S E   +++++++", /* 1 */
	  "+++*O * * *   O*++*++", /* 2 */
	  "++*++ O O O   ++++O++", /* 3 */
	  "++O++           O* EN", /* 4 */
	  "                O* EW", /* 5 */
	  "WN *O           O* ES", /* 6 */
	  "WE *O                ", /* 7 */
	  "WS *O           ++O++", /* 8 */
	  "++O++++   O O O ++*++", /* 9 */
	  "++*++*O   * * * O*+++", /* 10 */
	  "+++++++   W N E +++++", /* 11 */
	  "+++++++   S S S +++++"  /* 12 */
};

void semaphore_print ()
{
	int i;
	static unsigned int t_last_print = 0;
	unsigned int now;

	now = get_time();
	if ( t_last_print > now + 50 )
		return; /* dont print to frequently */
	t_last_print = now + 50;

	for ( i = 0; i < LEDS; i++ ) {
		if ( led[i].state >= ON )
			sem_map[led[i].x][led[i].y] = 'O';
		else
			sem_map[led[i].x][led[i].y] = '-';
	}
	for ( i = 0; i < SENSORS; i++ ) {
		if ( sensor[i].state == ON )
			sem_map[sensor[i].x][sensor[i].y] = '*';
		else
			sem_map[sensor[i].x][sensor[i].y] = '-';
	}

	printf ( "\033[2J\033[1;1H" ); /* clear screen */
	for ( i = 0; i < MAP_SIZE; i++ )
		printf ( "%s\n", sem_map[i] );
	printf ( "\nt = %7.2lf s\n", now/1000. );
}




/*
 *	Function simulates semaphore in off state
 */
void semaphore_blinking_yellow(){

	for(int i = 0; i < LEDS; i++){
		// frequency = 1/freq => 1/2 = 0.5s
		#ifdef USE_RPI_SENSE
			semaphore_set_led(i, YELLOW_STATE, 2);
		#else
			semaphore_set_led(i, ON, 2);
		#endif
	}

}

void semaphore_set_leds(int led_states, int freq){
	for(int i = 0; i < LEDS; i++){
		if(TEST_BIT(led_states, i)){
			semaphore_set_led(i, ON, freq);
		}else{
			semaphore_set_led(i, OFF, 0);
		}
	}
}

unsigned long t_basic_cycle_last_change = -1;
unsigned long t_basic_cycle_interval = 5000;
unsigned char basic_cycle_last_state = 0;

unsigned char basic_cycle_states[LEDS] = {0,0,1,1,0,0,1,1,1,1,1,0,0,0,1,1,1,0,0,0};
void semaphore_set_basic_cycle(int cycle){

	for(int i=0; i < LEDS; i++){
		if(cycle == 0){
			semaphore_set_led(i, basic_cycle_states[i], 0);
		}else{
			semaphore_set_led(i, 1 - basic_cycle_states[i], 0);
		}
	}
}

/*
 *	Function simulates basic cycles of the semaphore without processing
 *	signals from pedestrians.
 */
void semaphore_basic_cycles_update(){

		if( t_basic_cycle_last_change == -1 || (get_time() - t_basic_cycle_last_change) >= t_basic_cycle_interval ){

			semaphore_set_basic_cycle(basic_cycle_last_state);

			t_basic_cycle_last_change = get_time();
			basic_cycle_last_state = 1 - basic_cycle_last_state;
		}
}


void vehicle_sensors_print(struct vehicle_sensor *sensors){
	for(int i = 0; i < VEHICLE_SENSORS; i++){
		printf(" %d %d %d\n", vehicle_sensor[i].sensor_forward->is_long_pressed, vehicle_sensor[i].sensor_right->is_long_pressed, vehicle_sensor[i].sensor_left->is_long_pressed );
	}
}

int vehicle_sensors_check(struct vehicle_sensor* sensors){

	for(int i=0; i < VEHICLE_SENSORS; i++){
		if(vehicle_sensor[i].sensor_forward->is_long_pressed){
			return 1;
		}
	}
	return 0;
}

int semaphore_vehicle_on_demand_mode(unsigned char direction){
	// Find the next semaphore mode based on the vehicle sensors and current
	// direction and clear sensors that will be served in the next cycle

	//Returns:
	//Straight														0
	//Right																1
	//Left																2
	//No demand													 -1
	//Demand from different direction		 -2
	int tmp = -1;
	if(direction > 1){
		direction = 1;
	}
	for(int i = direction; i < VEHICLE_SENSORS; i+=2){
		if( TEST_STRAIGHT(vehicle_sensor[i]) ){
			if(TEST_LEFT(vehicle_sensor[i])){
				if(tmp == 2){
					//If the oposite vehicle sensor is activated and this is
					//active in the same state as the oposite one, clear this sensor also
					CLEAR_VEHICLE_SENSOR(vehicle_sensor[i]);
				}else if(tmp == -1){
					CLEAR_VEHICLE_SENSOR(vehicle_sensor[i]);
					tmp = 2;
				}
			}
			else if(TEST_RIGHT(vehicle_sensor[i])){
				if(tmp == 1){
					CLEAR_VEHICLE_SENSOR(vehicle_sensor[i]);
				}else if(tmp == -1){
					CLEAR_VEHICLE_SENSOR(vehicle_sensor[i]);
					tmp = 1;
				}
			}
			else{
				CLEAR_VEHICLE_SENSOR(vehicle_sensor[i]);
				tmp = 0;
			}

		}
	}

	if(tmp == -1){
		if( vehicle_sensors_check(vehicle_sensor) ){
			//There are active vehicle sensors but not in this direction
			return -2;
		}
	}

	return tmp;
}

unsigned char advanced_cycle_state = 0;
unsigned long advanced_cycle_led_states = 0;
unsigned long t_advanced_cycle_last_cycle = -1;
unsigned long t_advanced_cycle_interval = 0;

unsigned char semaphore_advanced_cycles_mode = 0; //0 20s, 1 40s
unsigned char semaphore_advanced_cycles_last_mode = 0;

unsigned char t_vehicle_mode_working = 0; //Flag indicating if the semaphore is processing vehicle request
unsigned char semaphore_vehicle_on_demand_led_freq = 0;
char vehicle_demand_direction = 0;

void semaphore_advanced_cycles_update(unsigned long *sensors_data, char advanced_mode){
	if(advanced_mode == ADVANCED_PEDESTRIANS_ON_DEMAND_MODE){
		semaphore_advanced_cycles_last_mode = semaphore_advanced_cycles_mode;
		semaphore_advanced_cycles_mode = (*sensors_data != 0) ? 0 : 1;

		// If the pedestrians activated sensors and if the semaphore is in 40000 interval mode
		// change it to 20000.
		if(semaphore_advanced_cycles_last_mode == 1 && semaphore_advanced_cycles_mode == 0){
			if(t_advanced_cycle_interval == 40000){
				t_advanced_cycle_interval = 20000;
			}
		}

	}

	// Check if enough time has passed for the next state
	if(t_advanced_cycle_last_cycle != -1 && (get_time() - t_advanced_cycle_last_cycle) < t_advanced_cycle_interval && advanced_mode != ADVANCED_VEHICLES_ON_DEMAND_MODE){
		//If not, return
		//printf("Normal return\n");
		return;
	}

	//Time keeping for the vehicle on demand mode
	if(advanced_mode == ADVANCED_VEHICLES_ON_DEMAND_MODE && t_vehicle_mode_working == 1 && (get_time() - t_advanced_cycle_last_cycle) < t_advanced_cycle_interval){
		//printf("Vehicle return\n");
		return;
	}


	int vehicle_mode_direction = -2;

	if(advanced_mode == ADVANCED_VEHICLES_ON_DEMAND_MODE){
		t_vehicle_mode_working = 0;
		//vehicle_sensors_print(vehicle_sensor);

		vehicle_mode_direction = semaphore_vehicle_on_demand_mode( vehicle_demand_direction );
		//printf("Vehicle mode direction: %d\n", vehicle_mode_direction);
		t_vehicle_mode_working = 1;
		switch(vehicle_mode_direction){
			case 0:
				//Straight
				advanced_cycle_state = 1 + vehicle_demand_direction * 3;
				//semaphore_vehicle_on_demand_led_freq = 200;
				break;
			case 2:
				//Left
				advanced_cycle_state = 2 + vehicle_demand_direction * 3;
				//semaphore_vehicle_on_demand_led_freq = 500;
				break;
			case 1:
				//Right
				advanced_cycle_state = 1 + vehicle_demand_direction * 3;
				//semaphore_vehicle_on_demand_led_freq = 300;
				break;
			case -2:
				//Switch direction
				advanced_cycle_state = 0;
				vehicle_demand_direction = vehicle_demand_direction ? 0 : 1;
				break;
			default:
				advanced_cycle_state = 0;
				semaphore_vehicle_on_demand_led_freq = 0;
				t_vehicle_mode_working = 0;
		}


	}


	advanced_cycle_led_states = 0; //Clear all previous LED states
	switch(advanced_cycle_state){
		case 0:
					//All off
					t_advanced_cycle_interval = 3000;				//3s
					break;
		case 1:
					//Cars: NW, NS, SN, SE 20s
					SET_BIT(advanced_cycle_led_states, 8);	//NW
					SET_BIT(advanced_cycle_led_states, 9);	//NS
					SET_BIT(advanced_cycle_led_states, 15);	//SN
					SET_BIT(advanced_cycle_led_states, 14);	//SE

					//Pedestrians:
					if(semaphore_advanced_cycles_mode){
						t_advanced_cycle_interval = 40000;		//40s
					}else{
						SET_BIT(advanced_cycle_led_states, 2); //EN
						SET_BIT(advanced_cycle_led_states, 3); //ES
						SET_BIT(advanced_cycle_led_states, 6); //WS
						SET_BIT(advanced_cycle_led_states, 7); //WN

						CLEAR_BIT(*sensors_data, 2);
						CLEAR_BIT(*sensors_data, 3);
						CLEAR_BIT(*sensors_data, 6);
						CLEAR_BIT(*sensors_data, 7);

						t_advanced_cycle_interval = 20000;			//20s
					}

					break;
		case 2:
					//NE, SW 10s
					SET_BIT(advanced_cycle_led_states, 10);	//NE
					SET_BIT(advanced_cycle_led_states, 16);	//SW
					t_advanced_cycle_interval = 10000;			//10s
					break;
		case 3:
					//All off
					t_advanced_cycle_interval = 3000;				//3s
					break;
		case 4:
					//WS, WE, EN, EW 20s
					SET_BIT(advanced_cycle_led_states, 17);	//WS
					SET_BIT(advanced_cycle_led_states, 18);	//WE
					SET_BIT(advanced_cycle_led_states, 11);	//EN
					SET_BIT(advanced_cycle_led_states, 12);	//EW

					//Pedestrians:
					if(semaphore_advanced_cycles_mode){
						t_advanced_cycle_interval = 40000;		//40s
					}else{
						SET_BIT(advanced_cycle_led_states, 0); //NW
						SET_BIT(advanced_cycle_led_states, 1); //NE
						SET_BIT(advanced_cycle_led_states, 4); //SE
						SET_BIT(advanced_cycle_led_states, 5); //SW

						CLEAR_BIT(*sensors_data, 0);
						CLEAR_BIT(*sensors_data, 1);
						CLEAR_BIT(*sensors_data, 4);
						CLEAR_BIT(*sensors_data, 5);

						t_advanced_cycle_interval = 20000;			//20s
					}
					break;
		case 5:
					//WN, ES 10s
					SET_BIT(advanced_cycle_led_states, 19);	//WN
					SET_BIT(advanced_cycle_led_states, 13);	//ES
					t_advanced_cycle_interval = 10000;			//10s
					break;
		default:
					semaphore_blinking_yellow();
	}

	if(t_vehicle_mode_working == 0){
		t_advanced_cycle_interval = 10;
	}

	//printf("Cycle state: %d \n", advanced_cycle_state);
	//Cycle trough states
	if(advanced_mode != ADVANCED_VEHICLES_ON_DEMAND_MODE){
		advanced_cycle_state = (advanced_cycle_state + 1) % 6;
	}

	if(advanced_mode == ADVANCED_VEHICLES_ON_DEMAND_MODE){
		#ifdef USE_RPI_SENSE
			semaphore_set_leds(advanced_cycle_led_states, 0);
		#else
			semaphore_set_leds(advanced_cycle_led_states, semaphore_vehicle_on_demand_led_freq);
		#endif
	}else{
		semaphore_set_leds(advanced_cycle_led_states, 0);
	}
	t_advanced_cycle_last_cycle = get_time();
}
