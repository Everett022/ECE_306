/*
 * switch.c
 *
 *  Created on: Feb 18, 2026
 *      Author: everettbrostedt
 */

#include <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "globals.h"
#include  "macros.h"

//Char arrays
char switch_speed_1[] = " 115,200  ";
char switch_speed_2[] = " 460,800  ";

unsigned char okay_to_look_at_switch1;
unsigned int sw1_position = 0;
unsigned int sw2_position = 0;
unsigned int count_debounce_SW1;
unsigned int number_of_presses = 0;

unsigned char okay_to_look_at_switch2;
unsigned int count_debounce_SW2;
unsigned int number_of_presses_2 = 0;

volatile unsigned int switch_1_count = 0;
volatile unsigned int switch_2_count = 0;
unsigned int project_5_flag = 0;
unsigned int line_detection_flag = 0;
unsigned int first = 0;

unsigned int black_value_R = 0;
unsigned int black_value_L = 0;
unsigned int white_value_R = 0;
unsigned int white_value_L = 0;

int speed_flag = 0;

void Switch_mode(void){
    if(switch_1_count){
        switch_1_count = 0;
        sw1_position = 1;
    }
}

void Switch_mode_2(void){
    if(switch_2_count){
        switch_2_count = 0;
        sw2_position = 1;
    }
}

#pragma vector= PORT4_VECTOR
__interrupt void switch1_interrupt(void) {
    if (P4IFG & SW1) {
        P4IE &= ~SW1;
        P4IFG &= ~SW1;
        TB0CCTL1 &= ~CCIFG;
        TB0CCR1 += TB0CCR1_INTERVAL;
        TB0CCTL1 |= CCIE;
        switch_1_count = 1;
 }
}

#pragma vector= PORT2_VECTOR
__interrupt void switch2_interrupt(void) {
    if (P2IFG & SW2) {
        P2IE &= ~SW2;
        P2IFG &= ~SW2;
        TB0CCTL2 &= ~CCIFG;
        TB0CCR2 += TB0CCR2_INTERVAL;
        TB0CCTL2 |= CCIE;
        switch_2_count = 1;
 }
}
