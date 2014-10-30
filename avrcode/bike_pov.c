#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "bike_pov.h"
#include "tlc5940.h"
#include "data.h"

volatile int8_t cycles=70;			//number of repetition of each line, will be controlled by sensor feedback
volatile uint8_t matrix_step=0;			//current matrix step
volatile uint8_t sensor_prev=1, sensor=1;	//for detecting the edge
volatile uint8_t spins=0;			//for the feedback loop

/* Note to animations:
 * There is no syncronisation between multiple spokePovs, so be aware that they
 * display different content in case they miss some wheel turns.
 */
#if defined frames && defined turns_per_frame
volatile uint8_t turns_left=turns_per_frame;
#endif
volatile uint8_t current_frame = 0;

int main(void) {
	uint8_t colour=0, number_cycles=0;

	//CTC mode
	TCCR0A |= (1<<WGM01)|(0<<WGM00);
	//prescaler by 8
	TCCR0B |= (0<<CS02)|(1<<CS01)|(0<<CS00);
	//interrupt with 10kHz frequency
	OCR0A = 30;
	//enable the compare interrupt
	TIMSK0 |= (1<<OCIE0A);
	//enable the timer0
	PRR &= ~(1<<PRTIM0);

	//Init the TLC5940
	CBI(SENSOR_DDR,SENSOR);

	TLC_Init();
	TLC_Send_GS_Value(0xFF);
	CBI(COLOUR_PORT, COLOUR_BLUE);
	SBI(COLOUR_PORT, COLOUR_GREEN);
	CBI(COLOUR_PORT, COLOUR_RED);
	TLC_Activate();
	TCNT1 = 0x00; //reset GSclk counter
	sei(); //global interrupts enable


	while(1) {
		if(ready_to_send){ //this is done during a GScycle, starting right after Blank

			//turn off all colours
			COLOUR_PORT &= ~((1<<COLOUR_BLUE)|(1<<COLOUR_RED)|(1<<COLOUR_GREEN));

			switch(colour) {
				case 0:	//lighting up the BLUES, while sending the REDS
					SBI(COLOUR_PORT, COLOUR_BLUE);
					CBI(BLANK_PORT, BLANK);
					TLC_Send_GS(green[matrix_step+current_frame*wheel_divisions]);
					break;
				case 1: //lighting up the REDS, while sending the GREENS
					SBI(COLOUR_PORT, COLOUR_RED);
					CBI(BLANK_PORT, BLANK);
					TLC_Send_GS(red[matrix_step+current_frame*wheel_divisions]);
					break;
				case 2: //lighting up the GREENS, while sending the BLUES
					SBI(COLOUR_PORT, COLOUR_GREEN);
					CBI(BLANK_PORT, BLANK);
					TLC_Send_GS(blue[matrix_step+current_frame*wheel_divisions]);
					break;
			}
			ready_to_send = 0; //clearing GSclock flag

			colour++;
			if(colour == 3) {
				colour = 0;
				number_cycles++;
				if(number_cycles >= cycles) {
					number_cycles = 0;
					matrix_step++;

					if(matrix_step >= wheel_divisions)  {
						spins++;
						matrix_step = 0;
#if defined frames && defined turns_per_frame	// if no animation, then current_frame will always be 0
						turns_left--;
						if (turns_left <=0)
						{
							turns_left =
								turns_per_frame;
							current_frame =
								(current_frame+1)%frames;
						}
#endif
					}
				}
			}
		}
	}
	return 0;
}


/**************************************************************
*	Control loop call
**************************************************************/
ISR (TIMER0_COMPA_vect){
	//detect rising edge when the magnet is detected
	sensor = (SENSOR_PIN & (1<<SENSOR));
	if(sensor && !sensor_prev) {
		if(spins>0) { //if the system has already pass through the entire matrix, it is displaying too fast!
			cycles +=  matrix_step/6;
			spins=0;
		} else { //if the system hasn't yet pass through the entire matrix, it is displaying too slow!
			cycles -= (wheel_divisions - matrix_step)/6;
		}
		if(cycles<0) cycles = 1;
		matrix_step=0;
	}
	sensor_prev = sensor;
}
