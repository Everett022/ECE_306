/*
 * iot.c
 *
 *  Created on: Mar 27, 2026
 *      Author: everettbrostedt
 */

#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "globals.h"
#include  "macros.h"

unsigned char IOT_Ring_Rx[64];         // Ring buffer for received IOT bytes
unsigned char IOT_Data[4][25];         // 4 lines x 25 characters buffer for parsing
unsigned char iot_TX_buf[25];          // transmit buffer for IOT (already using as global)
unsigned char test_Value;               // used in case 0 to trigger RED_LED
unsigned char IOT_parse;                // flag to indicate a command to parse
unsigned char boot_state;               // set in case 4 when 'y' is received
unsigned char ip_address_found;         // set in case 10 when 'I' received
unsigned char display_changed;          // flag for updating the display
unsigned char iot_index;                // reset in case 10
extern unsigned char AT[];              // AT command string
extern unsigned char ip_mac[];          // MAC address string
extern unsigned char ip_address[];      // IP address storage
extern char display_line[4][20];        // display buffer lines

 void IOT_Process(void){ // Process IOT messages
  int i;
  unsigned int iot_rx_wr_temp;
  iot_rx_wr_temp = iot_rx_wr;
  if(iot_rx_wr_temp != iot_rx_rd){
    IOT_Data[line][character] = IOT_Ring_Rx[iot_rx_rd++];
    if(iot_rx_rd >= sizeof(IOT_Ring_Rx)){
      iot_rx_rd = BEGINNING;
    }
    if(IOT_Data[line][character] == 0x0A){
      character = 0;
      line++;
      if(line >= 4){
        line = 0;
      }
      nextline = line + 1;
      if(nextline >= 4){
        nextline = 0;
      }
    }else{
      switch(character){
        case 0:
          if(IOT_Data[line][character] == '+'){ // Got "+"
            test_Value++;
            if(test_Value){
              RED_LED_ON;
            }
            IOT_parse = 1;
          }
          break;
        case 1:
          // GRN_LED_ON;
          break;
        case 4:
          if(IOT_Data[line][character] == 'y'){
            for(i = 0; i < sizeof(AT); i++){
              iot_TX_buf[i] = AT[i];
            }
            iot_tx = 0;
            UCA0IE |= UCTXIE;
            boot_state = 1;
            // RED_LED_ON;
            GRN_LED_OFF;
          }
          break;
        case 5:
          if(IOT_Data[line][character] == 'G'){ // Got IP
            for(i = 0; i < sizeof(ip_mac); i++){
              iot_TX_buf[i] = ip_mac[i];
            }
            iot_tx = 0;
            UCA0IE |= UCTXIE;
          }
          break;
        case 6:
          break;
        case 10:
          if(IOT_Data[line][character] == 'I'){
            ip_address_found = 1;
            strcpy(display_line[0], "IP Address");
            for(i = 0; i < sizeof(ip_address); i++){
              ip_address[i] = 0;
            }
            display_changed = 1;
            iot_index = 0;
          }
          break;
        default:
          break;
      }
      character++;
    }
  }
}
