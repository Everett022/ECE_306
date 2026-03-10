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
#include  "globals.h"

// Function Prototypes
void main(void);
void Init_Conditions(void);
void Init_LEDs(void);
void Carlson_StateMachine(void);

// Global Variables
volatile char slow_input_down;
unsigned char display_mode;
unsigned int test_value;
char chosen_direction;
char change;
unsigned int wheel_move;
char forward;
unsigned int Last_Time_Sequence;                        //a variable declared to determine if Time_Sequence has changed
unsigned int cycle_time = 0;                            //a new time base used to control making shapes
unsigned int time_change = 0;                           //identifier that a change has occurred.
unsigned char event = NONE;
unsigned int state = WAIT;
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
  Init_ADC();                          // Initialize ADC

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
    Switch_mode();                     //Switch 1 mode call
    Switch_mode_2();                   //Switch 2 mode call
    detect();                          //Constantly converting values from ADC and displaying
    if(line_detection_flag){
        line_detection();
    }
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


  }
}





