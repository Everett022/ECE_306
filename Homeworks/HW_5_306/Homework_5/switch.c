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
extern volatile int mode;

extern volatile unsigned char okay_to_look_at_switch1;
extern volatile unsigned char sw1_position;
extern volatile unsigned int count_debounce_SW1;
extern volatile unsigned int number_of_presses;

extern volatile unsigned char okay_to_look_at_switch2;
extern volatile unsigned char sw2_position;
extern volatile unsigned int count_debounce_SW2;
extern volatile unsigned int number_of_presses_2;


//Switch 1 is used for changing shape mode
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

//Switch 2 is used for changing the pin 3.4 mode from GPIO to CLK
void Switch2_Process(void){
    if (okay_to_look_at_switch2 && (sw2_position == RELEASED)){
      if (!(P2IN & SW2)){
          sw2_position = PRESSED;
          okay_to_look_at_switch2 = NOT_OKAY;
          count_debounce_SW2 = DEBOUNCE_RESTART;
          number_of_presses_2++;
      }
    }
   if (count_debounce_SW2 <= DEBOUNCE_TIME){
       count_debounce_SW2++;
   }else{
       okay_to_look_at_switch2 = OKAY;
       if (P2IN & SW2){
           sw2_position = RELEASED;
       }
    }
  }

void Switch_mode_2(void){
    if(number_of_presses_2 > 0){
        number_of_presses_2 = 0;

        if(mode == USE_GPIO){
            mode = USE_SMCLK;
            P1OUT &= ~RED_LED;
        }
        else{
            mode = USE_GPIO;

            P1OUT |= RED_LED;
        }
        Init_Port_3();

    }
}






