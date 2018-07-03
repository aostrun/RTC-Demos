/* LAB1 template usage example */
#include "lift.h"
#include "lift_io.h"
#include "debug_print.h"

#include <stdlib.h>
#include <math.h>

void lift_process_states(struct lift *lift, struct buttons *buttons);



// Iterate trough floor sensors and check if one
// of them is active
// returns 1 if there are active floor sensors
// otherwise returns 0
int check_floor_sensors(uint8_t *sensor_data){
  struct sensor floor_sen;
  uint8_t isActive = 0;
  for(int i=0; i < NUM_FLOOR; i++){
    floor_sen = lift_io__get_floor_button(i);
    sensor_data[i] = floor_sen.state;
    if(floor_sen.state == ON){
      isActive = 1;
    }
  }
  return isActive;
}

// Returns the id of the closest lift to the provided floor
int closest_lift_to_request(struct lift *lift, uint8_t floor_id){

  int lift_distance = 4*NUM_FLOOR;
  int lift_id = -1;
  for(int i=0; i < NUM_LIFT; i++){
    if(lift[i].floor == floor_id){
      return i;
    }else if( (lift[i].dir == DIR_UP && lift[i].floor <= floor_id) ||
              (lift[i].dir == DIR_DOWN && lift[i].floor >= floor_id) ){
      // Request is in the path of the lift
      if(fabs(lift[i].floor - floor_id) < lift_distance){
        lift_distance = fabs(lift[i].floor - floor_id);
        lift_id = i;
      }
    }else if( (lift[i].dir == DIR_UP && lift[i].floor > floor_id) ||
              (lift[i].dir == DIR_DOWN && lift[i].floor < floor_id)){
      // Request is not in the path of the lift
      continue;
    }
  }
  return lift_id;

}

int empty_internal(int lift_id, struct buttons *buttons){
  for (int i = 0; i < NUM_FLOOR; i++){
    if(buttons->in[lift_id][i] > 0){
      return -1;
    }
  }
  return 0;
}

void lift_process ( struct lift *lift, struct buttons *buttons )
{
	/* process inputs, state ... */
	/* simulate lift behavior - movement, door, ... */

  uint8_t floor_sensor_data[NUM_FLOOR];
  int result;

  unsigned long now = get_time();


  //Check floor sensors
  //result = check_floor_sensors(floor_sensor_data);

  for(int k = 0; k < NUM_FLOOR; k++){
    // Iterate over all floor sensors
    if(buttons->out[k][0] > 0 || buttons->out[k][1] > 0){
      // Request came from outside
      //printdbg("Processing external request...\n");
      // Initial values, NUM_FLOOR*4 should be
      // more than maximum possible distance
      uint8_t min_idx = -1;
      uint8_t min_distance = NUM_FLOOR * 4;
      // Calculate closest lift that can serve this request
      for(int i = 0; i < NUM_LIFT; i++){
        if(lift[i].state == LIFT_STOPPED && lift[i].door == DOOR_CLOSED){
          // lift is inactive
          if(fabs(lift[i].floor - k) < min_distance){
            min_distance = fabs(lift[i].floor - k);
            min_idx = i;
          }
        }else if(lift[i].dir != DIR_NONE && buttons->out[k][lift[i].dir]){
          // if request direction corresponds with the lift direction
          if(fabs(lift[i].floor - k) < min_distance){
            min_distance = fabs(lift[i].floor - k);
            min_idx = i;
          }
        }else if(lift[i].dir != DIR_NONE
              && buttons->out[k][1 - lift[i].dir]
              && empty_internal(i, buttons) == 0)
          {
            if(fabs(lift[i].floor - k) < min_distance){
              min_distance = fabs(lift[i].floor - k);
              min_idx = i;
            }
          }
      }
      //printdbg("Lift %d should serve request from floor %d\n", min_idx, k);
      if(min_idx > -1){
        if(fabs(lift[min_idx].floor - k) < LIFT_FLOOR_ERROR_EPS){
          // Lift is currently at the floor that request came from
          if(lift[min_idx].state != LIFT_STOPPED && lift[min_idx].state != LIFT_STOPPING){
            printdbg("Lift %d serving request %d\n", min_idx, k);
            lift[min_idx].floor = (float) k;
            lift[min_idx].dir = buttons->out[k][DIR_DOWN] > 0 ? DIR_DOWN : DIR_UP;
            lift_set_operation(&lift[min_idx], LIFT_STOPPING);
          }else if(lift[min_idx].state == LIFT_STOPPED && lift[min_idx].door != DOOR_OPEN && lift[min_idx].door != DOOR_OPENING){
            lift[min_idx].dir = buttons->out[k][DIR_DOWN] > 0 ? DIR_DOWN : DIR_UP;
            lift_set_operation(&lift[min_idx], DOOR_OPENING);
          }

        }else if(lift[min_idx].state == LIFT_STOPPED && lift[min_idx].door == DOOR_CLOSED){
          // Lift is not at the floor that request came from, send it there
          if(lift[min_idx].state != LIFT_STARTING && lift[min_idx].state != LIFT_MOVING){
            printdbg("Lift %d moving towars external request %d\n", min_idx, k);
            lift[min_idx].dir = lift[min_idx].floor > k ? DIR_DOWN : DIR_UP;
            lift_set_operation(&lift[min_idx], LIFT_STARTING);
          }

        }
      }

    }

  }

  lift_process_states(lift, buttons);

}

// Check if there are unserved requests that came from
// the lift itself
int lift_check_orders(int lift_id, struct buttons *buttons){
  for(int i=0; i < NUM_FLOOR; i++){
    if(buttons->in[lift_id][i] > 0){
      return 1;
    }
  }

  return 0;
}

void lift_process_states(struct lift *lift, struct buttons *buttons){

  unsigned long now = get_time();
  for(int i=0; i < NUM_LIFT; i++){

    if(lift[i].t_next_change > now){
      continue;
    }

    //printdbg("Processing %d lift...\n", i);
    switch(lift[i].state){
      case LIFT_MOVING:

      // check if lift has to stop on the floor due to internal
      // request
      if(buttons->in[i][FLOOR(lift[i].floor)] > 0){
        // Check if lift is positioned correctly
        float distanceToFloor = lift[i].floor - FLOOR(lift[i].floor);
        if(fabs(distanceToFloor) <= LIFT_FLOOR_ERROR_EPS){
              lift_set_operation(&lift[i], LIFT_STOPPING);
              lift[i].floor = FLOOR(lift[i].floor);
              // don't continue moving the lift, break the LIFT_MOVING
              // process
              break;
            }

      }

        // move lift in the direction dir
        lift[i].floor += lift[i].dir == DIR_UP ? LIFT_SPEED_DISTANCE : -LIFT_SPEED_DISTANCE;
        lift[i].t_next_change = now + LIFT_SPEED_TIME;
        if(lift[i].floor <= 0.0 || lift[i].floor >= NUM_FLOOR-1){
          lift_set_operation(&lift[i], LIFT_STOPPING);
          if(lift[i].floor <= 0.0){
            lift[i].floor = 0.0;
          }else{
            lift[i].floor = (float) NUM_FLOOR-1;
          }
        }

        //printdbg("Moved lift %d: %f", i, lift[i].floor);
        break;
      case LIFT_STARTING:
        // Lift started, change its state to MOVING
        lift[i].state = LIFT_MOVING;
        break;
      case LIFT_STOPPING:
        // stop the lift
        lift[i].state = LIFT_STOPPED;
        // check if door should open
        if( buttons->in[i][FLOOR(lift[i].floor)] > 0 ||
            buttons->out[FLOOR(lift[i].floor)][lift[i].dir]){
          printdbg("Opening door %d\n", i);
          lift_set_operation(&lift[i], DOOR_OPENING);
        }
        break;
      case LIFT_STOPPED:
        // check for lift doors
        switch(lift[i].door){
          case DOOR_OPENING:
            lift[i].door = DOOR_OPEN;
            buttons->in[i][ROUND(lift[i].floor)] = 0;
            buttons->out[ROUND(lift[i].floor)][lift[i].dir] = 0;
            lift[i].t_next_change = now + T_HOLD_DOOR;
            break;
          case DOOR_CLOSING:
            lift[i].door = DOOR_CLOSED;
            break;
          case DOOR_OPEN:
            // timeout for the opened doors reached
            lift_set_operation(&lift[i], DOOR_CLOSING);
            break;
          case DOOR_CLOSED:
            // Lift doors closed, process internal floor request
            // in the current direction or check if opposite direction
            // has unprocessed requests. If there are no internal requests
            // wait.
            if(lift[i].dir != DIR_NONE){
              //struct sensor internal_sensor = lift_io__get_lift_button(i);
              uint8_t hasRequest = 0;
              for(int k=FLOOR(lift[i].floor); (k >= 0 && k < NUM_FLOOR); lift[i].dir == DIR_UP ? k++ : k-- ){
                if(buttons->in[i][k] > 0){
                  // Start moving lift in the direction that it moved
                  // before stopping
                  hasRequest = 1;
                  lift_set_operation(&lift[i], LIFT_STARTING);
                }
              }
              if(hasRequest == 0){
                // Check opposite direction
                for(int k=FLOOR(lift[i].floor); (k >= 0 && k < NUM_FLOOR); lift[i].dir == DIR_UP ? k-- : k++ ){
                  if(buttons->in[i][k] > 0){
                    // Start moving lift in the direction that it moved
                    // before stopping
                    hasRequest = 1;
                    // Switch lift direction
                    lift[i].dir = lift[i].dir == DIR_UP ? DIR_DOWN : DIR_UP;
                    lift_set_operation(&lift[i], LIFT_STARTING);
                  }
                }
              }

            }else{
              // if(lift[i].dir == NONE) part
              for(int k=0; k < NUM_FLOOR; k++ ){
                if(buttons->in[i][k] > 0){
                  // Switch lift direction
                  lift[i].dir = FLOOR(lift[i].floor) > k ? DIR_DOWN : DIR_UP;
                  lift_set_operation(&lift[i], LIFT_STARTING);
                }
              }
            }

            break;
        }
        break;
    }

  }


}


void lift_set_operation(struct lift *lift, int operation){
  unsigned long now = get_time();
  switch(operation){
    case LIFT_STARTING:
      lift->t_next_change = now + T_LIFT_STARTING;
      lift->state = LIFT_STARTING;
      break;
    case LIFT_STOPPING:
      lift->t_next_change = now + T_LIFT_STOPPING;
      lift->state = LIFT_STOPPING;
      break;
    case DOOR_OPENING:
      if(lift->state == LIFT_STOPPED && lift->door == DOOR_CLOSED){
        // Start opening the door only when lift is STOPPED and
        // when lift doors CLOSED
        lift->t_next_change = now + T_DOOR_OPENING;
        lift->door = DOOR_OPENING;
      }
      break;
    case DOOR_CLOSING:
      if(lift->state == LIFT_STOPPED && lift->door == DOOR_OPEN){
        // Door can start closing only when lift is STOPPED and
        // when lift doors are OPEN
        lift->t_next_change = now + T_DOOR_CLOSING;
        lift->door = DOOR_CLOSING;
      }
      break;
  }
}
