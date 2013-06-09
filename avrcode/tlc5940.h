#ifndef _TLC5940_H_
#define _TLC5940_H_

#define SBI(x,y) (x |= (1<<y)); 				/* set bit y in byte x */ 
#define CBI(x,y) (x &= (~(1<<y))); 				/* clear bit y in byte x */ 

#define SIN_DDR DDRB
#define SCL_DDR DDRB
#define XLAT_DDR DDRD
#define BLANK_DDR DDRD
#define VPROG_DDR DDRD
#define DCPROG_DDR DDRD

#define SIN_PORT PORTB
#define SCL_PORT PORTB
#define XLAT_PORT PORTD
#define BLANK_PORT PORTD
#define VPROG_PORT PORTD
#define DCPROG_PORT PORTD

#define SIN PINB3
#define SCL PINB5
#define XLAT PIND5
#define BLANK PIND6
#define VPROG PIND4
#define DCPROG PIND7

//---------------------------------------------------------------
// colour selector
#define COLOUR_DDR DDRC
#define COLOUR_PORT PORTC

#define COLOUR_GREEN PINC0
#define COLOUR_RED PINC1
#define COLOUR_BLUE PINC2

//---------------------------------------------------------------


void TLC_Init();
void TLC_Activate();
void TLC_Deactivate();
void TLC_GS();
void TLC_DC();
void TLC_Send_GS(uint8_t *data);
void TLC_Send_GS_Value(uint8_t data);
void TLC_Send_DC_Value(uint8_t data);

//---------------------------------------------------------------

extern volatile uint8_t ready_to_send;


#endif
