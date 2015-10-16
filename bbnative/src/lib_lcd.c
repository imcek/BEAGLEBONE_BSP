/*******************************
 *          __
 *         /\ \
 *  __  _  \_\ \     __    ___
 * /\ \/'\ /'_' \  /'__'\ /'___\
 * \/>  <//\ \ \ \/\  __//\ \__/
 *  /\_/\_\ \___,_\ \____\ \____\
 *  \//\/_/\/__,_ /\/____/\/____/
 *
 *  Author: declis (xdec.de)
 ********************************/

#include "ascii_char.h"
#include "lib_lcd.h"
#include "delay.h"

#define max_f_size		8	// cache size for "font resize function"

unsigned char byte,bit_num;

void write_h_string(unsigned char x, unsigned char y, const char *string, unsigned char f_size)
{
	while(*string!=0)
	{
		write_char(x,y,*string++,f_size);
		if(f_size==1) y+=f_size+1;
		else if(f_size>1) y+=f_size;
		else y++;
	}
}

void string_typer(unsigned char x, unsigned char y, const char *string, unsigned char f_size, unsigned int ms)
{
	while(*string!=0)
	{
		write_char(x,y,*string++,f_size);
		if(f_size>1) x+=f_size;
		else x++;
		wait_ms(ms);
	}
}

void f_scroller_func(signed int x, signed int y, const char *string, const char *func, unsigned char func_size)
{
	byte=0;
	while(*string!=0&&x<lcd_width)
	{
		if(x>func_size) byte=x-func_size; // use abs() here
		else byte=x;
		draw_bitmap(x,y+func[byte],f_width,8,ascii_table[*string++]);
		set_instruction(1,0);	// clear byte before
		x+=f_width+space_char;
		byte++;
	}
}

void f_scroller_normal(signed int x, signed int y, const char *string)
{
	while(*string!=0&&x<lcd_width)
	{
		draw_bitmap(x,y,f_width,8,ascii_table[*string++]);
		set_instruction(1,0);	// clear byte before
		x+=f_width+space_char;
	}
}

void countdown(unsigned char num)
{
	signed char size=8;
	while(size!=-1)
  	{
  		write_char(4,0,num,size--);
  		wait_ms(111);
  		fill_display(96,64,0);
  	}
}

void draw_bitmap(signed int x, signed int y, unsigned int b_width, unsigned int b_height, const char *bmap)
{
	unsigned int h_index=0,w_index=0,width_max=0,x_off=0,y_off=0;
	byte=0;

	if(y+(signed int)b_height<0) return;	// outside display
	if(x+(signed int)b_width<0) return;
	if(y>=lcd_height) return;
	if(x>=lcd_width) return;

	if(x<0)
	{
		w_index=x*-1;		// set bmap x-offset
		x_off=w_index;
		x=0;				// start at display position x=0 with x-offset
	}

	if(b_height%8) b_height+=8;
	b_height/=8;

	if(x+(signed int)b_width>=lcd_width) 	// width overflow check
		width_max=lcd_width-x;
	else width_max=b_width;

	if(y<0)
	{
		y*=-1;
		y_off=y/8;
		b_height-=y_off;
		while(h_index<b_height)
		{
			set_cursor(x,h_index);
			while(w_index<width_max)
			{
				byte=bmap[w_index+((h_index+y_off)*b_width)]>>(y%8);
				if(h_index+1!=b_height)
					byte|=bmap[w_index+((h_index+y_off+1)*b_width)]<<(8-(y%8));
				set_instruction(1,byte);
				w_index++;
				byte=0;
			}
			h_index++;
			w_index=x_off;
			if(h_index>=lcd_height_b) return;
		}
	}
	else
	{
		y_off=y/8;
		while(h_index<=b_height)
		{
			set_cursor(x,y_off+h_index);
			while(w_index<width_max)
			{
				if(h_index!=b_height)
					byte=bmap[w_index+(h_index*b_width)]<<(y%8);
				if(h_index)
					byte|=(bmap[w_index+((h_index-1)*b_width)]>>(8-(y%8)));	// byte before
				set_instruction(1,byte);
				w_index++;
				byte=0;
			}
			h_index++;
			w_index=x_off;
			if(h_index+y_off>=lcd_height_b) return;
			if(h_index==b_height&&!(y%8)) return;
		}
	}
}

void convert_font_size(unsigned char x, unsigned char y, unsigned char character, unsigned char f_size)
{
	unsigned char x_char=0,bit_num_b=0,size=0,px_size=0,y_pos_new=0,x_pos_new=0;
	unsigned char cache[max_f_size],i=0;
	byte=0;
	bit_num=0;

	if (f_size==1) size=2;
	else size=f_size;

	while(x_char<f_width)													// test byte, starting at 0 to f_width (font width)
	{
		while(bit_num<8)													// test bit 0..7 of current byte
		{
			if(ascii_table[character][x_char]&(1<<bit_num))					// if bit=1 in byte...
			{
				while(px_size<size)											// duplicate bits (f_size*f_size)
				{
					if(bit_num_b>7&&px_size>0)								// byte overflow, new byte
					{
						set_cursor(x+x_pos_new,y+y_pos_new++);				// set cursor to next y-address
						set_instruction(1,byte);							// send byte
						bit_num_b=0;										// reset bit counter
						cache[i++]=byte;									// save byte in cache
						byte=0;												// reset byte
					}
					byte|=(1<<bit_num_b);									// set bit in byte
					px_size++;												// increment pixel duplicate counter
					bit_num_b++;											// calculate new bit position
				}
				px_size=0;													// reset pixel duplicate counter
			}
			else bit_num_b+=size;											// bit=0, calculate new bit position in byte

			if(bit_num_b>7)													// byte overflow, new byte
			{
				set_cursor(x+x_pos_new,y+y_pos_new++);
				set_instruction(1,byte);
				bit_num_b-=8;
				cache[i++]=byte;
				byte=0;
			}
			bit_num++;														// test next byte in array
		}
		y_pos_new=0;														// reset y-offset
		x_pos_new++;														// increment x-position
		i=0;																// reset cache counter
		if(f_size==1) size=0;												// double height font (only for f_size=1)
		else size--;														// first row is ready, only size-1
		while(size--)
		{
			while(i<f_size)
			{
				set_cursor(x+x_pos_new,y+y_pos_new++);
				set_instruction(1,cache[i++]);
			}
			i=0;
			y_pos_new=0;
			x_pos_new++;
		}
		x_char++;
		if(f_size==1) size=2;
		else size=f_size;
		i=0;
		bit_num=0;
	}
}

void write_string(unsigned char x, unsigned char y, const char *string, unsigned char f_size)
{
	x*=(f_width+space_char);
	set_cursor(x,y);
	while(*string!=0)
	{
		if(f_size)
		{
			convert_font_size(x,y,*string++,f_size);
			x+=(f_size*f_width+space_char);
		}
		else
		{
			send_data_array(ascii_table[*string++],f_width);
			y=space_char;
			while(y--)
				set_instruction(1,0);
		}
	}
}

void write_char(unsigned char x, unsigned char y, unsigned char character, unsigned char f_size)
{
	x*=(f_width+space_char);
	set_cursor(x,y);
	if(f_size)
		convert_font_size(x,y,character,f_size);
	else
		send_data_array(ascii_table[character],f_width);
}

void send_data_array(const char *d_array, unsigned char size)
{
	while(size--)
		set_instruction(1,*d_array++);
}

void set_cursor(unsigned char x, unsigned char y)
{
	set_instruction(0,0x80+x);
	set_instruction(0,0x40+y);
}

void fill_display(unsigned char width, unsigned char height, unsigned char byte)
{
	height/=8;
	while(height--)
	{
		set_cursor(0,height);
		while(width--)
			set_instruction(1,byte);
		width=lcd_width;
	}
}

void set_instruction(unsigned char register_sel, unsigned char number)
{
	//if(register_sel)
	//	P1OUT|=DC;
	//else P1OUT&=~DC;

//	P1OUT&=~CS;					// start condition
//	while(IFG2&UCB0TXIFG);		// TX buffer ready?
 // 	UCB0TXBUF=number;			// start transmission
}

void init_LCD(void)
{
	wait_ms(100);
//	P1OUT|=RES;

	set_instruction(0,0x21);			// function set, extended instruction set (h=1)
	set_instruction(0,0xCF);			// set Vop, contrast
	set_instruction(0,0x04);			// temperature control, set coefficient
	set_instruction(0,0x17);			// set bias system
	set_instruction(0,0x20);			// function set, basic instruction set (h=0)
	set_instruction(0,0x0C);			// display control, normal mode

	fill_display(lcd_width,lcd_height_real,0);	// display RAM is undefined after reset, clean dat shit
}

void init_USCI(void)
{
//	P1DIR|=RES+DC+CS;
//	P1OUT&=~RES+DC;
//	P1OUT&=~CS;

//	P1SEL|=SDIN+SCLK;
//	P1SEL2|=SDIN+SCLK;

//  	UCB0CTL1|=UCSWRST;					// USCI in reset state
  	// SPI Master, 8bit, MSB first, synchronous mode
 // 	UCB0CTL0|=UCMST+UCSYNC+UCCKPH+UCMSB;
 // 	UCB0CTL1|=UCSSEL_2;					// USCI CLK-SRC=SMCLK=~8MHz
 // 	UCB0CTL1&=~UCSWRST;					// USCI released for operation
 // 	IE2|=UCB0TXIE;						// enable TX interrupt
  //	IFG2&=~UCB0TXIFG;
//  	_EINT();							// enable interrupts
}

#pragma INTERRUPT (USCI)
#pragma vector=USCIAB0TX_VECTOR
void USCI(void)
{
//	P1OUT|=CS;				// transmission done
//	IFG2&=~UCB0TXIFG;		// clear TXIFG
}
