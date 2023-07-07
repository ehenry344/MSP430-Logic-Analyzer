// Evan Henry

// Written : 5 / 29 / 2023
// Updated : 6 / 4 / 2023

// Description : Source code for crude 8 bit logic analyzer, using UART to transfer data to terminal

#include <msp430g2553.h>

// typedefs

typedef enum interrupt_direction {
    FALLING,
    RISING
} interrupt_direction;

// prototypes

void setup_uart_transmitter(void);
void setup_gpio(interrupt_direction);
void setup_service_timer(void); // services (janitors (cleans up)) anything that the system has updated with intentions of cleaning up

// definitions

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;

    setup_service_timer();
    setup_uart_transmitter();
    setup_gpio(RISING);

    __enable_interrupt();

    P1OUT &= ~BIT4; // setup light
    UCA0TXBUF = 'R';
}

void setup_uart_transmitter(void) {
    P1SEL |= BIT1 + BIT2;
    P1SEL2 |= BIT1 + BIT2;

    UCA0CTL1 |= UCSWRST;

    // 1200 Baud Setup
    UCA0CTL1 |= UCSSEL_2;

    UCA0BR0 = 0x68;
    UCA0BR1 = 0x00;
    UCA0MCTL = UCBRS0;

    UCA0CTL1 &= ~UCSWRST;
}

void setup_gpio(interrupt_direction i_d) {
    P1SEL &= ~BIT3 + ~BIT4;
    P2SEL = 0x00;

    // status lights

    P1DIR |= (BIT3 + BIT4);
    P1OUT |= (BIT3 + BIT4);
    // analyzer inputs

    P2DIR = 0x00;
    P2REN = 0xFF;

    if (i_d == RISING) {
        P2OUT = 0x00;
        P2IES = 0x00;
    } else {
        P2OUT = 0xFF;
        P2IES = 0xFF;
    }
    P2IFG = 0x00;
    P2IE |= 0xFF;
}

void setup_service_timer(void) {
    // DCO setup (reliably calibrated to 1 mega-hertz)
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // timer setup
    TACCR0 = 0;
    TACCTL0 |= CCIE;
    TA0CTL |= (TASSEL_2 + ID_3 + MC_0);
}

// interrupt servicing

#pragma vector = TIMER0_A0_VECTOR
__interrupt void T_A0_ISR(void) {
    if (!(P1OUT & BIT3)) {
        P1OUT |= BIT3;
    }
    TACCR0 = 0;
    TA0CTL |= MC_0;
}


#pragma vector = PORT2_VECTOR
__interrupt void P2_ISR(void) {
    // UART byte transmission
    UCA0TXBUF = P2IN;

    UCA0TXBUF = P2IN; // perform transmission

    TA0CTL |= MC_1; // enable up mode
    TACCR0  = 62499;

    P2IFG = 0x00;
}



