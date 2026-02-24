/*
 * sequence.c
 *
 *  Created on: Feb 24, 2026
 *      Author: everettbrostedt
 */

#include <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"

extern volatile unsigned int project_5_flag;
extern volatile int one_second;
int timer = 0;
extern unsigned char display_mode_i;
extern volatile unsigned int update_display_count;
extern volatile unsigned char display_changed;
extern char display_line[4][11];
void project_5(void){
    if(one_second){
        one_second = 0;

        timer++;
        switch(display_mode_i){
                         case FORWARD_I:
                             strcpy(display_line[0], "          ");
                             strcpy(display_line[1], "          ");
                             strcpy(display_line[2], "  FORWARD ");
                             strcpy(display_line[3], "          ");
                             display_changed = TRUE;
                             Display_Process();
                             break;
                         case STOP_I:
                             strcpy(display_line[0], "          ");
                             strcpy(display_line[1], "          ");
                             strcpy(display_line[2], "   STILL  ");
                             strcpy(display_line[3], "          ");
                             display_changed = TRUE;
                             Display_Process();
                             break;
                         case TURN_I:
                             strcpy(display_line[0], "          ");
                             strcpy(display_line[1], "          ");
                             strcpy(display_line[2], "   TURN   ");
                             strcpy(display_line[3], "          ");
                             display_changed = TRUE;
                             Display_Process();
                             break;
                         case REVERSE_I:
                             strcpy(display_line[0], "          ");
                             strcpy(display_line[1], "          ");
                             strcpy(display_line[2], " REVERSE  ");
                             strcpy(display_line[3], "          ");
                             display_changed = TRUE;
                             Display_Process();
                         default: break;
                        }
        switch(timer){
            case 2://initial start
                forward_movement();
                display_mode_i = FORWARD_I;
                break;
            case 3://move for 1 second
                no_movement();
                display_mode_i = STOP_I;
                break;
            case 4://pause for 1 second
                reverse_movement();
                display_mode_i = STOP_I;
                break;
            case 6://reverse for 2 second
                no_movement();
                display_mode_i = REVERSE_I;
                break;
            case 7://pause for 1 second
                forward_movement();
                display_mode_i = FORWARD_I;
                break;
            case 8://forward for 1 second
                no_movement();
                display_mode_i = STOP_I;
                break;
            case 9://pause for 1 second
                turn_right();
                display_mode_i = TURN_I;
                break;
            case 12://turning 3 seconds clockwise
                no_movement();
                display_mode_i = STOP_I;
                break;
            case 14://pause for 2 seconds
                turn_left();
                display_mode_i = TURN_I;
                break;
            case 17://turn 3 seconds counterclockwise
                no_movement();
                display_mode_i = STOP_I;
                break;
            case 19:
                timer = 0;
                project_5_flag = 0;
                break;//pause for 2 seconds
    }
  }
}


