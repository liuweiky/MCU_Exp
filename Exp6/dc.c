#include <reg52.h>
#include <intrins.h>
#include <stdlib.h>

#define CHAR_HEIGHT 16
#define CHAR_WIDTH CHAR_HEIGHT

typedef unsigned char UCHAR;
typedef unsigned int UINT;

sbit CS1 = P1^7;
sbit CS2 = P1^6;
sbit RS = P3^5;
sbit RW = P3^4;
sbit E = P3^3;
sbit S1 = P3^6;
sbit S2 = P3^7;

sfr P4 = 0x0C0;
sfr P4SW = 0x0BB;

sbit sclk = P4^4;
sbit sdata = P4^5;
sbit motor = P1^1;

UCHAR cur_speed = 0, t_speed = 0, t0_cnt = 0;
UCHAR high_speed = 120, low_speed = 75;
UCHAR target_speed = 75;

// UINT expect_random = RAND_MAX / 2;

UINT M = 256, N = 34, X = 0;

UCHAR led_table[] = {
	0x0C0, 0x0F9, 0x0A4, 0x0B0,
	0x099, 0x092, 0x082, 0x0F8,
	0x080, 0x090, 0x088, 0x083,
	0x0C6, 0x0A1, 0x086, 0x08E
};

UCHAR code digit_code[10][16] = {
	
	{0x00,0xE0,0x10,0x08,0x08,0x10,0xE0,0x00,0x00,0x0F,0x10,0x20,0x20,0x10,0x0F,0x00},/*"0",0*/

	{0x00,0x00,0x10,0x10,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00},/*"1",1*/

	{0x00,0x70,0x08,0x08,0x08,0x08,0xF0,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00},/*"2",2*/

	{0x00,0x30,0x08,0x08,0x08,0x88,0x70,0x00,0x00,0x18,0x20,0x21,0x21,0x22,0x1C,0x00},/*"3",3*/

	{0x00,0x00,0x80,0x40,0x30,0xF8,0x00,0x00,0x00,0x06,0x05,0x24,0x24,0x3F,0x24,0x24},/*"4",4*/

	{0x00,0xF8,0x88,0x88,0x88,0x08,0x08,0x00,0x00,0x19,0x20,0x20,0x20,0x11,0x0E,0x00},/*"5",5*/

	{0x00,0xE0,0x10,0x88,0x88,0x90,0x00,0x00,0x00,0x0F,0x11,0x20,0x20,0x20,0x1F,0x00},/*"6",6*/

	{0x00,0x18,0x08,0x08,0x88,0x68,0x18,0x00,0x00,0x00,0x00,0x3E,0x01,0x00,0x00,0x00},/*"7",7*/

	{0x00,0x70,0x88,0x08,0x08,0x88,0x70,0x00,0x00,0x1C,0x22,0x21,0x21,0x22,0x1C,0x00},/*"8",8*/

	{0x00,0xF0,0x08,0x08,0x08,0x10,0xE0,0x00,0x00,0x01,0x12,0x22,0x22,0x11,0x0F,0x00},/*"9",9*/
		
};

void lcm_pending()
{
	P2 = 0x00;
	RS = 0;
	RW = 1;
	E = 1;
	while(P2^7 == 1);
	E = 0;
}
	
void delay_lcm()
{
	UCHAR t = 32;
	while (t--)
		_nop_();
}

void cmd_out(UCHAR cmd)
{
	P2 = 0xFF;
	lcm_pending();
	RS = 0;
	RW = 0;
	P2 = cmd;
	E = 1;
	delay_lcm();
	E = 0;
}

void switch_on()
{
	cmd_out(0x3F);
}

void switch_off()
{
	cmd_out(0x3E);
}

void set_init_line_ptr(UCHAR line)
{
	cmd_out(0xC0 | line);
}

void set_page_ptr(UCHAR page)
{
	cmd_out(0xB8 | page);
}

void set_column_ptr(UCHAR column)
{
	cmd_out(0x40 | column);
}

void write_data(UCHAR dt)
{
	P2 = 0xFF;
	lcm_pending();
	RS = 1;
	RW = 0;
	P2 = dt;
	E = 1;
	delay_lcm();
	E = 0;
	//CS1 = 0;
}

void reset_lcm()
{
	UCHAR i = 0, j = 0;
	CS1 = 1;
	CS2 = 1;
	set_init_line_ptr(0);
	for (i = 0; i < 8; i++)
	{
		set_page_ptr(i);
		set_column_ptr(0);
		for (j = 0; j < 64; j++)
			write_data(0x00);
	}
	CS1 = 0;
	CS2 = 0;
}

void dispaly_alnum(UCHAR xpage_offset, UCHAR y_offset, UCHAR* ptr)
{
	UCHAR i = 0, j = 0, yj;
	
	for (i = xpage_offset; i < xpage_offset + CHAR_HEIGHT / 8; i++)
	{
		set_column_ptr(y_offset);
		for (j = y_offset; j < y_offset + CHAR_WIDTH / 2; j++)
		{
			yj = j;
			if (yj < 64)
			{
				CS1 = 1;
				CS2 = 0;
			}
			else
			{
				CS1 = 0;
				CS2 = 1;
				yj -= 64;
			}
			set_page_ptr(i);
			set_column_ptr(yj);
			
			write_data(*(ptr));
			ptr++;
		}
	}
	
	CS1 = 0;
	CS2 = 0;
}

void display_3speed()
{
	dispaly_alnum(1, 44, digit_code[cur_speed / 100]);
	dispaly_alnum(1, 60, digit_code[cur_speed / 10 % 10]);
	dispaly_alnum(1, 76, digit_code[cur_speed % 10]);
	
	dispaly_alnum(3, 44, digit_code[high_speed / 100]);
	dispaly_alnum(3, 60, digit_code[high_speed / 10 % 10]);
	dispaly_alnum(3, 76, digit_code[high_speed % 10]);
	
	dispaly_alnum(5, 44, digit_code[low_speed / 100]);
	dispaly_alnum(5, 60, digit_code[low_speed / 10 % 10]);
	dispaly_alnum(5, 76, digit_code[low_speed % 10]);
}

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

void init()
{
	P4SW = 0x30;	// 00110000
	TMOD = 0x11;	// 00010001
	
	TH0 = 0x3C;
	TL0 = 0xB0;
	
	TH1 = 0xFF;
	TL1 = 0x00;
	
	TCON = 0x51;
	
	IE = 0x8B;
	IP = 0x01;
}

void delay_200ms()
{
	int a = 20, b = 20;
	while (a--)
		while (b--);
}

// round intr
ex_int0() interrupt 0
{
	t_speed++;
}

// 50ms intr
t0_int0() interrupt 1
{
	if (++t0_cnt < 20)	// 1s intr
	{
		TH0 = 0x3C;
		TL0 = 0xB0;
		
		display_led(cur_speed);
	
		if (S1 == 0)
			target_speed = low_speed;
		else if (S2 == 0)
			target_speed = high_speed;
		
		return;
	}
	
	
	t0_cnt = 0;
	cur_speed = t_speed;
	t_speed = 0;
	
	/*if (cur_speed > target_speed)
		expect_random = expect_random > 25 ? expect_random - 25 : expect_random;
	else if (cur_speed < target_speed)
		expect_random = expect_random < RAND_MAX - 25 ? expect_random + 25 : expect_random;*/
	
	if (cur_speed > target_speed)
		N = N > 2 ? N - 1 : N;
	else if (cur_speed < target_speed)
		N = N < M ? N + 1 : M;
}

// random n/m - xms

t1_int1() interrupt 3
{
	//UINT r = rand();
	
	/*if (r < expect_random)
		motor = 0;
	else
		motor = 1;*/
	
	X += N;
	
	if (X > M)
	{
		motor = 0;
		X -= M;
	}
	else
		motor = 1;
	
	TH1 = 0xFF;
	TL1 = 0x00;
}

void main()
{
	srand(0);
	init();
	lcm_pending();
	switch_on();
	
	reset_lcm();
	display_3speed();
	
	//motor = 0;
	
	while (1)
	{
		display_3speed();
		//delay_200ms();
	}
	
}