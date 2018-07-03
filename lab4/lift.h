#pragma once

#include <stdint.h>

#define NUM_LIFT 2
#define NUM_FLOOR 5
/*
#define DIR_UP		1
#define DIR_DOWN	2
#define DIR_NONE	3


#define DOOR_OPEN 		1
#define DOOR_CLOSED		2
#define DOOR_OPENING	3
#define DOOR_CLOSING	4
*/

#define LIFT_SPEED_DISTANCE 0.1
#define LIFT_SPEED_TIME 200

#define LIFT_FLOOR_ERROR_EPS 0.1

/*
#define LIFT_MOVING 	10
#define LIFT_STOPPED 	20
#define LIFT_STOPPING 30
#define LIFT_HALTED 	40
*/

#define T_LIFT_SENSOR_TRESHOLD 500

#define T_LIFT_STOPPING 500
#define T_LIFT_STARTING 500

#define T_DOOR_OPENING 1000
#define T_DOOR_CLOSING 1000
#define T_HOLD_DOOR 1000



/* lift state */
struct lift {
	float floor;   // current floor (0 - MAX_FLOOR; real number, e.g. 2.35)
	int8_t dir;    // direction (UP, DOWN, NONE)
	int8_t door;   //door state (OPEN, CLOSED, OPENING, CLOSING)
	int8_t state;  // MOVING, STOPPED, STOPPING, HALTED
	//int8_t stops[NUM_FLOOR];
	unsigned long t_next_change;
};

/* buttons (pressed or not) */
struct buttons {
	int8_t in[NUM_LIFT][NUM_FLOOR]; // for each lift NUM_FLOOR buttons
	int8_t open, stop, cont;
	int8_t out[NUM_FLOOR][2]; // for each floor: call for up and down
};

struct floor_stat {
	int8_t floor_id;
	int8_t calls;
};

enum { DIR_NONE = -1, DIR_DOWN = 0, DIR_UP = 1 };
enum { DOOR_OPEN = 0, DOOR_CLOSED=1, DOOR_OPENING=2, DOOR_CLOSING=3 };
enum { LIFT_STARTING = 9, LIFT_MOVING = 11, LIFT_STOPPED = 12, LIFT_STOPPING = 13, LIFT_HALTED = 14 };

#define FLOOR(X) ( (int) (X) )       /* only for X >= 0 */
#define ROUND(X) ( (int) (X + 0.5) ) /* only for X >= 0 */

#define RELEASE_TIMEOUT		1000 /* end of sequence interval */
#define SHORT_PRESS_TIMEOUT	500  /* end of sequence interval */

void update_floor_stats(int floor_id);

void lift_print ( char *title, struct lift *lift, struct buttons *buttons );
void lift_process ( struct lift *lift, struct buttons *buttons );
void lift_set_operation (struct lift *lift, int operation);
