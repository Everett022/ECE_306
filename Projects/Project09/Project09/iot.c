/*
 * iot.c
 *
 *  Created on: Mar 27, 2026
 *      Author: everettbrostedt
 */

#include  <string.h>
#include  "msp430.h"
#include  "functions.h"
#include  "ports.h"
#include  "globals.h"
#include  "macros.h"

int input_rd = 0;
unsigned int command_flag = 0;
unsigned int iot_state = 0;
unsigned int iot_status = IDLE;
unsigned int iot_startup_timer = 0;
unsigned int configured_flag = 0;

unsigned int advance_0 = 0;
unsigned int advance_1 = 0;
unsigned int advance_2 = 0;
unsigned int advance_3 = 0;

void IOT_State(void){
    switch(iot_status){
        case IDLE:
            P3OUT &= ~IOT_EN;
            zero_point_one = 0;
            iot_status = IOT_STARTUP;
            iot_startup_timer = 0;
            break;
        case IOT_STARTUP:
            if(zero_point_one){
                zero_point_one = 0;
                iot_startup_timer++;
                if(iot_startup_timer >= 2){
                    P3OUT |= IOT_EN;
                }
                if(iot_startup_timer >= 4){
                    temp_index_iot = 0;
                    temp_index_usb = 0;
                    configured_flag = 0;
                    iot_startup_timer = 0;
                    iot_status = COMMAND_START;
                }
            }
            break;
        case COMMAND_START:
            initial_process_iot();
            if(wifi_connected && ip_received){
                wifi_connected = 0;
                ip_received = 0;
                strcpy(iot_TX_buf, "AT+SYSSTORE=0\r\n");
                iot_tx = 0;
                UCA0IE |= UCTXIE;
                iot_status = MUX_RUN;
            }
            break;
        case MUX_RUN:
            initial_process_iot();
            if(ok_received){
                ok_received = 0;
                strcpy(iot_TX_buf, "AT+CIPMUX=1\r\n");
                iot_tx = 0;
                UCA0IE |= UCTXIE;
                iot_status = SERVER_RUN;
            }
            break;
        case SERVER_RUN:
            initial_process_iot();
            if(ok_received){
                ok_received = 0;
                strcpy(iot_TX_buf, "AT+CIPSERVER=1,8080\r\n");
                iot_tx = 0;
                UCA0IE |= UCTXIE;
                iot_status = IOT_READY;
            }
            break;
        case IOT_READY:
            initial_process_iot();
            if(ok_received){
                strcpy(display_line[2], "IOT READY!");
                display_changed = 1;

                configured_flag = 1;
                iot_status = IOT_DISPLAY;
            }
            break;
        case IOT_DISPLAY:
            if(configured_flag && sw1_position){
                configured_flag = 0;
                sw1_position = 0;
                ok_received = 0;

                strcpy(iot_TX_buf, "AT+CWJAP?\r\n");
                iot_tx = 0;
                UCA0IE |= UCTXIE;
                iot_status = SSID;
            }
            break;
        case SSID:
            initial_process_iot();
            if(ssid_string_flag){
                display_line[0][0] = 'S';
                display_line[0][1] = 'S';
                display_line[0][2] = 'I';
                display_line[0][3] = 'D';
                display_line[0][4] = ':';
                display_line[0][5] = ssid_string[0];
                display_line[0][6] = ssid_string[1];
                display_line[0][7] = ssid_string[2];
                display_line[0][8] = ssid_string[3];
                display_line[0][9] = '\0';
                strcpy(display_line[2], "          ");
                display_changed = 1;
                if(ok_received){
                    ok_received = 0;
                    wifi_address = 0;
                    ssid_string_flag = 0;
                    ending_quote = 0;
                    temp_index_iot = 0;

                    strcpy(iot_TX_buf, "AT+CIFSR\r\n");
                    iot_tx = 0;
                    UCA0IE |= UCTXIE;
                    iot_status = IP_ADDY;
                }
            }
            break;
        case IP_ADDY:
            initial_process_iot();
            if(ssid_string_flag){
                strcpy(display_line[1], "IP: ");
                display_line[2][0] = ssid_string[0];
                display_line[2][1] = ssid_string[1];
                display_line[2][2] = ssid_string[2];
                display_line[2][3] = ssid_string[3];
                display_line[2][4] = ssid_string[4];
                display_line[2][5] = ssid_string[5];
                display_line[2][6] = ssid_string[6];
                display_line[2][7] = ssid_string[7];
                display_line[2][8] = ssid_string[8];
                display_line[2][9] = ssid_string[9];
                display_line[3][0] = ssid_string[10];
                display_line[3][1] = ssid_string[11];
                display_line[3][2] = ssid_string[12];
                display_line[3][3] = ssid_string[13];
                display_changed = 1;
            }
            if(ok_received){
                ok_received = 0;
                ssid_string_flag = 0;
                wifi_address = 0;
                ending_quote = 0;
                temp_index_iot = 0;
                input_rd = 0;
                temp_index_iot = 0;
                command_flag = 0;
                iot_status = IOT_RUN;
            }
            break;
        case IOT_RUN:
            rx_process_iot();
            IOT_Process();
            break;
        default:break;
    }
}

void IOT_Process(void){
    iot_state = display_iot_rx_message[input_rd++];

    if (input_rd >= sizeof(display_iot_rx_message)){
        input_rd = BEGINNING;
    }

    if (command_flag){
        switch (iot_state){
            case '^':
                uca1_flag = 1;
                strcpy(repeat, " I'm here bbg \n\0");
                break;
            case 'F':
                uca1_flag = 1;
                UCA0BRW = 4;
                UCA0MCTLW = 0x5551;
                UCA1BRW = 4;
                UCA1MCTLW = 0x5551;

                strcpy(display_line[0], "   BAUD   ");
                strcpy(display_line[1], "          ");
                strcpy(display_line[2], speed_1);
                strcpy(display_line[3], "          ");
                display_changed = 1;
                break;
            case 'S':
                uca1_flag = 1;
                UCA0BRW = 52;
                UCA0MCTLW = 0x4911;
                UCA1BRW = 52;
                UCA1MCTLW = 0x4911;

                strcpy(repeat, " turtle mode ON! \n\0");
                strcpy(display_line[0], "   BAUD   ");
                strcpy(display_line[2], speed_3);
                display_changed = 1;
                break;
            default:
                break;
        }
    }
    else
    {
        if (iot_state == '^'){
            advance_0 = 1;
            return;
        }if (iot_state == '0' && advance_0){
            advance_1 = 1;
            return;
        }if (iot_state == '8' && advance_1){
            advance_2 = 1;
            return;
        }if (iot_state == '6' && advance_2){
            advance_3 = 1;
            return;
        }if (iot_state == '4' && advance_3){
            command_flag = 1;
            return;
        }
        command_flag = 0;
        advance_0 = 0;
        advance_1 = 0;
        advance_2 = 0;
        advance_3 = 0;
    }
}

