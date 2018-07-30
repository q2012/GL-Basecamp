#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <stdint.h>

static void soft_delay(volatile uint16_t N)
{
    volatile uint8_t inner = 0xFF;
    while (N--)
        while (inner--);
}

inline uint16_t LKG()
{
	static uint16_t seed = 10;
    return seed = (7141 * seed + 54773) % 259200;
}

void candle()
{
    uint16_t gen = LKG();
    if(gen > 30294)
    	OCR0A = 255;
    else if(gen > 24000)
        OCR0A = 232;
    else if(gen > 20458)
        OCR0A = 217;
    else if(gen > 18492)
        OCR0A = 200;
    else if(gen > 16524)
        OCR0A = 182;
    else if(gen > 14557)
        OCR0A = 167;
    else if(gen > 12590)
        OCR0A = 150;
}

int main(void)
{
	//Fast PWM on PIN6
	TCCR0A = (1 << COM0B1) | (1 << WGM00) | (1 << WGM01) | (1 << COM0A1);
	//No prescaler
    TCCR0B = (1 << CS00);
	//OC0A - out
    DDRD |= 1 << 6; 

    while(1) 
    {
    	candle();
    	soft_delay(750);
    }    

    return 0;
} 