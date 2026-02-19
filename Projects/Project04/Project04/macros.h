/*
 * macros.h
 *
 *  Created on: Feb 5, 2026
 *      Author: everettbrostedt
 */

#ifndef MACROS_H_
#define MACROS_H_

#define ALWAYS                  (1)
#define RESET_STATE             (0)
#define RED_LED              (0x01) // RED LED 0
#define GRN_LED              (0x40) // GREEN LED 1
#define TEST_PROBE           (0x01) // 0 TEST PROBE
#define TRUE                 (0x01) //


//Defines that go for the switch
#define OKAY 1
#define NOT_OKAY 0

#define PRESSED 1
#define RELEASED 0

#define DEBOUNCE_TIME 50
#define DEBOUNCE_RESTART 0

// STATES ======================================================================
#define NONE ('N')
#define FIGURE_8 ('F')
#define CIRCLE ('C')
#define TRIANGLE ('T')

#define WAIT ('W')
#define START ('S')
#define RUN ('R')
#define END ('E')

#define WAITING2START (150)

//defines for the two CIRCLE
#define CIRCLE_WHEEL_COUNT_TIME (20)
#define CIRCLE_RIGHT_COUNT_TIME (1)
#define CIRCLE_LEFT_COUNT_TIME (20)
#define CIRCLE_TRAVEL_DISTANCE (60)

//defines for a circle in one direction
#define FIRST_WHEEL_COUNT_TIME (20)
#define FIRST_RIGHT_COUNT_TIME (1)
#define FIRST_LEFT_COUNT_TIME (20)
#define FIRST_TRAVEL_DISTANCE (25)

//defines for a circle in second direction
#define SECOND_WHEEL_COUNT_TIME (20)
#define SECOND_RIGHT_COUNT_TIME (20)
#define SECOND_LEFT_COUNT_TIME (1)
#define SECOND_TRAVEL_DISTANCE (30)
#define THREE ('3')

//defines for the straight leg of the triangle
#define STRAIGHT_WHEEL_COUNT_TIME (20)
#define STRAIGHT_RIGHT_COUNT_TIME (16)
#define STRAIGHT_LEFT_COUNT_TIME (8)
#define STRAIGHT_TRAVEL_DISTANCE (5)

//defines for the 60 degree turn in the triangle
#define TURN_WHEEL_COUNT_TIME (20)
#define TURN_RIGHT_COUNT_TIME (0)
#define TURN_LEFT_COUNT_TIME (20)
#define TURN_TRAVEL_DISTANCE (8)
#define FOUR ('4')

#endif /* MACROS_H_ */
