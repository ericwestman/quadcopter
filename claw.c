#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "usb.h"
#include "pin.h"
#include "uart.h"
#include "ui.h"
#include "timer.h"
#include "oc.h"
#include <time.h>
#include <stdio.h>
#include <math.h>

#define HELLO           0   // Vendor request that prints "Hello World!"
#define SET_VALS        1   // Vendor request that receives 2 unsigned integer values
#define GET_VALS        2   // Vendor request that returns 2 unsigned integer values
#define PRINT_VALS      3   // Vendor request that prints 2 unsigned integer values 

#define THRESHOLD       24000
#define WAITING         0 
#define SENDING         1
#define VBUS            U1OTGSTATbits.SESVD

uint16_t val1, val2;
int estop = 0;
int counter = 0;
int buttonCounter = 0;
uint16_t D9 = 0;
int pinRead = -1;

int16_t main(void) {

    InitUSB(); // Initialize USB

    while (USB_USWSTAT!=CONFIG_STATE && VBUS) {     //while the peripheral plugged in but not configured...
        ServiceUSB();                               // ...service USB requests
    }

    init_clock();
    init_ui();
    init_uart();
    init_timer();
    init_pin();
    init_oc();

    // Set up timer for LED flashing
    timer_setPeriod(&timer1, 1);
    timer_start(&timer1);

    // Set up digital pins
    pin_digitalIn(&D[0]);
    pin_digitalOut(&D[2]);
    pin_digitalOut(&D[3]);
    pin_digitalOut(&D[4]);
    pin_digitalOut(&D[5]);
    pin_digitalOut(&D[6]);
    pin_digitalOut(&D[7]);
    pin_digitalOut(&D[8]);
    pin_digitalOut(&D[9]);
    pin_digitalIn(&D[10]);

    pin_analogIn(&A[0]);

    pin_set(&D[2]);
    pin_clear(&D[3]);
    pin_set(&D[4]);
    pin_clear(&D[6]);
    pin_clear(&D[7]);
    pin_set(&D[8]);
    pin_set(&D[9]);

    // Set up the PWM signals
    oc_pwm(&oc1, &D[5], NULL, 20E3, 32768); //or 65536

    // Set up estop
    estop = 0;

    // Set up OCInterrupt
    _OC1IE = 1;
    _TRISF7 = 1;
    
    while (1) {
        ServiceUSB(); // service any pending USB requests
        
        if (pinRead >= 0 && !estop) {
            pinRead = -1;
            if (pin_read(&A[0]) < THRESHOLD) {
                // counter ++;
                if (counter >= 500) {
                    led_on(&led2);
                    // estop = 1;
                }
            }
            else if (!estop) {
                counter = 0;
            }
            D9 = pin_read(&D[10]);
            if (D9) {
                buttonCounter ++;
                led_on(&led3);
                if (buttonCounter >= 500){
                    estop = 1;
                }
            }
            else if (!estop) {
                led_off(&led3);
                buttonCounter = 0;
            }
            if (estop){
                pin_write(&D[5], 0);
            }
        }
        
        // Blinky light
        if (timer_flag(&timer1)) {
            timer_lower(&timer1);
            led_toggle(&led1);
        }
    }
}

void __attribute__((interrupt, auto_psv)) _OC1Interrupt(void){
    IFS0bits.OC1IF = 0; 
    pinRead = 0;
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
            temp.w = pin_read(&A[0]);
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];

            // temp.w = pin_read(&A[1]);
            temp.w = counter;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];

            // temp.w = pin_read(&A[2]);
            temp.w = D9;
            BD[EP0IN].address[4] = temp.b[0];
            BD[EP0IN].address[5] = temp.b[1];
            
            temp.w = buttonCounter;
            BD[EP0IN].address[6] = temp.b[0];
            BD[EP0IN].address[7] = temp.b[1];
            
            BD[EP0IN].bytecount = 8;    // set EP0 IN byte count to 4
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