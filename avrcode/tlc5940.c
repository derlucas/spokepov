#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "tlc5940.h"

volatile uint8_t ready_to_send = 0;

/**************************************************
* Reset the GScounter interrupt
***************************************************/
ISR (TIMER1_COMPA_vect){
	SBI(BLANK_PORT, BLANK);
	SBI(XLAT_PORT, XLAT);
	ready_to_send = 1;
	CBI(XLAT_PORT, XLAT);
}

inline void TLC_Activate() {
	CBI(BLANK_PORT,BLANK);
}

inline void TLC_Deactivate() {
	SBI(BLANK_PORT,BLANK);
}

inline void TLC_DC() {
	SBI(VPROG_PORT,VPROG);
}

inline void TLC_GS() {
	CBI(VPROG_PORT,VPROG);
}

void TLC_Send_GS(const uint8_t *data) {
	uint8_t i;
	uint16_t byte1, byte2;

	for(i=0; i<8; i++) {
		byte1 = pgm_read_byte(data++) << 4;
		byte2 = pgm_read_byte(data++) << 4;

		SPDR = (byte1 >> 4) & 0xff;
		while (!(SPSR & (1<<SPIF)));

		SPDR = ( (byte1 << 4) & 0xf0 ) | ((byte2 >> 8) & 0x0f );
		while (!(SPSR & (1<<SPIF)));

		SPDR = byte2 & 0xff;
		while (!(SPSR & (1<<SPIF)));
	}
}

/*
void TLC_Send_GS(uint8_t *data) {
	uint8_t i;
	for(i=0; i<24; i++){
		SPDR = pgm_read_byte(data);	//start sending
		while (!(SPSR & (1<<SPIF))); //wait until not sent	
		data++;
	}
}
*/

/**************************************************
* Send single GS value
* ---------------------
* This sends a single value for all the leds
***************************************************/
void TLC_Send_GS_Value(uint8_t data) {
	uint8_t i;
	for(i=0; i<24; i++){
		SPDR = data;	//start sending
		while (!(SPSR & (1<<SPIF))); //wait until not sent	
	}
}

/**************************************************
* Send single DC value
* ---------------------
* First, set the TLC in DC mode
* This sends a single value for all the leds
***************************************************/
void TLC_Send_DC_Value(uint8_t data) {
	uint8_t i;

	for(i=0; i<12; i++){
		SPDR = data;	//start sending
		while(!(SPIF & SPIF)); //wait until not sent
	}
	SBI(XLAT_PORT,XLAT); //bump the XLAT
	CBI(XLAT_PORT,XLAT);
}


void TLC_Init(){
	SBI(XLAT_DDR,XLAT);
	SBI(BLANK_DDR,BLANK);
	SBI(VPROG_DDR, VPROG);
	SBI(DCPROG_DDR, DCPROG);
	//
	CBI(XLAT_PORT,XLAT);
	SBI(BLANK_PORT,BLANK); // deactivate the tlc, blank as high
	CBI(VPROG_PORT, VPROG); //GS mode
	CBI(DCPROG_PORT, DCPROG); //dot correction from EEPROM (0x3F by default)
	//SBI(DCPROG_PORT, DCPROG); //dot correction from register

	// set the colour selector pins as output, and deactivate
	SBI(COLOUR_DDR, COLOUR_RED);
	SBI(COLOUR_DDR, COLOUR_GREEN);
	SBI(COLOUR_DDR, COLOUR_BLUE);
	CBI(COLOUR_PORT, COLOUR_RED);
	CBI(COLOUR_PORT, COLOUR_GREEN);
	CBI(COLOUR_PORT, COLOUR_BLUE);


//Set timer for GSclk
	//CTC with TOP value on OCR1A
	TCCR1A |= (0<<WGM11) | (0<<WGM10);
	TCCR1B |= (0<<WGM13) | (1<<WGM12);

	//set value for compare, 1 tic before 0xFFF/4094
	OCR1AH = 0x0F;
	OCR1AL = 0xFB;

	//enable the timer1 on PowerReductions Register
	PRR &= ~(1<<PRTIM1);

	//set clock source, no prescalling from cklio
	TCCR1B |= (0<<CS12)|(0<<CS11)|(1<<CS10);

	//set interrupt when compare with regA
	TIMSK1 |= (1<<OCIE1A);

//activate SPI in master mode to output data
	//set the MOSI,SCK and /SS pins as outputs
	SBI(SIN_DDR, SIN);
	SBI(SIN_DDR, SCL);
	SBI(SIN_DDR, PINB2); //!SS pin

	//enable the SPI on Power Reductions Register
	PRR &= ~(1<<PRSPI);

	//set clk 2x faster
	SPSR = (1<<SPI2X);

	//enable, set MSB first, set as master, set lowest clk divide
	SPCR = (1<<SPE)|(0<<DORD)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA) |(0<<SPR1)|(0<<SPR0);
}
