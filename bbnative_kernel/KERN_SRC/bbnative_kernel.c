/*
 * Copyright 2014-2015 Recep, Inc. All Rights Reserved.
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 *
 * charDev.c
 *
 *  Created on: Oct 3, 2014
 *      Author: user
 */

/* Native Sample Driver */
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/charDev.h>
#include <linux/version.h>    /* Current version Linux kernel */
#include <linux/module.h>    /* Basic support for loadable modules,
                  printk */
#include <linux/init.h>        /* module_init, module_exit */
#include <linux/kernel.h>    /* General kernel system calls */
#include <linux/types.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include<linux/interrupt.h>

#define GPIO_NUMBER    68 //User LED 0. GPIO number 149. Page 71 of BB-xM Sys Ref Manual.

//-gpios for lcd
#define DB4 69 //ok 8.9 ok
#define DB5 61 //ok 8.26 ok
#define DB6 44 //ok 8.12 ok
#define DB7 45 //ok 8.11 ok
#define RS 46 //ok
#define EN 60 //
#define BACKLIGHT 66 //

#define WIRELESS_INPUT 112

#define DRIVER_NAME "bbnative_kernel"
#define PDEBUG(fmt,args...) printk(KERN_DEBUG"%s:"fmt,DRIVER_NAME, ##args)
#define PERR(fmt,args...) printk(KERN_ERR"%s:"fmt,DRIVER_NAME,##args)
#define PINFO(fmt,args...) printk(KERN_INFO"%s:"fmt,DRIVER_NAME, ##args)

//COMMANDS
#define GPIO_SET_VALUE 		10
#define LCD_SEND_COMMAND 	11
#define LCD_WRITE_LINE	 	12

#define GPIO_ANY_GPIO_DEVICE_DESC    "some_device2"
#define GPIO_ANY_GPIO_DESC "GPIO desc2"

#define IOCTL_ECR_SUCCESS 0

static char msg[100]={0};
static short readPos=0;

static struct cdev bbb_cdev;
static dev_t bbb_char_dev;
static struct class *bbb_class;
static struct device *bbb_class_dev;

static ssize_t dev_read(struct file *, char *,size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static void lcd_send_command(unsigned char); // LCD ye komut göndermeye yarar
static void lcd_clear(void);        // LCD ekranı temizler
static void lcd_write_data(const char*);    // LCD ye string ifade yazar
static void lcd_go_to(char,char);      // LCD de satır ve stün olarak istenilen yere gider
static void lcd_initialize(void);           // LCD başlangıç ayarları yapılır
static void lcd_putch(char);          // LCD ye tek karakter yazmak için kullanılır.

static void LCD_RS(char);
static void LCD_EN(char);
static void LCD_EN_REFRESH(void);
static void set_gpios_for_lcd(unsigned char);
static void lcd_show_ready_message(void);
static void lcd_show_goodbyte_message(void);
static int gpio_initialize_driver(void);

static struct timer_list my_timer;
unsigned int irq_serial_com;
void my_timer_callback( unsigned long data )
{
  printk( "my_timer_callback called (%ld).\n", jiffies );
  gpio_set_value(BACKLIGHT, 1);
}

static irqreturn_t serial_com_handler(int irq, void *dev_id){
	unsigned long flags;
	// disable hard interrupts (remember them in flag 'flags')
	local_irq_save(flags);
	udelay(1000);
	udelay(1000);
	int z;
	for (z = 0;z  < 8; z++) {
		udelay(500);
		printk("gpio_val:%d\n",gpio_get_value(WIRELESS_INPUT));
		udelay(500);
	}
	udelay(1000);
	udelay(1000);
	udelay(1000);
	printk("stop bit RECEIVED\n");
	// restore hard interrupts
	local_irq_restore(flags);
	return IRQ_HANDLED;
}

static long device_ioctl(struct file* file_p_,
        unsigned int cmd_, unsigned long data_) {
       unsigned int cmd = cmd_;
        int error_code = IOCTL_ECR_SUCCESS;
        char status[16];
       printk(KERN_ERR "Sample driver: Received command (0x%x)\n", cmd);
       switch (cmd) {
              case GPIO_SET_VALUE:
				if (copy_from_user(&status, (void*)data_, 2)) {
					printk(KERN_ERR "ERROR");
					error_code = -EFAULT;
				}
				printk(KERN_ERR "GPIO_%d = %d\n",status[0], status[1]);
				gpio_export(status[0],status[1]);
				gpio_direction_output(status[0], status[1]);
				gpio_set_value(status[0], status[1]);
				gpio_unexport(status[0]);
				break;
              case LCD_SEND_COMMAND:

				if (copy_from_user(&status, (void*)data_, 1)) {
					printk(KERN_ERR "ERROR");
					error_code = -EFAULT;
				}
				 printk(KERN_ERR " lcd command :%d\n",status[0]);
				lcd_send_command(status[0]);
            	  break;
              case LCD_WRITE_LINE:
            	  if (copy_from_user(&status, (void*)data_, 16)) {
            		  printk(KERN_ERR "ERROR");
            		  error_code = -EFAULT;
            	  }
            	  lcd_write_data(status);
            	  break;
              default:
            	  break;
       }
       return 1;
}


struct file_operations bbbNativeFops = {
   .owner = THIS_MODULE,
   .read=dev_read,
   .write =dev_write,
   .unlocked_ioctl = device_ioctl
};

static void lcd_send_command(unsigned char command){
	LCD_RS(0);
	udelay(200);
	set_gpios_for_lcd(command & 0xF0);
	LCD_EN_REFRESH();
	set_gpios_for_lcd((command & 0x0F)<<4);
	LCD_EN_REFRESH();
}

static void lcd_clear(void){
	lcd_send_command(0x01);
	mdelay(2);
}
static void lcd_write_data(const char* string){
	gpio_set_value(BACKLIGHT, 0);
	printk( "Starting timer to fire in 5000ms (%ld)\n", jiffies );
	int res = mod_timer( &my_timer, jiffies + msecs_to_jiffies(5000) );
	if (res) printk("Error in mod_timer\n");
	while(*string)
	    lcd_putch(*string++);
}
static void lcd_go_to(char x, char y){
	if(x==1)
		lcd_send_command(0x80+((y-1)%16));
	else
		lcd_send_command(0xC0+((y-1)%16));

}

static int gpio_initialize_driver(){
	irq_handler_t serial_handler;
	int err;
	err = gpio_request_one(DB7, GPIOF_OUT_INIT_LOW, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO %d\n", DB7);
		goto exit_all;
	}
	err = gpio_request_one(DB6, GPIOF_OUT_INIT_LOW, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO %d\n", DB6);
		goto exit_free_db7;
	}
	err = gpio_request_one(DB5, GPIOF_OUT_INIT_LOW, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO %d\n", DB5);
		goto exit_free_db6;
	}
	err = gpio_request_one(DB4, GPIOF_OUT_INIT_LOW, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO %d\n", DB4);
		goto exit_free_db5;
	}
	err = gpio_request_one(RS, GPIOF_OUT_INIT_LOW, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO %d\n", RS);
		goto exit_free_db4;
	}
	err = gpio_request_one(EN, GPIOF_OUT_INIT_LOW, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO %d\n", EN);
		goto exit_free_rs;
	}

	err = gpio_request_one(BACKLIGHT, GPIOF_OUT_INIT_LOW, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO %d\n", BACKLIGHT);
		goto exit_free_en;
	}
	err = gpio_request_one(WIRELESS_INPUT, GPIOF_IN, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO Button%d\n", WIRELESS_INPUT);
		goto exit_free_backlight;
	}
	irq_serial_com = gpio_to_irq(WIRELESS_INPUT);
	serial_handler = &serial_com_handler;
	err = request_irq(irq_serial_com, serial_handler,
					  IRQF_TRIGGER_FALLING,
					  GPIO_ANY_GPIO_DESC, GPIO_ANY_GPIO_DEVICE_DESC);
	if (err) {
		printk("unable to request IRQ %d\n", irq_serial_com);
		goto exit_free_wireless_input;
	}
	err= gpio_direction_output(DB7, 0);
	err= gpio_direction_output(DB6, 0);
	err= gpio_direction_output(DB5, 0);
	err= gpio_direction_output(DB4, 0);
	err= gpio_direction_output(RS, 0);
	err= gpio_direction_output(EN, 0);
	err= gpio_direction_output(BACKLIGHT, 0);
	return 0;
exit_free_wireless_input:
	gpio_free(WIRELESS_INPUT);
exit_free_backlight:
	gpio_free(BACKLIGHT);
exit_free_en:
	gpio_free(EN);
exit_free_rs:
	gpio_free(RS);
exit_free_db4:
	gpio_free(DB4);
exit_free_db5:
	gpio_free(DB5);
exit_free_db6:
	gpio_free(DB6);
exit_free_db7:
	gpio_free(DB7);
exit_all:
	return -1;
}

static void lcd_initialize(void){
	LCD_RS(0);
	LCD_EN(0);
	lcd_send_command(0x28);  // 4 Bit , Çift Satır LCD
	lcd_send_command(0x0C);  // İmleç Gizleniyor
	lcd_send_command(0x06);  // Sağa doğru yazma aktif
	lcd_send_command(0x80);  // LCD Birinci Satır Konumunda
	lcd_send_command(0x28);  // 4 Bit , Çift Satır LCD
	lcd_clear();    // Ekran Temizleniyor
}

static void lcd_show_ready_message(void){
	PINFO("Lcd is ready\n");
	lcd_go_to(1,1);
	lcd_write_data("*Kernel Driver*");
	mdelay(1000);
	lcd_go_to(2,1);
	lcd_write_data("*Initialize Ok*");
	mdelay(1000);
	lcd_clear();
}

static void lcd_show_goodbyte_message(void){
	PINFO("GOODBYE\n");
	lcd_go_to(1,1);
	lcd_write_data("*Kernel Driver*");
	mdelay(1000);
	lcd_go_to(2,1);
	lcd_write_data("   *GOODBYE*   ");
	mdelay(1000);
	lcd_clear();
}

static void lcd_putch(char c){
	LCD_RS(1);
	udelay(200);
	set_gpios_for_lcd(c & 0xF0);
	LCD_EN_REFRESH();
	set_gpios_for_lcd((c & 0x0F)<<4);
	LCD_EN_REFRESH();
}

static void set_gpios_for_lcd(unsigned char value){
	int k;
	unsigned char port_val=0;
	for (k = 0; k < 4; k++) {
	   if (value > 127) {
		   port_val = 1;
	   } else {
		   port_val = 0;
	   }
	   switch (k) {
	   	   case 0:
	   		   gpio_set_value(DB7, port_val);
	   		   break;
		   case 1:
	   		   gpio_set_value(DB6, port_val);
	   		   break;
	   	   case 2:
	   		   gpio_set_value(DB5, port_val);
	   		   break;
	   	   case 3:
	   		   gpio_set_value(DB4, port_val);
	   		   break;
	   	   default:
	   		   break;
	   }
	   value = (unsigned char)(value << 1);
	}
}

static void LCD_RS(char value){
	gpio_set_value(RS, value);
}

static void LCD_EN(char value){
	gpio_set_value(EN, value);
}

static void LCD_EN_REFRESH(void){
	LCD_EN(1);
	udelay(20);
	LCD_EN(0);
}


static int __init ecr_helper_init(void)
{
   int ret, bbb_major;

   ret = alloc_chrdev_region(&bbb_char_dev, 1, 1,    "bbnative");
   bbb_major = MAJOR(bbb_char_dev);
   printk(KERN_ERR "Sample driver: major device num %d\n", bbb_major);
   if (ret < 0) {
       printk(KERN_ERR "Sample driver: can't get major ecr %d\n", bbb_major);
       goto err3;
   }

   cdev_init(&bbb_cdev, &bbbNativeFops);
   bbb_cdev.owner = THIS_MODULE;
   bbb_cdev.ops = &bbbNativeFops;
   ret = cdev_add(&bbb_cdev, bbb_char_dev, 1);
   if (ret) {
       printk(KERN_ERR "Sample driver: can't add ecr cdev\n");
       goto err2;
   }

   /* create class and device for udev information */
   bbb_class = class_create(THIS_MODULE, "bbnative");
   if (IS_ERR(bbb_class)) {
       printk(KERN_ERR "Sample driver: failed to create dev class\n");
       ret = -ENOMEM;
       goto err1;
   }

   bbb_class_dev = device_create(bbb_class, NULL, MKDEV(bbb_major, 1), NULL,
           "bbnative");
   if (IS_ERR(bbb_class_dev)) {
       printk(KERN_ERR "Sample driver: failed to create dev gpio class device\n");
       ret = -ENOMEM;
       goto err0;
   }

   ret = gpio_initialize_driver();
   if(ret!=0)
	   printk("Error in initialize gpios");

   printk("Timer module installing\n");
   gpio_set_value(BACKLIGHT, 1);
   	// my_timer.function, my_timer.data
   setup_timer( &my_timer, my_timer_callback, 0 );

   lcd_initialize();
   lcd_show_ready_message();

   return 0;
err0:
   class_destroy(bbb_class);
err1:
   cdev_del(&bbb_cdev);
err2:
   unregister_chrdev_region(bbb_char_dev, 1);
err3:
   return ret;
}
module_init(ecr_helper_init);

static void __exit ecr_helper_exit(void)
{
	/* destroy dev device class */
	device_destroy(bbb_class, MKDEV(MAJOR(bbb_char_dev), 1));
	class_destroy(bbb_class);

	/* Unregister the device */
	cdev_del(&bbb_cdev);
	unregister_chrdev_region(bbb_char_dev, 1);
	lcd_show_goodbyte_message();
	del_timer(&my_timer);
	gpio_free(DB7);
	gpio_free(DB6);
	gpio_free(DB5);
	gpio_free(DB4);
	gpio_free(RS);
	gpio_free(EN);
	gpio_free(BACKLIGHT);
	free_irq(irq_serial_com, GPIO_ANY_GPIO_DEVICE_DESC);
	gpio_free(WIRELESS_INPUT);

}
module_exit(ecr_helper_exit);

static ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off)
{
	lcd_write_data(buff);
	short count =0 ;
	while (len && (msg[readPos]!=0))
	{
		put_user(msg[readPos],buff++);
		count++;
		len--;
		readPos++;
	}
	return count;
}

static ssize_t dev_write(struct file *filp,const char *buf, size_t len,loff_t *off)
{
	short ind = len-1;
	short count=0;
	memset(msg,0,100);
	readPos=0;
	while(len>0)
	{
		msg[count++] = buf[ind--];
		len--;
	}
	return count;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Recep Yigitbasi");
MODULE_DESCRIPTION("Device Driver for Sample");
