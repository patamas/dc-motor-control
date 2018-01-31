#include <stdint.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#define	PORTB_RS	0
#define	PORTB_EN	1
#define	N   7

//	lcd_RS	PORTB.0
//	lcd_EN	PORTB.1
//	lcd_D4	PORTD.4
//	lcd_D5	PORTD.5
//	lcd_D6	PORTD.6
//	lcd_D7	PORTD.7

#define lcd_home()          ( (PORTB &= ~(1<<PORTB_RS)), (lcd_send(0b00000010)), (_delay_ms(2)) )
#define lcd_clr()           ( (PORTB &= ~(1<<PORTB_RS)), (lcd_send(0b00000001)), (_delay_ms(2)) )
#define lcd_on()            ( (PORTB &= ~(1<<PORTB_RS)), (lcd_send(0b00001100)) )
#define lcd_off()      		( (PORTB &= ~(1<<PORTB_RS)), (lcd_send(0b00001000)) )
#define lcd_cursor_on()     ( (PORTB &= ~(1<<PORTB_RS)), (lcd_send(0b00001110)) )
#define lcd_cursor_off()    ( (PORTB &= ~(1<<PORTB_RS)), (lcd_send(0b00001100)) )

static const unsigned char lcd_custom[] =
{
	0b00000,

	0b10000,

	0b11000,

	0b11100,

	0b11110,

	0b11111
};

void lcd_send(unsigned char );
void lcd_set(unsigned char );
void lcd_putc(unsigned int );
void lcd_putn(unsigned char );
void lcd_puts(char *);
void lcd_xy(unsigned char , unsigned char );
void lcd_numberx(unsigned char , unsigned char );

static inline void lcd_init()
{
  	_delay_ms(50);          //watchdog unstable, use crystal

	PORTB |= (1<<PORTB_EN);
	PORTD |= 0b00110000;
	PORTB &= ~(1<<PORTB_EN);
	PORTD &= ~0b00110000;
	_delay_ms(5);

	PORTB |= (1<<PORTB_EN);
	PORTD |= 0b00110000;
	PORTB &= ~(1<<PORTB_EN);
	PORTD &= ~0b00110000;
	_delay_us(120);

	PORTB |= (1<<PORTB_EN);
	PORTD |= 0b00110000;
	PORTB &= ~(1<<PORTB_EN);
	PORTD &= ~0b00110000;
	_delay_us(50);

	PORTB |= (1<<PORTB_EN);
	PORTD |= 0b00100000;
	PORTB &= ~(1<<PORTB_EN);
	PORTD &= ~0b00100000;
	_delay_us(50);

	lcd_set(0b00101000);    //4bit mode, 2-lines, 5x8 size
	lcd_on();
	lcd_clr();
	lcd_set(0b00000110);    //increment, screen does not shift

	lcd_set(0x40);			//set custom characters
	unsigned char i, j, temp;
	for(i = 0; i < (sizeof(lcd_custom) / sizeof(unsigned char)); ++i)
	{	temp = lcd_custom[i];

		for(j = 8; j; --j)	{	lcd_putc(temp);}
		}
	lcd_home();
}
