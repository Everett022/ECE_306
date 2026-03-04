/*
 * adc.c
 *
 *  Created on: Mar 1, 2026
 *      Author: everettbrostedt
 */

#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "macros.h"
#include  "globals.h"
#include <stdio.h>

char thousands = '0';
char hundreds = '0';
char tens = '0';
char ones = '0';

unsigned int value_passed;
unsigned int movement = WAIT;
unsigned int color_detected = 0;
unsigned int start_flag = 0;

void BCD_Convert(unsigned int value_passed){

    thousands = '0';
    hundreds  = '0';
    tens      = '0';
    ones      = '0';

    if(value_passed >= 1000){
         value_passed -= 1000;
         thousands = '1';
    }
    while(value_passed >= 100){
        value_passed -= 100;
        hundreds++;
    }
    while(value_passed >= 10){
        value_passed -= 10;
        tens++;
    }
    while(value_passed >= 1){
        value_passed -= 1;
        ones++;
    }
}

void detect(void){
    if(condition){
        sprintf(display_line[0], "State: ON ");
        display_changed = 1;
    }else{
        sprintf(display_line[0], "State: OFF");
        display_changed = 1;
    }

    if(left_flag){
        left_flag = 0;
        BCD_Convert (ADC_Left_Detect);
        sprintf(display_line[2], "L: %c%c%c%c   ",thousands, hundreds, tens, ones);
        display_changed = 1;
    }
    if(right_flag){
        right_flag = 0;
        BCD_Convert (ADC_Right_Detect);
        sprintf(display_line[3], "R: %c%c%c%c   ",thousands, hundreds, tens, ones);
        display_changed = 1;
    }
    if(thumb_flag){
        thumb_flag = 0;
        BCD_Convert (ADC_Thumb);
        sprintf(display_line[1], "T: %c%c%c%c   ",thousands, hundreds, tens, ones);
        display_changed = 1;
    }
    Display_Process();
}

void line_detection(void){
    if (first){
        first = 0;
        delay(3);
    }//630 and 550
    if((ADC_Left_Detect < 630) || (ADC_Right_Detect < 550)){
        forward_movement();
    }else{
        no_movement();
        reverse_movement();
        small_delay(2);
        no_movement();
        delay(3);
        turn_right();
        small_delay(3);
        no_movement();
        delay(10);
    }
}

void turn_line(void){
    if((ADC_Left_Detect < 630) || (ADC_Right_Detect < 550)){
            turn_right();
        }else{
            no_movement();
            small_delay(2);
        }
}

/*int color (void){
    color_detected = 1;         //start as black
    if((ADC_Left_Detect < 680) || (ADC_Right_Detect < 630)){
        color_detected = 0;
    }else{
        color_detected = 1;
    }
    return color_detected;
}
*/

//shitty broken line detection code
/*
 * delay(3);

       color();

       switch(movement){
           case WAIT:
               no_movement();
               if(color_detected && !start_flag){
                   movement = WAIT;
               }
               else if(!start_flag){
                   movement = START;
               }
               else{
                   no_movement();
               }
               break;
           case START:
               if(!color_detected){
                   start_flag = 1;
                   forward_movement();
               }
               else{
                   no_movement();
                   reverse_movement();
                   small_delay(5);
                   delay(4);
                   movement = TURN_I;
               }
               break;
           case TURN_I:
               if(!color_detected){
                   turn_right();
               }
               else{
                   movement = WAIT;
               }
               break;
           default:
               movement = WAIT;
               break;
       }
       delay(4);
*/
