/*
 * delay.c
 *
 *  Created on: Aug 3, 2015
 *      Author: recep
 */

#include <unistd.h>
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

void wait_ms(unsigned int m_sec)
{
	usleep(m_sec*1000);
}

void wait_us(unsigned int u_sec)
{
	usleep(u_sec);
}
