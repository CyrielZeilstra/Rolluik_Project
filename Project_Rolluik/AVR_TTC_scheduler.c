#include "AVR_TTC_scheduler.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>
#include <serial_OUT.h>
#include <serial_IN.h>
#include <distance.h>
/*------------------------------------------------------------------*-

  SCH_Dispatch_Tasks()

  This is the 'dispatcher' function.  When a task (function)
  is due to run, SCH_Dispatch_Tasks() will run it.
  This function must be called (repeatedly) from the main loop.

-*------------------------------------------------------------------*/

void SCH_Dispatch_Tasks(void)
{
   unsigned char Index;

   // Dispatches (runs) the next task (if one is ready)
   for(Index = 0; Index < SCH_MAX_TASKS; Index++)
   {
      if((SCH_tasks_G[Index].RunMe > 0) && (SCH_tasks_G[Index].pTask != 0))
      {
         (*SCH_tasks_G[Index].pTask)();  // Run the task
         SCH_tasks_G[Index].RunMe -= 1;   // Reset / reduce RunMe flag

         // Periodic tasks will automatically run again
         // - if this is a 'one shot' task, remove it from the array
         if(SCH_tasks_G[Index].Period == 0)
         {
            SCH_Delete_Task(Index);
         }
      }
   }
}

/*------------------------------------------------------------------*-

  SCH_Add_Task()

  Causes a task (function) to be executed at regular intervals 
  or after a user-defined delay

  pFunction - The name of the function which is to be scheduled.
              NOTE: All scheduled functions must be 'void, void' -
              that is, they must take no parameters, and have 
              a void return type. 
                   
  DELAY     - The interval (TICKS) before the task is first executed

  PERIOD    - If 'PERIOD' is 0, the function is only called once,
              at the time determined by 'DELAY'.  If PERIOD is non-zero,
              then the function is called repeatedly at an interval
              determined by the value of PERIOD (see below for examples
              which should help clarify this).


  RETURN VALUE:  

  Returns the position in the task array at which the task has been 
  added.  If the return value is SCH_MAX_TASKS then the task could 
  not be added to the array (there was insufficient space).  If the
  return value is < SCH_MAX_TASKS, then the task was added 
  successfully.  

  Note: this return value may be required, if a task is
  to be subsequently deleted - see SCH_Delete_Task().

  EXAMPLES:

  Task_ID = SCH_Add_Task(Do_X,1000,0);
  Causes the function Do_X() to be executed once after 1000 sch ticks.            

  Task_ID = SCH_Add_Task(Do_X,0,1000);
  Causes the function Do_X() to be executed regularly, every 1000 sch ticks.            

  Task_ID = SCH_Add_Task(Do_X,300,1000);
  Causes the function Do_X() to be executed regularly, every 1000 ticks.
  Task will be first executed at T = 300 ticks, then 1300, 2300, etc.            
 
-*------------------------------------------------------------------*/

unsigned char SCH_Add_Task(void (*pFunction)(), const unsigned int DELAY, const unsigned int PERIOD)
{
   unsigned char Index = 0;

   // First find a gap in the array (if there is one)
   while((SCH_tasks_G[Index].pTask != 0) && (Index < SCH_MAX_TASKS))
   {
      Index++;
   }

   // Have we reached the end of the list?   
   if(Index == SCH_MAX_TASKS)
   {
      // Task list is full, return an error code
      return SCH_MAX_TASKS;  
   }

   // If we're here, there is a space in the task array
   SCH_tasks_G[Index].pTask = pFunction;
   SCH_tasks_G[Index].Delay =DELAY;
   SCH_tasks_G[Index].Period = PERIOD;
   SCH_tasks_G[Index].RunMe = 0;

   // return position of task (to allow later deletion)
   return Index;
}

/*------------------------------------------------------------------*-

  SCH_Delete_Task()

  Removes a task from the scheduler.  Note that this does
  *not* delete the associated function from memory: 
  it simply means that it is no longer called by the scheduler. 
 
  TASK_INDEX - The task index.  Provided by SCH_Add_Task(). 

  RETURN VALUE:  RETURN_ERROR or RETURN_NORMAL

-*------------------------------------------------------------------*/

unsigned char SCH_Delete_Task(const unsigned char TASK_INDEX)
{
   // Return_code can be used for error reporting, NOT USED HERE THOUGH!
   unsigned char Return_code = 0;

   SCH_tasks_G[TASK_INDEX].pTask = 0;
   SCH_tasks_G[TASK_INDEX].Delay = 0;
   SCH_tasks_G[TASK_INDEX].Period = 0;
   SCH_tasks_G[TASK_INDEX].RunMe = 0;

   return Return_code;
}

/*------------------------------------------------------------------*-

  SCH_Init_T1()

  Scheduler initialisation function.  Prepares scheduler
  data structures and sets up timer interrupts at required rate.
  You must call this function before using the scheduler.  

-*------------------------------------------------------------------*/

void SCH_Init_T1(void)
{
   unsigned char i;

   for(i = 0; i < SCH_MAX_TASKS; i++)
   {
      SCH_Delete_Task(i);
   }

   // Set up Timer 1
   // Values for 1ms and 10ms ticks are provided for various crystals

   // Hier moet de timer periode worden aangepast ....!
   OCR1A = (uint16_t)625;   		     // 10ms = (256/16.000.000) * 625
   TCCR1B = (1 << CS12) | (1 << WGM12);  // prescale op 64, top counter = value OCR1A (CTC mode)
   TIMSK1 = 1 << OCIE1A;   		     // Timer 1 Output Compare A Match Interrupt Enable
}

/*------------------------------------------------------------------*-

  SCH_Start()

  Starts the scheduler, by enabling interrupts.

  NOTE: Usually called after all regular tasks are added,
  to keep the tasks synchronised.

  NOTE: ONLY THE SCHEDULER INTERRUPT SHOULD BE ENABLED!!! 
 
-*------------------------------------------------------------------*/

void SCH_Start(void)
{
      sei();
}

/*------------------------------------------------------------------*-

  SCH_Update

  This is the scheduler ISR.  It is called at a rate 
  determined by the timer settings in SCH_Init_T1().

-*------------------------------------------------------------------*/

ISR(TIMER1_COMPA_vect)
{
   unsigned char Index;
   for(Index = 0; Index < SCH_MAX_TASKS; Index++)
   {
      // Check if there is a task at this location
      if(SCH_tasks_G[Index].pTask)
      {
         if(SCH_tasks_G[Index].Delay == 0)
         {
            // The task is due to run, Inc. the 'RunMe' flag
            SCH_tasks_G[Index].RunMe += 1;

            if(SCH_tasks_G[Index].Period)
            {
               // Schedule periodic tasks to run again
               SCH_tasks_G[Index].Delay = SCH_tasks_G[Index].Period;
               SCH_tasks_G[Index].Delay -= 1;
            }
         }
         else
         {
            // Not yet ready to run: just decrement the delay
            SCH_tasks_G[Index].Delay -= 1;
         }
      }
   }
}

// ------------------------------------------------------------------

// Command
// C:\avrdude\avrdude.exe

// Argument
// "-C "C:\avrdude\avrdude.conf" -p atmega328P -c arduino -P COM3 -b 115200 -U flash:w:"$(ProjectDir)Debug\Project_Rolluik.hex":i"

void init_led(){
	// set pin pins of PORTB for output LED
	DDRB |= _BV(DDB0); // Rood
	DDRB |= _BV(DDB1); // Geel
	DDRB |= _BV(DDB2); // Groen
}

void led_test()
{
	// Rood laten branden
	PORTB |= _BV(PORTB0);// Rood
	PORTB &= ~_BV(PORTB1); // Geel
	PORTB &= ~_BV(PORTB2); // Groen
	_delay_ms(10000);
	
	// Geel laten branden
	PORTB &= ~_BV(PORTB0);// Rood
	PORTB |= _BV(PORTB1); // Geel
	PORTB &= ~_BV(PORTB2); // Groen
		_delay_ms(10000);
	// Groen laten branden
	PORTB &= ~_BV(PORTB0);// Rood
	PORTB &= ~_BV(PORTB1); // Geel
	PORTB |= _BV(PORTB2); // Groen
	_delay_ms(10000);
	// alles uit
	PORTB &= ~_BV(PORTB0); // Rood
	PORTB &= ~_BV(PORTB1); // Geel
	PORTB &= ~_BV(PORTB2); // Groen
}

void init_serial_out(){
	UBRR0H = (BRC >> 8);
	UBRR0L =  BRC;
	
	UCSR0B = (1 << TXEN0)  | (1 << TXCIE0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	
	sei();
}

void init_serial_in(){
    UBRR0H = (BRC >> 8);
    UBRR0L =  BRC;
    
    UCSR0B = (1 << RXEN0)  | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    
    sei();
}

void InitADC()
{
 // Select Vref=AVcc
 ADMUX |= (1<<REFS0);
 //set prescaller to 128 and enable ADC 
 ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);    
}

uint16_t ReadADC(uint8_t ADCchannel)
{
 //select ADC channel with safety mask
 ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
 //single conversion mode
 ADCSRA |= (1<<ADSC);
 // wait until ADC conversion is complete
 while( ADCSRA & (1<<ADSC) );
 return ADC;
}

void read_temp_senor(){
	init_serial_out();
	int reading = ReadADC(0);
	
	char reader[50];
	sprintf(reader, "%i", reading);
	
	serialWrite("|C");
    serialWrite(reader);
	serialWrite("|\n");
}

void read_light_sensor(){
	init_serial_out();
	int reading = ReadADC(1);
	
	char reader[50];
	sprintf(reader, "%i", reading);
	
	serialWrite("|L");
    serialWrite(reader);
	serialWrite("|\n");
}

void blink_yellow(){
	PORTB &= ~_BV(PORTB2); // Groen
	PORTB &= ~_BV(PORTB0);// Rood
	
	PORTB |= _BV(PORTB1); // Geel Aan
	_delay_ms(10000);
	PORTB &= ~_BV(PORTB1); // Geel Uit
	_delay_ms(10000);
	PORTB |= _BV(PORTB1); // Geel Aan
}

void set_leds(){
	int reading = calc_cm();
//	max_rolluik_lenght
//	min_rolluik_lenght
	if (reading < 15){
	// laat groen branden
	PORTB &= ~_BV(PORTB0);// Rood
	PORTB &= ~_BV(PORTB1); // Geel
	PORTB |= _BV(PORTB2); // Groen
	}
	else if (reading > 25){	
	// Rood laten branden
	PORTB |= _BV(PORTB0);// Rood
	PORTB &= ~_BV(PORTB1); // Geel
	PORTB &= ~_BV(PORTB2); // Groen
	}
	else {
	blink_yellow();
	}
}

void read_python_input(){
	init_serial_in();
	char c = getChar();
	
	init_serial_out();

	char reader[50];
	reader[0] = c;
	serialWrite("|KK");
    serialWrite(reader);
	serialWrite("|\n");
	
	if (c == '6'){
		PORTB |= _BV(PORTB0);// Rood
		PORTB &= ~_BV(PORTB1); // Geel
		PORTB &= ~_BV(PORTB2); // Groen
	}
}

int main()
{
	 // Initialize components
	 SCH_Init_T1(); 
	 init_led();
	 InitADC();	 
	 init_sensor_ports();
	 init_timer();
	 init_ext_int();
	 
	 
	// Add tasks
	
	// Task_ID = SCH_Add_Task(Do_X,300,1000);
	// Causes the function Do_X() to be executed regularly, every 1000 ticks.
	// Task will be first executed at T = 300 ticks, then 1300, 2300, etc.       
	 
	SCH_Add_Task(set_leds,100,500);
	SCH_Add_Task(read_temp_senor,200,750);
	SCH_Add_Task(read_light_sensor,100,500);
//	SCH_Add_Task(read_python_input,5,100);
	 
	 // Start Scheduler
	 SCH_Start();
	
   while(1) 
      {
	    SCH_Dispatch_Tasks();
      } 	
	return 0;
}


