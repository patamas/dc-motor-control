#define F_CPU 16000000UL

#include "main.h"
#include "lcd.h"

volatile unsigned char counter = 0, temp_counter1[4] = {0xFF}, ciklus = 0;
volatile unsigned int temp_timer1[4] = {0xFFFF};
volatile long i_temp = 0;

// visszameres		pipa
// EEPROM mentes	pipa
// vedelem 			pipa

ISR(INT0_vect)		// ez akkor következik be amikor a felfuttó él bejön az érzékelőtől az int0-es lábra
{
	TCCR1B = 0;

	temp_timer1[ciklus] = TCNT1;
	temp_counter1[ciklus] = counter;

	TCNT1 = 1;
	counter = 0;

	TCCR1B = 0b1;

	ciklus++;

	if(ciklus == 4)
	{	ciklus = 0;
		}
}

ISR(TIMER0_COMPA_vect)
{
	DDRB &= ~(1<<2);
}

ISR(TIMER0_COMPB_vect)
{
	DDRB |= (1<<2);
}

ISR(TIMER1_OVF_vect)
{
	++counter;

	if(counter == 0xFF)
	{	temp_counter1[0] = 255;
		temp_counter1[1] = 255;
		temp_counter1[2] = 255;
		temp_counter1[3] = 255;

		temp_timer1[0] = 0xFFFF;
		temp_timer1[1] = 0xFFFF;
		temp_timer1[2] = 0xFFFF;
		temp_timer1[3] = 0xFFFF;

		ciklus = 0;
		counter = 0;
		}
}

static inline void mcu_init()
{
	DIDR0 = 0b00001111;		//disable ad pin
	DDRB = 0b00001011;		//portb 0-1 output lcd, 3 555
	DDRD = 0b11111000;		//portd 4-7 output lcd, 3 buzzer
    DDRC = 0;				//AD pin input
//----------------------------------------------------------------------
	lcd_init();             //initialize lcd display
//----------------------------------------------------------------------
	TCCR2A = 0b10000001;	//Phase correct pwm mode, TOP = 0xFF
	//OCR2A = 0;			//pwm duty
	OCR2B = 31;				//buzzer 25% duty
//----------------------------------------------------------------------
	//TCCR1A = 0;			//Normal mode, TOP = 0xFFFF
	TIMSK1 = 0b1;			//overflow interrupt
//----------------------------------------------------------------------
	TCCR0A = 0b11;			//Fast pwm mode, TOP = OCR0A
	OCR0A = 77;				//4.992ms interrupt
	OCR0B = 25;				//fenyero pwm
	TIMSK0 = 0b110;			//OCR0A és OCR0B megszakítás
//----------------------------------------------------------------------
	EICRA = 0b0011;			//INT0 felfutó él
//----------------------------------------------------------------------
	PRR   = 0b10000110;		//nem használt perifériák tiltása
}

int main()
{
	mcu_init();															//mikrovezérlő lábak és az lcd kijelző inicializálása

	unsigned char i, l, x, z, a, p;										//ciklusváltozók definiálása
	unsigned int rpm_all;
	int	adc_val;

	eeprom_read_data(&rpm_all, &a);

	if(rpm_all > 9000 || (rpm_all % 250) != 0)							//eeprom kezdeti értékek ellenőrzése
	{	rpm_all = 0;
		}

	if(OCR0B > 77)
	{	OCR0B = 25;
		}

	sei();

	TCCR0B = 0b1101;													//1024	prescaler, start timer0

	for(i = 255, l = 59, x = 0, z = 0, p = 0; ; )
	{
		if(TIFR0 & 0b1)
		{	TIFR0 = 0b1;

			if(i >= 59)													//300ms display update time
			{	i = 255;												//kijelző frissítés
				}

			if(i == l)													//gombok újraengedélyezése
			{	l = 59;
				x = 0;
				}

			adc_val = adc_read(0);										//gomb érték ad

			if((adc_val < 896 || i == 255) && x == 0)					//kijelzés
			{
				if(adc_val < 896)										//gombok letiltása
				{	x = 1;
					l = i;
					}

				//lcd_xy(0, 0);

				if(adc_val < 64)										//right, jobbra léptet menübe
				{	//lcd_puts("right");

					++z;

					if(z == 3)
					{	z = 0;
						}

					lcd_clr();
					}
				else if(adc_val < 256)									//up, növeli az értéket
				{	//lcd_puts("up");

					if(z == 0)											//beállított rpm
					{	if(a)
						{	if(OCR2A < 255)
							{	OCR2A++;
								}
							}
						else
						{	if(rpm_all == 0)
							{	rpm_all = 1000;
								}
							else if(rpm_all < 9000)
							{	rpm_all += 250;
								}
							}
						}
					else if(z == 1)										//fényerő
					{	if(OCR0B < 77)
						{	OCR0B++;
							}
						}
					else if(z == 2)										//control type
					{	a = !a;
						}

					p = 0xFF;											//változás az értékekben, később mentés eepromba
					}
				else if(adc_val < 384)									//down, csökkenti az értéket
				{	//lcd_puts("down");

					if(z == 0)											//beállított rpm
					{	if(a)
						{	if(OCR2A > 0)
							{	OCR2A--;
								}
							}
						else
						{	if(rpm_all == 1000)
							{	rpm_all = 0;
								}
							else if(rpm_all > 1000)
							{	rpm_all -= 250;
								}
							}
						}
					else if(z == 1)										//fényerő
					{	if(OCR0B > 0)
						{	OCR0B--;
							}
						}
					else if(z == 2)										//control type
					{	a = !a;
						}

					p = 0xFF;											//változás az értékekben, később mentés eepromba
					}
				else if(adc_val < 576)									//left, balra léptet menübe
				{	//lcd_puts("left");

					if(z == 0)
					{	z = 3;
						}

					--z;

					lcd_clr();
					}
				else if(adc_val < 896)									//select, tovább a főciklusra
				{	//lcd_puts("select");

					z = 0;

					lcd_clr();

					break;
					}

				lcd_xy(0, 0);

				if(z == 0)												//főmenü
				{	lcd_xy(8, 0);

					if(a)												//kitöltés
					{	szamkiiras( ((OCR2A * 100) / 255), 3);
						lcd_putc('%');
						}
					else												//rpm
					{	szamkiiras(rpm_all, 5);
						lcd_puts("rpm");
						}
					}
				else if(z == 1)											//fényerő menü
				{	lcd_puts("fenyero: ");

					szamkiiras( ((OCR0B * 100) / 77), 3);
					lcd_putc('%');
					}
				else if(z == 2)											//control type menü
				{	if(a)
					{	lcd_puts("PWM control");
						}
					else
					{	lcd_puts("PI  control");
						}
					}
				}

			++i;
			}
		}

	unsigned long rpm_value;											//ciklusváltozók definiálása
	unsigned int rpm_akt, rpm_avg, v_value, i_value, v_avg, i_avg, i_akt;

	GTCCR = 0b10000011;		//prescaler reset

	TCNT0 = 0;				//reset timer0
	TCNT1 = 1;				//timer1 kezdeti érték
    TCCR1B = 0b1;			//1	prescaler, start timer1
	TCCR2B = 0b1;			//1 prescaler, start timer2

	EIMSK = 0b1;			//enable INT0 interrupt
	GTCCR = 0;				//timerek engedélyezése

	for(i = 255, l = 58, x = 2, rpm_value = 0, z = 0, rpm_avg = 0, v_value = 0, i_value = 0, v_avg = 0, i_avg = 0;;)
	{
		if(TIFR0 & 0b1)
		{	TIFR0 = 0b1;

//----------------------------------------------------------------------		rpm számolás
			cli();

			rpm_akt = ((F_CPU * 60) / (  ( ( ((unsigned long) temp_counter1[0] << 16) | temp_timer1[0] ) +
										   ( ((unsigned long) temp_counter1[1] << 16) | temp_timer1[1] ) +
										   ( ((unsigned long) temp_counter1[2] << 16) | temp_timer1[2] ) +
										   ( ((unsigned long) temp_counter1[3] << 16) | temp_timer1[3] ) )
								    / 4));

			sei();

//----------------------------------------------------------------------		átlagoláshoz értékek mentése
			rpm_value += rpm_akt;

			i_akt = adc_read(3);										//áram érték ad
			i_value += i_akt;

			v_value += adc_read(2) - adc_read(1);						//feszültség érték ad

//----------------------------------------------------------------------		hibakezelés és PI szabályozó
			if(!a)
			{	if((rpm_akt == 57 && i_temp >= 16384))
				{	fault_handler("RPM mero hiba!");

					rpm_value = 0;										//értékek nullázása
					v_value = 0;
					i_value = 0;
					i = 255;											//gombok tiltása
					l = 58;
					x = 2;												//túláram védelem tiltása
					z = 0;												//főmenü
					}
				else
				{	OCR2A = pi_control(rpm_all - rpm_akt);				// pi szabályozó
					}
				}

			if(i_akt > 920 && x < 2)									//11A
			{	fault_handler("Tularam_1 hiba!");

				rpm_value = 0;											//értékek nullázása
				v_value = 0;
				i_value = 0;
				i = 255;													//gombok tiltása
				l = 58;
				x = 2;													//túláram védelem tiltása
				z = 0;													//főmenü
				}

//----------------------------------------------------------------------		rpm, u , i átlagolás
			if(i >= 59)													//300ms display update time
			{	i = 255;												//kijelző frissítés

				rpm_avg = rpm_value / 60;
				rpm_value = 0;

				if(rpm_avg == 57)
				{	rpm_avg = 0;
					}

				v_avg = v_value / 60;
				v_value = 0;

				i_avg = i_value / 60;
				i_value = 0;

				if(i_avg > 768)											//6.8A
				{	fault_handler("Tularam_2 hiba!");

					rpm_value = 0;										//értékek nullázása
					v_value = 0;
					i_value = 0;
					l = 58;
					x = 2;												//túláram védelem tiltása
					z = 0;												//főmenü
					}
				}

			if(i == l)													//gombok újraengedélyezése
			{	l = 59;
				x = 0;
				}

//----------------------------------------------------------------------
			adc_val = adc_read(0);										//gomb érték ad

			if((adc_val < 896 || i == 255) && x == 0)					//kijelzés
			{
				if(adc_val < 896)								//gombok letiltása
				{	x = 1;
					l = i;
					}

				//lcd_xy(0, 0);

				if(adc_val < 64)										//right, jobbra léptet menübe
				{	//lcd_puts("right");

					++z;

					if(z == 3)
					{	z = 0;
						}

					lcd_clr();
					}
				else if(adc_val < 256)									//up, növeli az értéket
				{	//lcd_puts("up");

					if(z == 0)											//beállított rpm
					{	if(a)
						{	if(OCR2A < 255)
							{	OCR2A++;
								}
							}
						else
						{	if(rpm_all == 0)
							{	rpm_all = 1000;
								}
							else if(rpm_all < 9000)
							{	rpm_all += 250;
								}
							}
						}
					else if(z == 1)										//fényerő
					{	if(OCR0B < 77)
						{	OCR0B++;
							}
						}
					else if(z == 2)										//control type
					{	a = !a;

						if(a)											//pwm vezérlésre váltásnál beolvassa az elmentett kitöltési tényezőt
						{	OCR2A = eeprom_read_byte((uint8_t *) 0x02);

							x = 2;
							}
						}

					p = 0xFF;											//változás az értékekben, később mentés eepromba
					}
				else if(adc_val < 384)									//down, csökkenti az értéket
				{	//lcd_puts("down");

					if(z == 0)											//beállított rpm
					{	if(a)
						{	if(OCR2A > 0)
							{	OCR2A--;
								}
							}
						else
						{	if(rpm_all == 1000)
							{	rpm_all = 0;
								}
							else if(rpm_all > 1000)
							{	rpm_all -= 250;
								}
							}
						}
					else if(z == 1)										//fényerő
					{	if(OCR0B > 0)
						{	OCR0B--;
							}
						}
					else if(z == 2)										//control type
					{	a = !a;

						if(a)											//pwm vezérlésre váltásnál beolvassa az elmentett kitöltési tényezőt
						{	OCR2A = eeprom_read_byte((uint8_t *) 0x02);

							x = 2;
							}
						}

					p = 0xFF;											//változás az értékekben, később mentés eepromba
					}
				else if(adc_val < 576)									//left, balra léptet menübe
				{	//lcd_puts("left");

					if(z == 0)
					{	z = 3;
						}

					--z;

					lcd_clr();
					}
				else if(adc_val < 896)									//select, visszalép a főmenübe
				{	//lcd_puts("select");

					z = 0;

					lcd_clr();
					}

				lcd_xy(0, 0);

				if(z == 0)												//főmenü
				{														//feszültség kijelzés
					adc_val = ((unsigned long) v_avg * 5 * 11 * 10 + 1) / 1024;

					szamkiiras(adc_val / 10, 2);
					lcd_putc('.');
					lcd_putn(adc_val % 10);
					lcd_putc('V');

					lcd_xy(8, 0);										//fordulatszámkijelzés

					szamkiiras(rpm_avg, 5);
					lcd_puts("rpm");

					lcd_xy(0, 1);										//áram kijelzés

					adc_val = (((unsigned long) i_avg - 508) * 270) / 1024;

					szamkiiras(adc_val / 10, 2);
					lcd_putc('.');
					lcd_putn(adc_val % 10);
					lcd_putc('A');

					if(p)												//eepromba ment
					{	p = 0;

						eeprom_write_data(rpm_all, a);
						}
					}
				else if(z == 1)											//fényerő menü
				{	lcd_puts("fenyero: ");

					szamkiiras( ((OCR0B * 100) / 77), 3);
					lcd_putc('%');
					}
				else if(z == 2)											//control type menü
				{	if(a)
					{	lcd_puts("PWM control");
						}
					else
					{	lcd_puts("PI  control");
						}
					}
//----------------------------------------------------------------------		töltés jel számolás, kijelzés

				lcd_xy(8, 1);

				unsigned char j, k;

				j = (rpm_avg / 225);									//225 x 40 = 9000rpm max
				if(j > 40)
				{	j = 40;
					}

				for(k = 0; j >= 5; j -= 5, ++k)
				{	lcd_putc(5);
					}

				lcd_putc(j);

				for(k = (7 - k); k; --k)
				{	lcd_putc(' ');
					}
//----------------------------------------------------------------------
				}

			++i;
			}
		}

	return 0;
}

void szamkiiras(unsigned int szam, unsigned char db)
{
	unsigned char tomb[db], i, j;

	db--;

	for(i = db; i != 0xFF; --i, szam /= 10)
	{	tomb[i] = (szam % 10);
		}

	for(i = 0, j = 0; i <= db; ++i)
	{	if((tomb[i] != 0 && !j) || i == db)
		{	j = 1;
			}

		if(j == 0)
		{	lcd_putc(' ');
			}
		else
		{	lcd_putn(tomb[i]);
			}
		}
}

unsigned int adc_read(unsigned char ch)
{
	ADMUX = 0b01000000 | ch;
	ADCSRA = 0b11000110;

	while(ADCSRA & (1<<ADSC));

	return (ADC & 0x3FE);
}

unsigned char pi_control(int hiba)
{
	int duty;

	if(hiba > 0)
	{	i_temp += hiba / 2;
		}
	else if(hiba < 0)
	{	i_temp += hiba / 3;
		}

	if (i_temp > 65535)			{i_temp = 65535;}
	else if (i_temp < -65535)	{i_temp = -65535;}

	//i_temp = 0;

	duty = (hiba / 16) + (i_temp / 256);

	if (duty > 255)		{duty = 255;	}
	else if (duty < 0)	{duty = 0;		}

	return duty;
}

void eeprom_write_data(unsigned int rpm_all, unsigned char a)
{
	eeprom_write_word((uint16_t *) 0x00, rpm_all);
	eeprom_write_byte((uint8_t *) 0x02, OCR2A);
	eeprom_write_byte((uint8_t *) 0x03, OCR0B);
	eeprom_write_byte((uint8_t *) 0x04, a);
}

void eeprom_read_data(unsigned int *rpm_all, unsigned char *a)
{
	*rpm_all = eeprom_read_word((uint16_t *) 0x00);
	OCR2A = eeprom_read_byte((uint8_t *) 0x02);
	OCR0B = eeprom_read_byte((uint8_t *) 0x03);
	*a = eeprom_read_byte((uint8_t *) 0x04);
}

void fault_handler(char *msg)
{
	TCCR2A = 0b00100001;			//buzzer on, pwm off
	TCCR2B = 0b100;					//1kHz

	i_temp = 0;						//szabályzó nullázás

	lcd_clr();
	lcd_puts(msg);
	lcd_xy(0, 1);
	lcd_puts("Nyomj egy gombot");

	while(!(adc_read(0) < 896))
	{	_delay_ms(5);
		}

	TCCR2A = 0b00000001;			//timer2 output off

	lcd_clr();

	while((adc_read(0) < 896))
	{	_delay_ms(5);
		}

	lcd_puts("Inditashoz");
	lcd_xy(0, 1);
	lcd_puts("Nyomj egy gombot");

	while(!(adc_read(0) < 896))
	{	_delay_ms(5);
		}

	lcd_clr();

	while((adc_read(0) < 896))
	{	_delay_ms(5);
		}

	TCCR2B = 0b1;					//31250Hz
	TCCR2A = 0b10000001;			//Phase correct pwm mode, TOP = 0xFF

	TIFR0 = 0b1;					//clear flag
	GTCCR = 0b11;					//prescaler reset
	TCNT0 = 0;						//reset timer0
	TCNT1 = 1;						//timer1 kezdeti érték
}
