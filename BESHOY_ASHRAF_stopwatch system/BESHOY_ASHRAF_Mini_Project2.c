#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/interrupt.h>
unsigned char second = 0;
unsigned char minute = 0;
unsigned char hour = 0;


/* Description :
 * For clock=1Mhz and prescaler F_CPU/1024 every count will take 1ms
 * so put initial timer counter = 0  0 --> 1000 (1ms per tick of interrupt)
 * so we need timer to set OCRA1 to 1000 to get 1 second per interrupt
 */

void Timer1_Init(){
	/* Configure the timer mode
	 * 1. Non PWM mode FOC1A = 1 (using Channel A).
	 * 2. CTC Mode WGM10 = 0 & WGM11 = 0 & WGM12 = 1 & WGM13 = 0.
	 * 3. clock = CPU clock/1024 CS10 = 1 CS11 = 0 CS12 = 1.
	 */
	TCCR1A = (1<<FOC1A);
	TCNT1 = 0;
	OCR1A = 1000;
	TIMSK|=(1<<OCIE1A); // Timer1 Compare match interrupt
	TCCR1B = (1<<WGM12)|(1<<CS10)|(1<<CS12); //Timer1 clock enable
}


ISR(TIMER1_COMPA_vect)
{
	second ++;
	if (second == 60){
		second = 0;
		minute ++;
	}
	if (minute == 60){
		second = 0;
		minute = 0;
		hour ++;
	}
	if (hour == 12){
		second = 0;
		minute = 0;
		hour = 0 ;
	}
}

void INT0_Init(){
	DDRD &= ~(1<<PD2); //PD2 pin is used as input for interrupt request
	PORTD |= (1<<PD2); //internal pull up for PD2
	MCUCR |= (1<<ISC01); //falling edge interrupt
	GICR |= (1<<INT0); //enable module interrupt for external INT0
}

ISR(INT0_vect){
	//reseting the timer to 0
	second = 0;
	minute = 0;
	hour = 0;
}


void INT1_Init(){
	DDRD &= ~(1<<PD3); //PD3 pin is used as input for interrupt request (INT2)
	MCUCR |= (1<<ISC11) |  (1<<ISC10); //raising edge interrupt
	GICR |= (1<<INT1); //enable module interrupt for external INT1
}

ISR(INT1_vect){
	//pause the time by stopping the clock
	TCCR1B &= ~(1<<CS10) & ~(1<<CS12);
}


void INT2_Init(){
	DDRB &= ~(1<<PB2); //PB2 pin is used as input for interrupt request
	PORTB |= (1<<PB2); //internal pull up
	MCUCSR &= ~(1<<ISC2); //falling edge interrupt
	GICR |= (1<<INT2); //enable module interrupt for external INT2
}

ISR(INT2_vect){
	//continue the time by stopping the clock
	TCCR1B = (1<<WGM12)|(1<<CS10)|(1<<CS12);
}

void General_Init(){
	DDRA |= 0x3F;  //enables of all 7 segments
	PORTA |= 0x3F; //turn on all 7 segments displaying zeroes at first
	DDRC = 0x0F;  // first 4 pint are outputs to display number in 7 segments
	PORTC &= 0xF0; //initialized as zero at first
}


int main(){
	 General_Init();
	 INT2_Init();
	 INT1_Init();
	 INT0_Init();
	 Timer1_Init();
	 SREG |= (1<<7); //ENABLE GLOBAL INTERRUPS
	while(1){
		/* Description :
		 * Delay is 5 micro seconds between each 7-segment (most suitable one).
		 * Only one variable for seconds is needed by using '%' operator.
		 * for example : if seconds = 59 then 59%10=9 (SECONDS1) and 59/10=5 (SECONDS2) .And same for minutes and hours.
		 */
		PORTA = (1<<5);
		PORTC = second % 10;
		_delay_us(5);
		PORTA = (1<<4);
		PORTC = second / 10;
		_delay_us(5);
		PORTA = (1<<3);
		PORTC = minute % 10;
		_delay_us(5);
		PORTA = (1<<2);
		PORTC = minute / 10;
		_delay_us(5);
		PORTA = (1<<1);
		PORTC = hour % 10;
		_delay_us(5);
		PORTA = (1<<0);
		PORTC = hour / 10;
		_delay_us(5);
	}
}
