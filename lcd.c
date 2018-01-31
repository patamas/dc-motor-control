#define F_CPU 16000000UL

#include "lcd.h"

void lcd_send(unsigned char cd)
{
	cli();
	
	PORTB |= (1<<PORTB_EN);
	PORTD |= (cd & 0xF0);
	PORTB &= ~(1<<PORTB_EN);
	PORTD &= ~(0b11110000);

    asm volatile("nop"::);

	PORTB |= (1<<PORTB_EN);
	PORTD |= (cd & 0x0F)<<4;
	PORTB &= ~(1<<PORTB_EN);
	PORTD &= ~(0b11110000);
	
	sei();
	
	_delay_us(50);
}

void lcd_set(unsigned char t)
{
	PORTB &= ~(1<<PORTB_RS);
	lcd_send(t);
}

void lcd_putc(unsigned int c)
{
/*	if(c == 195)
	{	return;
		}

	if(c == 197)
	{	return;
		}

	if(c == 161)		//á
	{	c = 160;
		}

	else if(c == 169)	//é
	{	c = 130;
		}

	else if(c == 173)	//í
	{	c = 161;
		}

	else if(c == 179)	//ó
	{	c = 162;
		}

	else if(c == 186)	//ú
	{	c = 163;
		}

	else if(c == 188)	//ü
	{	c = 129;
		}

	else if(c == 182) 	//ö
	{	c = 148;
		}

	else if(c == 177)	//ű
	{	c = 150;
		}

	else if(c == 145)	//ő
	{	c = 147;
		}
		*/
	PORTB |= (1<<PORTB_RS);
	lcd_send(c);
}

void lcd_putn(unsigned char n)
{
	PORTB |= (1<<PORTB_RS);
	lcd_send(n + 48);
}

void lcd_puts(char *s)
{
	while(*s)	{	lcd_putc(*s++);}
}

void lcd_xy(unsigned char x, unsigned char y)
{
	x += 0x80;

	switch(y)
	{	case 0:
		{	lcd_set( x );
			break;
			}
		case 1:
		{	lcd_set((x + 0x40));
			break;
			}
		}
}