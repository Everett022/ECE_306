/*
 * motor.c
 *
 *  Created on: Feb 8, 2026
 *      Author: everettbrostedt
 * ---------------------------------------------------------------------------------------------------------------------------------------------------
 * Description: This file contains a function that turns off all the motors, turns them on, and then puts them in reverse. This
 * fulfills the simple motor control described in Project 03.
 * ---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#include  "msp430.h"
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"


void no_movement(void){           //reset all the bits back to 0 so no movement
    P6OUT &= ~R_FORWARD;
    P6OUT &= ~L_FORWARD;
    P6OUT &= ~R_REVERSE;
    P6OUT &= ~L_REVERSE;
}

void forward_movement(void){
    P6OUT |= R_FORWARD;
    P6OUT |= L_FORWARD;
}
