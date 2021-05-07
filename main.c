/*
 * Prosjekt_HDM.c
 *
 * Created: 21/04/2021 14:48:03
 * Author : haako
 */ 

#define F_CPU 16000000UL
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "USART.h"


void ADC_Init(void)
{
	ADCSRA = (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
	ADMUX |= (1<<REFS0) | (1<<REFS1);		//0b1100 0000		0b1100 0001		C0  C1
	ADCSRA |= (1<<ADIE);
	ADCSRA |= (1<<ADEN);
	sei();	
	
	ADCSRA |= (1<<ADSC);
}

void initInterrupt0(void)
{
	EIMSK |= (1<<INT0);
	EICRA |= (1<<ISC00);
}


void initInterrupt1(void)
{
	EIMSK |= (1<<INT1);
	EICRA |= (1<<ISC10);
}


static inline void initTimer0(void)
{
	TCCR0A = (1<<WGM01) | (1<<WGM00);
	TCCR0B = (1<<CS00) | (1<<CS01);
	TCCR0A |= (1<<COM0A1);			// OCR0A (Grønn RGB) 
	TCCR0A |= (1<<COM0B1);			// OCR0B (Rød LED)
}

static inline void initTimer1(void)
{
	TCCR1A |= (1<<WGM10);
	TCCR1B |= (1<<WGM12);
	TCCR1B |= (1<<CS11);
	TCCR1A |= (1<<COM1A1);			// OCR1A (Blå RGB)
	TCCR1A |= (1<<COM1B1);			// OCR1B (Rød RGB)
}

int main(void)
{
	ADC_Init();
	initTimer0();
	initTimer1();
	initUSART();
	initInterrupt0();
	initInterrupt1();
	
	DDRB |= (1<<PINB0);			// Blå LED
	DDRB |= (1<<PINB1);			// Blå RGB
	DDRB |= (1<<PINB2);			// Rød RGB
	
	DDRD |= (1<<PIND4);			// Grønn LED
	DDRD |= (1<<PIND5);			// Rød LED
	DDRD |= (1<<PIND6);			// Grønn RGB
	
	PORTD |= (1<<PIND3);		// Knapp lys opp
	PORTD |= (1<<PIND2);		// Knapp lys ned
	
	OCR0B = 0;
	
	OCR0A = 0;
	OCR1A = 0;		
	OCR1B = 0;	

	while (1) 
	{
		char color;
						
		printString("Choose between these colors:\r\n");
		printString("Red: r, Green: g, Blue: b, Magneta: m, Cyan: c, Purple: p, Yellow: y, No color: space\r\n");
									
		color = receiveByte();
		
		switch(color)
		{
			case 'r':
											
				OCR1B = 255;
				OCR0A = 0;
				OCR1A = 0;
				break;
			
			case 'g':
						
				OCR1B = 0;
				OCR0A = 255;
				OCR1A = 0;
				break;
			
			case 'b':
									
				OCR1B = 0;
				OCR0A = 0;
				OCR1A = 255;
				break;
				
			case 'm':
				OCR1B = 255;
				OCR0A = 0;
				OCR1A = 255;
				break;
				
			case 'c':
				OCR1B = 0;
				OCR0A = 255;
				OCR1A = 255;
				break;
				
			case 'p':
				OCR1B = 255;
				OCR0A = 20;
				OCR1A = 147;
				break;
				
			case 'y':
				OCR1B = 255;
				OCR0A = 125;
				OCR1A = 0;
				break;
				
			case ' ':
				
				OCR1B = 0;
				OCR0A = 0;
				OCR1A = 0;
				break;
				
			default:
	
				break;
		}
	}
}

ISR(INT0_vect)
{
	if (bit_is_clear(PIND, PIND2))
	{
		if (OCR0B > 0)
		{
			OCR0B--;
		}
		_delay_ms(50);
	}
}

ISR(INT1_vect)
{
	if (bit_is_clear(PIND, PIND3))
	{
		if (OCR0B < 255)
		{
			OCR0B++;
		}
		_delay_ms(50);
	}
}
	
ISR(ADC_vect)
{

	uint8_t light = ADCL;
	uint8_t pot = ADCH<<8 | light;
	
	
	switch (ADMUX)
	{
		case 0xC0:
			if (light > 100)
			{
				PORTD &= ~(1<<PIND4); 
			}
			else
			{
				PORTD |= (1<<PIND4);	
			}
			ADMUX = 0xC1;
			break;
		case 0xC1: 
			
			if (pot > 128)
			{
				PORTB |= (1<<PINB0);
			}
			else
			{
				PORTB &= ~(1<<PINB0);
			}
			ADMUX = 0xC0;
			break;
		default:
			//Default code
			break;
	}
	ADCSRA |= (1<<ADSC); 
}

