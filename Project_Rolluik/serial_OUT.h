/*
 * serial_OUT.h
 *
 * Created: 11-11-2016 07:03:54
 *  Author: Cyriel
 */ 


#ifndef SERIAL_OUT_H_
#define SERIAL_OUT_H_

// Serial comm
#define F_CPU   16000000
#define BUAD    9600
#define BRC     ((F_CPU/16/BUAD) - 1)
#define TX_BUFFER_SIZE  1024

// The array of tasks
sTask SCH_tasks_G[SCH_MAX_TASKS];
char serialBuffer[TX_BUFFER_SIZE];


uint8_t serialReadPos = 0;
uint8_t serialWritePos = 0;


void appendSerial(char c);
void serialWrite(char  c[]);


void appendSerial(char c)
{
	serialBuffer[serialWritePos] = c;
	serialWritePos++;
	
	if(serialWritePos >= TX_BUFFER_SIZE)
	{
		serialWritePos = 0;
	}
}

void serialWrite(char c[])
{
	for(uint8_t i = 0; i < strlen(c); i++)
	{
		appendSerial(c[i]);
	}
	
	if(UCSR0A & (1 << UDRE0))
	{
		UDR0 = 0;
	}
}

ISR(USART_TX_vect)
{
	if(serialReadPos != serialWritePos)
	{
		UDR0 = serialBuffer[serialReadPos];
		serialReadPos++;
		
		if(serialReadPos >= TX_BUFFER_SIZE)
		{
			serialReadPos++;
		}
	}
}

#endif /* SERIAL_OUT_H_ */