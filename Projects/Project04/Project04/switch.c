/*
 * switch.c
 *
 *  Created on: Feb 18, 2026
 *      Author: everettbrostedt
 */

//------------------------------------------------------------------------------
// Switch 1 Created for Project 4
//------------------------------------------------------------------------------
#include <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"

extern volatile unsigned char event;
extern volatile unsigned int state;

extern volatile unsigned char okay_to_look_at_switch1;
extern volatile unsigned char sw1_position;
extern volatile unsigned int count_debounce_SW1;
extern volatile unsigned int number_of_presses;

void Switch1_Process(void){
    if (okay_to_look_at_switch1 && (sw1_position == RELEASED)){
      if (!(P4IN & SW1)){
          sw1_position = PRESSED;
          okay_to_look_at_switch1 = NOT_OKAY;
          count_debounce_SW1 = DEBOUNCE_RESTART;
          number_of_presses++;
      }
    }
   if (count_debounce_SW1 <= DEBOUNCE_TIME){
       count_debounce_SW1++;
   }else{
       okay_to_look_at_switch1 = OKAY;
       if (P4IN & SW1){
           sw1_position = RELEASED;
       }
    }
  }

void Switch_mode(void){
    if(number_of_presses){
        number_of_presses = 0;

        if(event == NONE){
            event = CIRCLE;
        }
        else if(event == CIRCLE){
            event = FIGURE_8;
        }
        else if(event == FIGURE_8){
            event = TRIANGLE;
        }
        else{
            event = CIRCLE;
        }
    }
}



