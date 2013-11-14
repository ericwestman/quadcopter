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

#define THRESHOLD 300
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
        update_servos();
        send_or_recive();
        // if(started == 0){
        //     timer_start(&timer5);
        //     started = 1;
        // }
        // if (timer_flag(&timer4)) {
        //     timer_lower(&timer4);
        //     led_toggle(&led3);
        //     printf("%i\n", timer_read(&timer4));
        // }
    }
}

void init(void){

    init_clock();
    init_ui();
    init_uart();
    init_timer();
    init_pin();
    init_oc();

    pin_digitalIn(&D[3]);
    oc_servo(&oc1, &D[0], &timer1, 20E-3, 6E-4, 2.2E-3, 0);
    oc_servo(&oc2, &D[1], &timer2, 20E-3, 6E-4, 2.2E-3, 0);
    oc_pwm(&oc3, &D[2], NULL, 40E3, 32000);
    timer_setPeriod(&timer3, 0.1);
    timer_start(&timer3);
    timer_setPeriod(&timer4, 0.020);
    timer_start(&timer4);

    led_on(&led1);
    val1 = 5;
    val2 = 2;

    InitUSB();
    printf("\n\nHello world\n");

}

void send_or_recive(void){
    switch(state){
        case SENDING:
            if( timer_flag(&timer3)){ //if we should stop sending
                state = WAITING;
                led_on(&led2);
                led_off(&led1);
                oc_pwm(&oc3, &D[2], NULL, 40E3, 0);
                timer_setPeriod(&timer4, 0.05);
                timer_start(&timer4);
                temp_dist = 0;
                printf("\n");                    
            }

            break;
        case WAITING:
            if(pin_read(&D[3]) == 0 && temp_dist == 0){
                temp_dist = timer_read(&timer4);
                if(temp_dist < THRESHOLD){
                    temp_dist = 0;
                }
                else{
                    last_dist = temp_dist;
                    printf("%i\n", temp_dist);                    
                }
            }
            if( timer_flag(&timer4)){ //if we should stop waiting
                led_on(&led1);
                led_off(&led2);
                state = SENDING;
                oc_pwm(&oc3, &D[2], NULL, 40E3, 32000);
                timer_setPeriod(&timer3, 1000E-6);
                timer_start(&timer3);
            }
            break;
    }
}

void update_servos(void){
    pin_write(&D[0], val1);
    pin_write(&D[1], val2);
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

