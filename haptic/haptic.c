#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "usb.h"
#include "pin.h"
#include "uart.h"
#include "ui.h"
#include "timer.h"
#include "oc.h"
#include <stdio.h>

#define HELLO       0   // Vendor request that prints "Hello World!"
#define SET_VALS    1   // Vendor request that receives 2 unsigned integer values
#define GET_VALS    2   // Vendor request that returns 2 unsigned integer values
#define PRINT_VALS  3   // Vendor request that prints 2 unsigned integer values 

#define THRESHOLD 1000
#define WAITING 0 
#define SENDING 1

uint16_t val1, val2, state, temp_dist, last_dist;

int16_t main(void) {
    init();
    uint16_t started = 0;
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }

    while (1) {

        ServiceUSB();                       // service any pending USB requests
        if (timer_flag(&timer1)) {
            timer_lower(&timer1);
            led_toggle(&led1);
        }
    }
}

void __attribute__((interrupt, auto_psv)) _T2Interrupt(void);
void __attribute__((interrupt, auto_psv)) _T2Interrupt(void){
    _T2IF = 0;
    led_toggle(&led2);
}

void init(void){

    init_clock();
    init_ui();
    init_uart();
    init_timer();
    init_pin();
    init_oc();

    // Set up timer for LED flashing
    timer_setPeriod(&timer1, 0.5);
    timer_start(&timer1);

    // Set up digital output pins
    pin_digitalOut(&D[2]);
    pin_digitalOut(&D[3]);
    pin_digitalOut(&D[4]);
    pin_digitalOut(&D[5]);
    pin_digitalOut(&D[6]);
    pin_digitalOut(&D[7]);
    pin_digitalOut(&D[8]);
    
    pin_set(&D[2]);
    pin_clear(&D[3]);
    pin_set(&D[4]);
    pin_clear(&D[6]);
    pin_clear(&D[7]);
    pin_clear(&D[8]);

    oc_pwm(&oc1, &D[5], NULL, 20E3, 65000);
    oc_pwm(&oc2, &D[10], NULL, 20, 65000);

    InitUSB();
    printf("\n\nHello world\n");
}


void VendorRequests(void) {
    WORD temp;

    switch (USB_setup.bRequest) {
        case HELLO:
            printf("Hello World!\n");
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case SET_VALS:
            val1 = USB_setup.wValue.w;
            val2 = USB_setup.wIndex.w;
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_VALS:
            temp.w = last_dist;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            temp.w = val2;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];
            BD[EP0IN].bytecount = 4;    // set EP0 IN byte count to 4
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;            
        case PRINT_VALS:
            printf("val1 = %u, val2 = %u\n", val1, val2);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        default:
            USB_error_flags |= 0x01;    // set Request Error Flag
    }
}

void VendorRequestsIn(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

void VendorRequestsOut(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

