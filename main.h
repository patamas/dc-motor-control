#include <stdint.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

static inline void mcu_init();
void szamkiiras(unsigned int , unsigned char );
unsigned int adc_read(unsigned char );
unsigned char pi_control(int );
void eeprom_write_data(unsigned int , unsigned char );
void eeprom_read_data(unsigned int *, unsigned char *);
void fault_handler(char *);

//select		720
//left			478
//up			132
//down 			306
//right			0
