/*
 * serial.c
 *
 *  Created on: Mar 26, 2026
 *      Author: everettbrostedt
 */

#include  <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "globals.h"
#include  "macros.h"

//LCD display arrays
char ncsu_display[] = "NCSU  #1\r\n";
char speed_1[] = " 115,200  ";
char speed_2[] = " 460,800  ";

// Serial Flags
int serial_part = 0;
int temp_rx_size_0 = 0;
int temp_rx_size_1 = 0;
int iot_rx_rd = 0;

int transmit_flag_0 = 0;
int transmit_flag_1 = 0;

unsigned int serial_flag = 0;
unsigned int serial_state = STARTUP;

//MSP430 stores
char display__rx_message[11];
char display__tx_message[11];
char speed_type[10];
char safe[32];

// Baud config
unsigned int set_baud = 4;
unsigned int tuning = ONE_FIFTEEN;

//Buffers
char IOT_Ring_Rx[32];
char USB_Ring_Rx[32];
char iot_TX_buf[32];
char usb_TX_buf[32];

// Indexes
volatile unsigned int iot_rx_wr = 0;
volatile unsigned int usb_rx_wr = 0;
volatile unsigned int iot_tx = 0;
volatile unsigned int usb_tx = 0;

void Init_Serial_UCA0(int speed){
//------------------------------------------------------------------------------
// TX error (%) RX error (%)
// BRCLK    Baudrate    UCOS16  UCBRx   UCFx    UCSx    neg pos   neg  pos
// 8000000  4800        1       104     2       0xD6 -0.08 0.04 -0.10 0.14
// 8000000  9600        1       52      1       0x49 -0.08 0.04 -0.10 0.14
// 8000000  19200       1       26      0       0xB6 -0.08 0.16 -0.28 0.20
// 8000000  57600       1       8       10      0xF7 -0.32 0.32 -1.00 0.36
// 8000000  115200      1       4       5       0x55 -0.80 0.64 -1.12 1.76
// 8000000  460800      0       17      0       0x4A -2.72 2.56 -3.76 7.28
//------------------------------------------------------------------------------
// Configure eUSCI_A0 for UART mode
    UCA0CTLW0 = 0;
    UCA0CTLW0 |= UCSWRST ;              // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;         // Set SMCLK as fBRCLK
    UCA0CTLW0 &= ~UCMSB;                // MSB, LSB select
    UCA0CTLW0 &= ~UCSPB;                // UCSPB = 0(1 stop bit) OR 1(2 stop bits)
    UCA0CTLW0 &= ~UCPEN;                // No Parity
    UCA0CTLW0 &= ~UCSYNC;
    UCA0CTLW0 &= ~UC7BIT;
    UCA0CTLW0 |= UCMODE_0;

    UCA1CTLW0 = 0;
    UCA1CTLW0 |= UCSWRST ;              // Put eUSCI in reset
    UCA1CTLW0 |= UCSSEL__SMCLK;         // Set SMCLK as fBRCLK
    UCA1CTLW0 &= ~UCMSB;                // MSB, LSB select
    UCA1CTLW0 &= ~UCSPB;                // UCSPB = 0(1 stop bit) OR 1(2 stop bits)
    UCA1CTLW0 &= ~UCPEN;                // No Parity
    UCA1CTLW0 &= ~UCSYNC;
    UCA1CTLW0 &= ~UC7BIT;
    UCA1CTLW0 |= UCMODE_0;

    UCA0BRW = 4;  //variable that changes the baud rate
    UCA0MCTLW = 0x5551 ; //variable that is change by the switch in order to fine tune the output

    UCA1BRW = 4;
    UCA1MCTLW = 0x5551 ;

    UCA0CTLW0 &= ~UCSWRST ; // release from reset
    UCA0TXBUF = 0x00; // Prime the Pump
    UCA0IE |= UCRXIE; // Enable RX interrupt

    UCA1CTLW0 &= ~UCSWRST;
    UCA1TXBUF = 0x00;
    UCA1IE |= UCRXIE;
    //------------------------------------------------------------------------------
}


#pragma vector = EUSCI_A0_VECTOR
__interrupt void eUSCI_A0_ISR(void){ //This interrupt is the interrupt relating to serial communication port UCA0
 //------------------------------------------------------------------------------
 // Interrupt name: eUSCI_A0_ISR
 // Description: This interrupt transmits and receives through UCA0
 //------------------------------------------------------------------------------
 char iot_receive;
 switch(__even_in_range(UCA0IV,0x08)){
     case 0: break; //Vector 0 - no interrupt
     case 2: // Vector 2 – Rx0IFG
         iot_receive = UCA0RXBUF;
         IOT_Ring_Rx[iot_rx_wr++] = iot_receive;
         if(iot_rx_wr >= sizeof(IOT_Ring_Rx)){
             temp_rx_size_0 = iot_rx_wr;
             iot_rx_wr = BEGINNING;
         }
         if(iot_receive == '\r'){
             transmit_flag_0 = 1;
             temp_rx_size_0 = iot_rx_wr;
         }
         UCA1TXBUF = iot_receive;
     break;
     case 4: // Vector 4 – Tx0IFG
         UCA0TXBUF = iot_TX_buf[iot_tx];
         iot_TX_buf[iot_tx++] = 0;
         if(iot_TX_buf[iot_tx] == 0x00){
             UCA0IE &= ~UCTXIE;
             iot_tx = 0;

         }
     break;
     default: break;
 }
}


#pragma vector = EUSCI_A1_VECTOR
__interrupt void eUSCI_A1_ISR(void){ //This interrupt is the interrupt relating to serial communication port UCA1
//------------------------------------------------------------------------------
// Interrupt name: eUSCI_A1_ISR
// Description: This interrupt transmits and receives through UCA1
//------------------------------------------------------------------------------
    char usb_value;
    switch(__even_in_range(UCA1IV,0x08)){
        case 0: break; //Vector 0 - no interrupt
        case 2: // Vector 2 – Rx1IFG
            usb_value = UCA1RXBUF;
             USB_Ring_Rx[usb_rx_wr++] = usb_value;
             if(usb_rx_wr++ >= sizeof(USB_Ring_Rx)){
                 usb_rx_wr = BEGINNING;
             }
             if(usb_value == '\r'){
                 transmit_flag_1 = 1;
                 temp_rx_size_1 = usb_rx_wr;
             }
             UCA0TXBUF = usb_value;
             break;
         case 4: // Vector 4 – TX1IFG
             UCA1TXBUF = usb_TX_buf[usb_tx];
             usb_TX_buf[usb_tx++] = 0;
             if(usb_TX_buf[usb_tx] == 0x00){
                 UCA1IE &= ~UCTXIE;
                 usb_tx = 0;

             }
         break;
        default:break;
}
//------------------------------------------------------------------------------
}

void serial_update(void){
    int j = 0;
    int i = 0;

    if(speed_flag){
        strcpy(speed_type, speed_2);
    }else{
        strcpy(speed_type, speed_1);
    }

    switch(serial_state){
    case STARTUP:
        strcpy(display_line[0], "  STARTUP ");
        strcpy(display_line[1], "          ");
        strcpy(display_line[2], speed_type);
        strcpy(display_line[3], "          ");
        display_changed = 1;
        Display_Process();
        break;
    case RECEIVE:
        UCA1IE |= UCRXIE;
        if(!transmit_flag_1){
           strcpy(USB_Ring_Rx, "         \0");
        }
        if(transmit_flag_1){
        transmit_flag_1 = 0;
        for(i = 0; i<11; i++){
           if(USB_Ring_Rx[i] == '\0') continue;
           display__rx_message[j] = IOT_Ring_Rx[i];
           j++;
        }
        display__rx_message[j++] = ' ';
        display__rx_message[j] = '\0';

        strcpy(display_line[0], " RECEIVED ");
        strcpy(display_line[3], display__rx_message);
        display_changed = 1;
        Display_Process();
        break;
    case TRANSMIT:
        UCA1IE |= UCTXIE;
        strcpy(usb_TX_buf, display__rx_message);
        usb_tx = 0;

        for(i = 0; i<11; i++){
           if(usb_TX_buf[i] == '\0') continue;
           display__tx_message[j] = usb_TX_buf[i];
           j++;
        }
        display__tx_message[j++] = ' ';
        display__tx_message[j] = '\0';

        strcpy(display_line[0], " TRANSMIT ");
        strcpy(display_line[1], display__tx_message);
        strcpy(display_line[3], "          ");
        display_changed = 1;
        Display_Process();
        break;
    case DISPLAY:
        UCA0IE |= UCRXIE;
        if(!transmit_flag_0){
            strcpy(IOT_Ring_Rx, "          ");
        }
        if(transmit_flag_0){
        for(i = 0; i<11; i++){
            if(IOT_Ring_Rx[i] == '\0') continue;
            temp[j] = IOT_Ring_Rx[i];
            j++;
        }
        temp[j++] = ' ';
        temp[j] = '\0';
        strcpy(display_line[0], temp);
        display_changed = 1;
        Display_Process();
        serial_state = FINAL;
        transmit_flag = 0;
    }
        break;
    default:break;
    }
}

// Global Variables


/*void USCI_A0_transmit(void){ // Transmit Function for USCI_A0
// Contents must be in process_buffer
// End of Transmission is identified by NULL character in process_buffer
// process_buffer includes Carriage Return and Line Feed
 pb_index = 0; // Set Array index to first location
 UCA0IE |= UCTXIE; // Enable TX interrupt
}
*/

