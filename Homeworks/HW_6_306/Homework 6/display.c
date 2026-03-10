/*
 * display.c
 *
 *  Created on: Feb 5, 2026
 *      Author: everettbrostedt
 *
 * -----------------------------------------------------------------------------------------------------
 * Description: This file contains the display_process function. This function updates the update_display and
 * display_changed variables based on the current values of update_display and display_changed variables.
 * -----------------------------------------------------------------------------------------------------
 */

#include  "msp430.h"
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"
#include  "globals.h"

int blink_count = 0;

void Display_Process(void){
  if(update_display){
    update_display = 0;
    blink_count++;
    if(blink_count == 1 && mode_flag){
        P6OUT &= ~LCD_BACKLITE;
    }else if(blink_count >= 2 && mode_flag){
        P6OUT |= LCD_BACKLITE;
        blink_count = 0;
    }else{
        P6OUT &= ~LCD_BACKLITE;
    }
    if(display_changed){
      display_changed = 0;
      Display_Update(0,0,0,0);
    }
  }
}
