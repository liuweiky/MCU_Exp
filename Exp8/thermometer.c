#include <reg52.h>
#include <intrins.h>

#define CHAR_HEIGHT 16
#define CHAR_WIDTH CHAR_HEIGHT

#define LEFT_LCD 0
#define RIGHT_LCD 1

typedef unsigned char UCHAR;
typedef unsigned int UINT;

sbit CS1 = P1^7;
sbit CS2 = P1^6;
sbit RS = P3^5;
sbit RW = P3^4;
sbit E = P3^3;

sbit DQ = P1^4;

sbit S1 = P3^6;
sbit S2 = P3^7;

sbit heater = P1^1;

int M = 256, N = 34, X = 0;
int target_temp = 25, cur_temp = 100;
int prev_temp = 0;	// (temp)k-1, current is (temp)k
int err_integral = 0; // integral term

UCHAR key_delay = 0;

int kp = 20;
int ki = 50;
int kd = 0;

int t1_cnt = 0, integral_reset = 0;

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
	while (t--);
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

void display_temp()
{
	dispaly_alnum(2, 44, digit_code[cur_temp / 100]);
	dispaly_alnum(2, 60, digit_code[cur_temp / 10 % 10]);
	dispaly_alnum(2, 76, digit_code[cur_temp % 10]);
	
	dispaly_alnum(4, 44, digit_code[target_temp / 100]);
	dispaly_alnum(4, 60, digit_code[target_temp / 10 % 10]);
	dispaly_alnum(4, 76, digit_code[target_temp % 10]);
}

void delay_1us(UINT t)
{
	while (t--);
}

bit reset_ds18b20()
{
	bit res;
	/*DQ = 1;
	delay_1us(10);*/
	
	// pull down dq 400us-960us
	DQ = 0;
	delay_1us(960);
	
	// pull up dq 15us-60us
	DQ = 1;
	delay_1us(60);
	
	// res = 1: failed
	// res = 0: success
	res = DQ;
	
	//delay 60us-240us
	delay_1us(240);
	//DQ = 1;
	
	return res;
}

void write_byte_ds18b20(UCHAR byte)
{
	UCHAR i = 8;
	while (i--)
	{
		DQ = 0; // begin write
		
		DQ = byte & 1;
		// delay 15us-60us for sampling
		delay_1us(60);
		
		//release
		DQ = 1;
		
		byte >>= 1;
		delay_1us(25);
	}
}

UCHAR read_byte_ds18b20()
{
	UCHAR res = 0, i = 8;
	while (i--)
	{
		// begin read
		DQ = 0;
		delay_1us(5);
		DQ = 1;
		res >>= 1;
		if (DQ)
			res |= 0x80;
		delay_1us(60);
	}
	return res;
}

UCHAR get_temp()
{
	UCHAR low8, high8, t;
	UINT res;
	while(reset_ds18b20());
	cur_temp = 50;
	write_byte_ds18b20(0xCC);
	write_byte_ds18b20(0x44);
	
	while(reset_ds18b20());
	
	cur_temp = 51;
	write_byte_ds18b20(0xCC);
	write_byte_ds18b20(0xBE);
	
	low8 = read_byte_ds18b20();
	high8 = read_byte_ds18b20();
	
	res = high8;
	res <<= 8;
	res |= low8;
	t = res * 0.0625;
	return t;
}

void init_all()
{
	lcm_pending();
	switch_on();
	reset_lcm();
	
	// use T0
	
	TMOD = 0x11;	// 00010001
	
	TH0 = 0xEE;
	TL0 = 0xEE;
	
	TH1 = 0x3C;
	TL1 = 0xB0;
	
	TCON = 0x50;	// 01010000
	
	IE = 0x8A;		// 10001010
	IP = 0x02;		// 00001000
	
	get_temp();
}

void pid()
{
	int res;
	int cur_err = target_temp - cur_temp;
	int prev_err = target_temp - prev_temp;
	
	/*if (cur_err > 0 && err_integral < 30000 - cur_err)
		err_integral += cur_err;	// integral
	else if (cur_err < 0 && err_integral > -30000 - cur_err)
		err_integral += cur_err;*/
	
	res = kp * cur_err + ki * err_integral + kd * (cur_err - prev_err);
	
	if (res <= 0)
		N = 1;
	else if (res > M)
		N = M;
	else
		N = res;
}

t0_int0() interrupt 1
{
	
	X += N;
	
	if (X > M)
	{
		heater = 1;
		X -= M;
	}
	else
		heater = 0;
	
	TH0 = 0xEE;
	TL0 = 0xEE;
}

t1_int1() interrupt 3
{
	
	if (++key_delay == 4)
	{
		if (S1 == 0)
			target_temp++;
		else if (S2 == 0)
			target_temp--;
		key_delay = 0;
	}
	
	if (++t1_cnt < 10)	// 0.5s intr
	{
		TH1 = 0x3C;
		TL1 = 0xB0;
		
		return;
	}
	
	
	pid();
	
	t1_cnt = 0;
	
	
	
	display_temp();
	
	TH1 = 0x3C;
	TL1 = 0xB0;
}

void main()
{
	UCHAR i = 255, j = 5;
	init_all();
	cur_temp = 52;
	while (i--)
			while (j--);
	while (1)
	{
		delay_1us(65535);
		delay_1us(65535);
		delay_1us(65535);
		delay_1us(65535);
		delay_1us(65535);
		delay_1us(65535);
		delay_1us(65535);
		prev_temp = cur_temp;
		cur_temp = get_temp();
	}
}