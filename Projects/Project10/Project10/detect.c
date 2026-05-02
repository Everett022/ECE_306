/*
 * adc.c
 *
 *  Created on: Mar 1, 2026
 *      Author: everettbrostedt
 */

#include  <stdio.h>
#include  <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "globals.h"
#include  "macros.h"

//BCD Conversion variables
char thousands = '0';
char hundreds = '0';
char tens = '0';
char ones = '0';

int delay_amount = 0;
int small_delay_amount = 0;
int circle_time = 0;

unsigned int intersect_flag = 0;
unsigned int full_turn_flag = 0;
unsigned int left_turn_flag = 0;
unsigned int last_dir = 1;

unsigned int segment = WAIT_CASE;
unsigned int saved_case = FIND_LINE;

volatile int left_forward = SLOW;
volatile int right_forward = SLOW;

//PID Control Variables
int prev_error = 0;
int p_error = 0;
int i_error = 0;

//Code that takes in hex values for IR detectors and converts to BCD
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

//Display the IR ADC readings for the initial check
void detect(void){
    if(left_flag){
        left_flag = 0;
        BCD_Convert (ADC_Left_Detect);
        display_line[3][0] = 'L';
        display_line[3][1] = thousands;
        display_line[3][2] = hundreds;
        display_line[3][3] = tens;
        display_line[3][4] = ones;
        display_changed = 1;
    }
    if(right_flag){
        right_flag = 0;
        BCD_Convert (ADC_Right_Detect);
        display_line[3][5] = 'R';
        display_line[3][6] = thousands;
        display_line[3][7] = hundreds;
        display_line[3][8] = tens;
        display_line[3][9] = ones;
        display_changed = 1;
    }
}

//Seek and navigate black line command
void black_line(void){
    const double Kp = 183.3;
    const double Kd = 102.3;
    const double Ki = 0.02;

    switch(segment){
        case WAIT_CASE:
            if(one_second){
                no_movement();
                one_second = 0;
                delay_amount++;
                if(delay_amount >= 3){
                    segment = saved_case;
                    delay_amount = 0;
                }
            }
            break;
        case DISPLAY_CHECK:
            if(one_second){
                one_second = 0;
                delay_amount++;
                if(delay_amount >= 7){
                    segment = saved_case;
                    delay_amount = 0;
                }
            }
            break;
        case OVERSHOOT:
            if(zero_point_one){
                zero_point_one = 0;
                small_delay_amount++;
                if(small_delay_amount >= 6){
                    no_movement();
                    segment = WAIT_CASE;
                    small_delay_amount = 0;
                }
            }
            break;
        case FIND_LINE:
              slow_forward();
              if(intersect_flag){
                segment = OVERSHOOT;
                saved_case = TURN_TO_LINE;
                strcpy(display_line[0], "INTERCEPT ");
                display_changed = 1;
              }
            break;
        case TURN_TO_LINE:
            slow_right();
            strcpy(display_line[0], "BL TURN   ");
            display_changed = 1;
            if(left_turn_flag){
                left_turn_flag = 0;
                no_movement();
                slow_left();
                if(full_turn_flag){
                    full_turn_flag = 0;
                    no_movement();
                    segment = DISPLAY_CHECK;
                    saved_case = NAV_LINE;
                }
            }
            if(full_turn_flag){
                no_movement();
                full_turn_flag = 0;
                saved_case = NAV_LINE;
                segment = DISPLAY_CHECK;
            }
            break;
        case NAV_LINE:
            if(speed_update){
                if(one_second){
                    one_second = 0;
                    circle_time++;
                    if(circle_time >= 15){
                        circle_time = 0;
                        no_movement();
                        strcpy(display_line[0], "BL CIRCLE ");
                        display_changed = 1;
                        segment = WAIT_CASE;
                        saved_case = NAV_LINE;
                    }
                }
                speed_update = 0;
                int min_speed = 5000;
                display_changed = 1;


                int p_error = ADC_Right_Detect - ADC_Left_Detect;
                int d_error = p_error - prev_error;
                prev_error = p_error;

                i_error += p_error;
                if(i_error > 1000) i_error = 1000;
                if(i_error < -1000) i_error = -1000;



                int correction = (int)((Kp*p_error + Kd*d_error + Ki*i_error)*(1.2*GOLDY/SLOW));

                if(correction > 15000) correction = 15000;
                if(correction < -15000) correction = -15000;

                right_forward = GOLDY - correction;
                left_forward = GOLDY + correction;

                if(correction < 0){
                    last_dir = 0;
                }else{
                    last_dir = 1;
                }

                if(right_forward > WHEEL_PERIOD) right_forward = LUCID;
                if(left_forward > WHEEL_PERIOD) left_forward = LUCID;

                if(right_forward < min_speed) right_forward = min_speed;
                if(left_forward < min_speed) left_forward = min_speed;

                if((ADC_Right_Detect <= (white_value_R * 1.01 )) && (ADC_Left_Detect <= (white_value_L * 1.01))){
                    no_movement();
                    if(last_dir){
                        medium_left();
                    }else{
                        medium_right();
                    }
                }
                else {
                    no_movement();
                    RIGHT_FORWARD_SPEED = right_forward;
                    LEFT_FORWARD_SPEED  = left_forward;
                }
            }
            break;
        default: break;
    }
}

void black_flag(void){
    full_turn_flag = 0;
    left_turn_flag = 0;
    intersect_flag = 0;

    if(ADC_Right_Detect >= 730 || ADC_Left_Detect >= 780){
        intersect_flag = 1;
    }

    if(ADC_Right_Detect >= black_value_R && ADC_Left_Detect >= black_value_L){
        full_turn_flag = 1;
    }

    if(ADC_Right_Detect < 905 && ADC_Left_Detect >= 900){
        left_turn_flag = 1;
    }
}




