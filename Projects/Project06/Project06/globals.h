/*
 * globals.h
 *
 *  Created on: Mar 1, 2026
 *      Author: everettbrostedt
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

extern volatile unsigned int Time_Sequence;
extern volatile char one_time;
extern volatile unsigned char display_changed;
extern volatile unsigned char update_display;
extern char display_line[4][11];
extern char *display[4];
extern unsigned char display_mode_i;
extern unsigned int project_5_flag;
extern volatile unsigned int one_second;
extern volatile unsigned int zero_point_one;

extern unsigned char event;
extern unsigned int state;
extern volatile int mode;
extern unsigned int movement;
extern unsigned int start_flag;

extern unsigned int cycle_time;
extern unsigned int time_change;

extern volatile unsigned int ADC_Thumb;
extern volatile unsigned int ADC_Left_Detect;
extern volatile unsigned int ADC_Right_Detect;

extern volatile unsigned char left_flag;
extern volatile unsigned char right_flag;
extern volatile unsigned char thumb_flag;

extern char thousands;
extern char hundreds;
extern char tens;
extern char ones;

extern volatile int condition;
extern int delay_amount;
extern int small_delay_amount;
extern unsigned int line_detection_flag;
extern unsigned int first;

#endif /* GLOBALS_H_ */
