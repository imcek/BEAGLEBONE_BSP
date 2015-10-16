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

#ifndef LIB_LCD_H_
#define LIB_LCD_H_

#define CS		0x0010		// chip select at P1.4
#define SDIN	0x0080		// data in at P1.7
#define SCLK	0x0020		// clk at P1.5
#define DC		0x0040		// Data/Cmd at P1.6
#define RES 	0x0001		// reset at P1.0

#define lcd_width		96
#define lcd_height		64
#define lcd_height_b	8
#define lcd_height_real	72	// display RAM is > 96x64
							// real rows=9 (only the MSB is visible on display (9th row, one bit)
							// real columns >96

#define space_char	1		// space between chars

void write_h_string(unsigned char,unsigned char,const char*,unsigned char);
void string_typer(unsigned char,unsigned char,const char*,unsigned char,unsigned int);
void countdown(unsigned char);
void f_scroller_func(signed int,signed int,const char*,const char*,unsigned char);
void f_scroller_normal(signed int,signed int,const char*);
void draw_bitmap(signed int,signed int,unsigned int,unsigned int,const char*);
void draw_line(unsigned char,unsigned char,unsigned char,unsigned char);
void convert_font_size(unsigned char,unsigned char,unsigned char,unsigned char);
void write_string(unsigned char,unsigned char,const char*,unsigned char);
void write_char(unsigned char,unsigned char,unsigned char,unsigned char);
void send_data_array(const char*,unsigned char);
void set_cursor(unsigned char,unsigned char);
void fill_display(unsigned char,unsigned char,unsigned char);
void set_instruction(unsigned char,unsigned char);
void init_LCD(void);
void init_USCI(void);

#endif /*LIB_LCD_H_*/
