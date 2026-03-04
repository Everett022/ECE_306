 //------------------------------------------------------------------------------
//
//  Description: This file contains the Main Routine - "While" Operating System
//
//  Jim Carlson
//  Jan 2023
//  Built with Code Composer Version: CCS12.4.0.00007_win64
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#include  <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "LCD.h"
#include  "ports.h"
#include  "macros.h"

// Function Prototypes
void main(void);
void Init_Conditions(void);
void Init_LEDs(void);
void Carlson_StateMachine(void);

// Global Variables
volatile char slow_input_down;
unsigned char display_mode;
extern volatile unsigned int update_display_count;
extern volatile unsigned char display_changed;
extern char display_line[4][11];
unsigned int test_value;
char chosen_direction;
char change;
unsigned int wheel_move;
char forward;
unsigned int Last_Time_Sequence;                        //a variable declared to determine if Time_Sequence has changed
unsigned int cycle_time = 0;                            //a new time base used to control making shapes
unsigned int time_change = 0;                           //identifier that a change has occurred.
extern volatile unsigned int Time_Sequence;
extern volatile char one_time;
extern volatile unsigned char display_changed;
extern volatile unsigned int project_5_flag;

unsigned char event = NONE;
unsigned int state = WAIT;
extern volatile unsigned int one_second;
unsigned char display_mode_i = FORWARD_I;

//void main(void){
void main(void){
//    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

//------------------------------------------------------------------------------
// Main Program
// This is the main routine for the program. Execution of code starts here.
// The operating system is Back Ground Fore Ground.
//
//------------------------------------------------------------------------------
  PM5CTL0 &= ~LOCKLPM5;
// Disable the GPIO power-on default high-impedance mode to activate
// previously configured port settings

  Init_Ports();                        // Initialize Ports
  Init_Clocks();                       // Initialize Clock System
  Init_Conditions();                   // Initialize Variables and Initial Conditions
  Init_Timer_B0();                     // Initialize the timer B0 that was made
  Init_LCD();                          // Initialize LCD
//P2OUT &= ~RESET_LCD;
  // Place the contents of what you want on the display, in between the quotes
// Limited to 10 characters per line
  strcpy(display_line[0], "   NCSU   ");
  strcpy(display_line[1], " WOLFPACK ");
  strcpy(display_line[2], "  ECE306  ");
  strcpy(display_line[3], "  GP I/O  ");
  display_changed = TRUE;
//  Display_Update(0,0,0,0);

  wheel_move = 0;
  forward = TRUE;

//------------------------------------------------------------------------------
// Beginning of the "While" Operating System
//------------------------------------------------------------------------------
  while(ALWAYS) {                      // Can the Operating system run
    //Carlson_StateMachine();            // Run a Time Based State Machine
    Switches_Process();                // Check for switch state change
    Display_Process();                 // Update Display
    Switch_mode();                     //Switch 1 mode call
    Switch_mode_2();                   //Switch 2 mode call
    P3OUT ^= TEST_PROBE;               // Change State of TEST_PROBE OFF

    if(Last_Time_Sequence != Time_Sequence){
     Last_Time_Sequence = Time_Sequence;
     cycle_time++;
     time_change = 1;
     }

    if ((P6IN & L_FORWARD) && (P6IN & L_REVERSE)) {
        P6IN &= ~L_FORWARD;
        P6IN &= ~L_REVERSE;
        P1OUT |= RED_LED;
        }

    if ((P6IN & R_FORWARD) && (P6IN & R_REVERSE)) {
        P6IN &= ~R_FORWARD;
        P6IN &= ~R_REVERSE;
        P1OUT |= RED_LED;
    }

    if(project_5_flag){
        project_5();
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
    }

  }
}





