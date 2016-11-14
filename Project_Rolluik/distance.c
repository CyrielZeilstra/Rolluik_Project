/*
 * distance.c
 *
 * Created: 29-6-2016 14:44:43
 *  Author: jacob
 */ 

/* 
 * HC-SR04
 * trigger : uno 0 (PD0) out
 * echo    : uno 3 (PD3) INT1 in
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#define  F_CPU 16000000
#include <util/delay.h>
#include "distance.h"

volatile uint16_t gv_counter; // 16 bit
volatile uint8_t echo; // a flag

void init_sensor_ports(void)
{
    DDRD=0x01; // set port D0 as output, D3 as input
    PORTD = 0x00; // clear bit D0
	_delay_us(2);
}

void init_timer(void)
// prescale, no interrupt, counting up
{
    // prescaling : max time = 2^16/16E6 = 4.1 ms, 4.1 >> 2.3, so no prescaling required
    TCCR1A = 0;
    TCCR1B = _BV(CS10);
}

void init_ext_int(void)
{
    // any change triggers ext interrupt 1
    EICRA = (1 << ISC10);
    EIMSK = (1 << INT1);
}


void send_trigger(){
	echo = BEGIN; // set flag
	// start trigger puls lo -> hi
	PORTD |= _BV(0); // set bit D0
	_delay_us(12); // micro sec
	// stop trigger puls hi -> lo
	PORTD &=~ _BV(0); // clear bit D0
	_delay_ms(20); // milli sec, timer1 is read in ISR
}

int calc_cm()
{
	send_trigger();
    // counter 0 ... 65535, f = 16 MHz
    int micro_sec = gv_counter/16;
    // micro_sec 0..4095 cm 0..70
    return (micro_sec / 58.2);
}

ISR (INT1_vect)
{
    if (echo == BEGIN) {
        // set timer1 value to zero
        TCNT1 = 0;
        // clear flag
        echo = END;
    } else {
        // read value timer1
        gv_counter = TCNT1;
    }
}