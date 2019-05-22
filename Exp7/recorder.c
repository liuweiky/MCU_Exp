#include <reg52.h>
#include <intrins.h>

#define FOSC 18432000L
#define BAUD 9600

#define REC_SIZE 32000

typedef unsigned char UCHAR;
typedef unsigned int UINT;

sbit ADC0 = P1^0;
sbit WR1 = P1^1;
sbit WR2 = P1^4;

sfr P4 = 0x0C0;
sfr P4SW = 0x0BB;

sbit sclk = P4^4;
sbit sdata = P4^5;

/*Declare SFR associated with the ADC */
sfr ADC_CONTR = 0xBC; //ADC control register
sfr ADC_RES = 0xBD; //ADC high 8-bit result register
sfr ADC_LOW2 = 0xBE; //ADC low 2-bit result register
sfr P1ASF = 0x9D; //P1 secondary function control register
/*Define ADC operation const for ADC_CONTR*/
#define ADC_POWER 0x80 //ADC power control bit
#define ADC_FLAG 0x10 //ADC complete flag
#define ADC_START 0x08 //ADC start control bit
#define ADC_SPEEDLL 0x00 //540 clocks
#define ADC_SPEEDL 0x20 //360 clocks
#define ADC_SPEEDH 0x40 //180 clocks
#define ADC_SPEEDHH 0x60 //90 clocks

UCHAR led_table[] = {
	0x0C0, 0x0F9, 0x0A4, 0x0B0,
	0x099, 0x092, 0x082, 0x0F8,
	0x080, 0x090, 0x088, 0x083,
	0x0C6, 0x0A1, 0x086, 0x08E
};

UCHAR code rec[32000];

void send_byte(UCHAR byte)
{
	UCHAR dat = led_table[byte];
	//UCHAR dat = 0xC0;
	UCHAR c = 8;
	while (c--)
	{
		sdata = dat & 0x80;
		sclk = 0;
		sclk = 1;
		dat <<= 1;
	}
}

void display_led(UCHAR s)
{
	send_byte(s % 10);
	s /= 10;
	send_byte(s % 10);
	send_byte(s / 10);
}


void delay_1us(UINT t)
{
	while (t--);
}

void init_uart()
{
	SCON = 0x5a; //8 bit data ,no parity bit
	TMOD = 0x20; //T1 as 8-bit auto reload
	TH1 = TL1 = -(FOSC/12/32/BAUD); //Set Uart baudrate
	TR1 = 1;
}

void init_adc()
{
	P1ASF = 0x01; //Open 8 channels ADC function
	ADC_RES = 0; //Clear previous result
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL;
	delay_1us(20000); //ADC power-on and delay
}

UCHAR read_adc(UCHAR channel)
{
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | channel | ADC_START;
	_nop_(); //Must wait before inquiry
	_nop_();
	_nop_();
	_nop_();
	while (!(ADC_CONTR & ADC_FLAG)); //Wait complete flag
	ADC_CONTR &= ~ADC_FLAG; //Close ADC
	return ADC_RES; //Return ADC result
}


void init_all()
{
	P4SW = 0x30;	// 00110000
	TMOD = 0x11;	// 00010001
	init_uart();
	init_adc();
}

void main()
{
	UCHAR i;
	//a = 1;
	init_all();
	WR1 = 1;
	WR2 = 1;
	while (1)
	{
		i = read_adc(0);
		display_led(i);
		delay_1us(1000);
	}
}