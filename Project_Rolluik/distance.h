/*
 * distance.h
 *
 * Created: 13-11-2016 03:42:27
 *  Author: Cyriel
 */ 


#ifndef DISTANCE_H_
#define DISTANCE_H_

#define HIGH 0x1
#define LOW 0x0

#define BEGIN 0x1
#define END 0x0

void init_sensor_ports(void);
void init_timer(void);
void init_ext_int(void);
void sendCommand(uint8_t value);
void write(uint8_t pin, uint8_t val);
void shiftOut (uint8_t val);
int calc_cm(void);

#endif /* DISTANCE_H_ */