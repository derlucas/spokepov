#ifndef _BIKE_POV_H_

#define SBI(x,y) (x |= (1<<y)); 				/* set bit y in byte x */ 
#define CBI(x,y) (x &= (~(1<<y))); 				/* clear bit y in byte x */ 

#define SENSOR_PORT PORTB
#define SENSOR_PIN PINB
#define SENSOR_DDR DDRB
#define SENSOR PINB1

#endif //_BIKE_POV_H_
