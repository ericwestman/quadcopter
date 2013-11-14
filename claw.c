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
#include <math.h>

#define HELLO       0   // Vendor request that prints "Hello World!"
#define SET_VALS    1   // Vendor request that receives 2 unsigned integer values
#define GET_VALS    2   // Vendor request that returns 2 unsigned integer values
#define PRINT_VALS  3   // Vendor request that prints 2 unsigned integer values 

#define THRESHOLD 1000
#define WAITING 0 
#define SENDING 1

uint16_t val1, val2, state, temp_dist, last_dist, encoder_count, current_measure;
int estop = 0;
int counter = 0;
int pinRead = -1;
uint16_t OC_count = 0;

// void __attribute__((interrupt)) _CNInterrupt(void); 
// void __attribute__((interrupt)) _OC2Interrupt(void);
// void init(void);

int16_t main(void) {
    init();
    uint16_t started = 0;
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
    }

    while (1) {
        ServiceUSB(); // service any pending USB requests
        
        if (pinRead >= 0) {
            pinRead = -1;
            if (pin_read(&A[0]) < 20000) {
                counter ++;
                if (counter >= 100) {
                    estop = 1;
                }
            }
            else if (!estop) {
                counter = 0;
            }
            if (estop){
                oc_pwm(&oc1, &D[5], NULL, 20E3, pow(2,0));
            }
        }
                              
        if (timer_flag(&timer1)) {
            timer_lower(&timer1);
            led_toggle(&led1);
        }
    }
}

void __attribute__((interrupt, auto_psv)) _CNInterrupt(void){
    IFS1bits.CNIF = 0;
    pin_read(&D[0]);
    encoder_count ++;
}

void __attribute__((interrupt, auto_psv)) _OC1Interrupt(void){
    IFS0bits.OC1IF = 0; 
    pin_read(&D[10]);
    pinRead = 0;
    OC_count ++;
    led_toggle(&led3);
}

void init(void){

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
    
    pin_analogIn(&A[0]);

    pin_set(&D[2]);
    pin_clear(&D[3]);
    pin_set(&D[4]);
    pin_clear(&D[6]);
    pin_clear(&D[7]);
    pin_set(&D[8]);

    // Set up the PWM signals
    oc_pwm(&oc1, &D[5], NULL, 20E3, pow(2,15));
    // oc_pwm(&oc2, &D[10], NULL, 2E3, 32500);

    // Set up estop
    estop = 0;

    // Set up CNInterrupt
    // IEC1bits.CNIE = 1;
    // CNEN1bits.CN14IE = 1;
    // IFS1bits.CNIF = 0;

    // IPC4bits.CNIP2 = 1;
    // IPC4bits.CNIP2 = 1;
    // IPC4bits.CNIP2 = 1;

    // Set up OCInterrupt
    _OC1IE = 1;
    // IFS0bits.OC1IF = 0;

    // OC1CON1bits.OCM2 = 1;
    // OC1CON1bits.OCM1 = 1;
    // OC1CON1bits.OCM0 = 0;

    // IPC0bits.OC1IP2 = 0; 
    // IPC0bits.OC1IP1 = 0;
    // IPC0bits.OC1IP0 = 0;   


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
            temp.w = pin_read(&A[0]);
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];

            // temp.w = pin_read(&A[1]);
            temp.w = counter;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];

            // temp.w = pin_read(&A[2]);
            temp.w = OC_count;
            BD[EP0IN].address[4] = temp.b[0];
            BD[EP0IN].address[5] = temp.b[1];
            
            temp.w = encoder_count;
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

