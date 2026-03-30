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

//Serial
char ncsu_display[] = "NCSU  #1\r\n";
int serial_part = 0;
int temp_rx_size = 0;
int iot_rx_rd = 0;
char temp[11];
char speed_1[] = " 115,200  ";
char speed_2[] = " 460,800  ";
char speed_type[10];
char safe[32];
int transmit_flag = 0;

unsigned int serial_flag = 0;
unsigned int serial_state = STARTUP;

// Baud config
unsigned int set_baud = 4;
unsigned int tuning = ONE_FIFTEEN;

//Buffers
char IOT_Ring_Rx[32];
char USB_Ring_Rx[32];
char iot_TX_buf[32];

// Indexes
volatile unsigned int iot_rx_wr = 0;
volatile unsigned int usb_rx_wr = 0;
volatile unsigned int iot_tx = 0;
volatile unsigned int direct_iot = 0;
unsigned char iox_TX_buf[25];           // Size for appropriate Command Length


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

    UCA1BRW = 17;
    UCA1MCTLW = 0x4A00;

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
             temp_rx_size = iot_rx_wr;
             iot_rx_wr = BEGINNING;
         }
         if(iot_receive == '\r'){
             transmit_flag = 1;
             temp_rx_size = iot_rx_wr;
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
        case 2: // Vector 2 - Rx1IFG
            usb_value = UCA1RXBUF;
            USB_Ring_Rx[usb_rx_wr++] = usb_value; // Add to Ring Buffer
            if(usb_rx_wr >= sizeof(USB_Ring_Rx)){
                usb_rx_wr = BEGINNING;
            }
            UCA0TXBUF = usb_value;
            // UCA0IE |= UCTXIE;
        break;
        case 4: // Vector 4 - TX1IFG
            UCA1TXBUF = IOT_Ring_Rx[direct_iot++];
            // IOT_Ring_Rx[direct_iot++]= 0;
            if(direct_iot >= sizeof(IOT_Ring_Rx)){
                direct_iot = BEGINNING;
            }
            if(iot_rx_wr == direct_iot){
                UCA1IE &= ~UCTXIE;
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
    if(one_second){
        one_second = 0;
        serial_part++;
        if(serial_part == 1){
            serial_state = STARTUP;
        }else if(serial_part == 3){
            serial_state = TRANSMIT;
        }else if (serial_state == TEST && serial_part == 5){
            serial_state = DISPLAY;
            iot_rx_rd = 0;
            i = 0;
        }
    }


    switch(serial_state){
    case STARTUP:
        strcpy(display_line[0], "This      ");
        strcpy(display_line[1], "   is     ");
        strcpy(display_line[2], " my great ");
        strcpy(display_line[3], " project  ");
        display_changed = 1;
        Display_Process();
        break;
    case TRANSMIT:
        strcpy(display_line[0], "          ");
        strcpy(display_line[1], "          ");
        strcpy(display_line[2], "   Baud   ");
        strcpy(display_line[3], speed_type);

        display_changed = 1;
        Display_Process();

        strcpy(iot_TX_buf, ncsu_display);
        iot_tx = 0;
        UCA0IE |= UCTXIE;


        serial_state = TEST;
        break;
    case DISPLAY:
        UCA0IE |= UCRXIE;
        if(!transmit_flag){
            strcpy(display_line[0], "          ");
        }
        if(transmit_flag){
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

