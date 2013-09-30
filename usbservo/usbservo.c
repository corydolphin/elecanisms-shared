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


#define WAITING 0 
#define SENDING 1

uint16_t val1, val2;

void init(void){
    init_clock();
    init_ui();
    init_timer();
    init_pin();
    init_oc();

    timer_setPeriod(&timer4, 0.95);
    timer_start(&timer4);

    pin_digitalOut(&D[0]);
    pin_digitalOut(&D[1]);
    pin_digitalOut(&D[2]);
    oc_servo(&oc1, &D[0], &timer1, 20E-3, 6E-4, 2.2E-3, 0);
    oc_servo(&oc2, &D[1], &timer2, 20E-3, 6E-4, 2.2E-3, 0);

    oc_pwm(&oc3, &D[2], NULL, 40E3, 32000);

    led_on(&led1);
    val1 = 5;
    val2 = 2;

    InitUSB();
}


int16_t main(void) {
    init();
              
    uint16_t duty, state;
    duty = 0;
    state = WAITING;
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }
    while (1) {
        ServiceUSB();                       // service any pending USB requests
        pin_write(&D[0], val1);
        pin_write(&D[1], val2);
        // pin_write(&D[2], 0.5);

        if (timer_flag(&timer4)) {
            timer_lower(&timer4);

            switch(state){
                case WAITING:
                    led_on(&led1);
                    led_off(&led2);
                    state = SENDING;
                    oc_pwm(&oc3, &D[3], NULL, 40E3, 32000);
                    timer_setPeriod(&timer4, 500E-6);
                    timer_start(&timer4);
                    break;

                case SENDING:
                    led_on(&led2);
                    led_off(&led1);
                    state = WAITING;
                    oc_pwm(&oc3, &D[3], NULL, 40E3, 0);
                    timer_setPeriod(&timer4, 0.05);
                    timer_start(&timer4);
                    break;
            }
        }
    }
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
            temp.w = val1;
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

