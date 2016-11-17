/*
 * serial_IN.h
 *
 * Created: 16-11-2016 19:46:01
 *  Author: Cyriel
 */ 


#ifndef SERIAL_IN_H_
#define SERIAL_IN_H_

#include <avr/io.h>
#include <avr/interrupt.h>
 
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
 
#define F_CPU   16000000
#define BUAD    9600
#define BRC     ((F_CPU/16/BUAD) - 1)
 
#define RX_BUFFER_SIZE  514
 
char rxBuffer[RX_BUFFER_SIZE];

uint8_t rxReadPos = 0;
uint8_t rxWritePos = 0;
 
char getChar(void);
char peekChar(void);
 
char peekChar(void)
{
    char ret = '\0';
     
    if(rxReadPos != rxWritePos)
    {
        ret = rxBuffer[rxReadPos];
    }
    return ret;
}
 
char getChar(void)
{
    char ret = '\0';
     
    if(rxReadPos != rxWritePos)
    {
        ret = rxBuffer[rxReadPos];
         
        rxReadPos++;
         
        if(rxReadPos >= RX_BUFFER_SIZE)
        {
            rxReadPos = 0;
        }
    }
     
    return ret;
}
 
ISR(USART_RX_vect)
{
    rxBuffer[rxWritePos] = UDR0;
     
    rxWritePos++;
     
    if(rxWritePos >= RX_BUFFER_SIZE)
    {
        rxWritePos = 0;
    }
}

#endif /* SERIAL_IN_H_ */