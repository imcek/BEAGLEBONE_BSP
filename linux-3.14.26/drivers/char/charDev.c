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


#define GPIO_NUMBER    67 //User LED 0. GPIO number 149. Page 71 of BB-xM Sys Ref Manual.

static char msg[100]={0};
static short readPos=0;
static int times=0;

static struct cdev mxc_cdev;
static dev_t mxc_char_dev;
static struct class *mxc_class;
static struct device *mxc_class_dev;

static ssize_t dev_read(struct file *, char *,size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static int device_ioctl(struct inode* inode_p_, struct file* file_p_,
        unsigned int cmd_, unsigned long data_) {
       unsigned int cmd = cmd_;
        struct timespec ts;
        unsigned int status;
       printk(KERN_ERR "Sample driver: Received command (0x%x)\n", cmd);
       return 1;
}


struct file_operations ecrNativeFops = {
   .owner = THIS_MODULE,
   .read=dev_read,
   .write =dev_write,
   .compat_ioctl = device_ioctl
};

static int __init ecr_helper_init(void)
{
   int ret, ecr_major;

   ret = alloc_chrdev_region(&mxc_char_dev, 1, 1,    ECR_NATIVE_DEVICE_NAME);
   ecr_major = MAJOR(mxc_char_dev);
   printk(KERN_ERR "Sample driver: major device num %d\n", ecr_major);
   if (ret < 0) {
       printk(KERN_ERR "Sample driver: can't get major ecr %d\n", ecr_major);
       goto err3;
   }

   cdev_init(&mxc_cdev, &ecrNativeFops);
   mxc_cdev.owner = THIS_MODULE;
   mxc_cdev.ops = &ecrNativeFops;
   ret = cdev_add(&mxc_cdev, mxc_char_dev, 1);
   if (ret) {
       printk(KERN_ERR "Sample driver: can't add ecr cdev\n");
       goto err2;
   }

   /* create class and device for udev information */
   mxc_class = class_create(THIS_MODULE, ECR_NATIVE_DEVICE_NAME);
   if (IS_ERR(mxc_class)) {
       printk(KERN_ERR "Sample driver: failed to create dev class\n");
       ret = -ENOMEM;
       goto err1;
   }

   mxc_class_dev = device_create(mxc_class, NULL, MKDEV(ecr_major, 1), NULL,
           ECR_NATIVE_DEVICE_NAME);
   if (IS_ERR(mxc_class_dev)) {
       printk(KERN_ERR "Sample driver: failed to create dev gpio class device\n");
       ret = -ENOMEM;
       goto err0;
   }

   return 0;
err0:
   class_destroy(mxc_class);
err1:
   cdev_del(&mxc_cdev);
err2:
   unregister_chrdev_region(mxc_char_dev, 1);
err3:
   return ret;
}
module_init(ecr_helper_init);

static void __exit ecr_helper_exit(void)
{
   /* destroy dev device class */
   device_destroy(mxc_class, MKDEV(MAJOR(mxc_char_dev), 1));
   class_destroy(mxc_class);

   /* Unregister the device */
   cdev_del(&mxc_cdev);
   unregister_chrdev_region(mxc_char_dev, 1);

}
module_exit(ecr_helper_exit);

static ssize_t dev_read(struct file *filp, char *buff, size_t len, loff_t *off)
{
	   gpio_export(GPIO_NUMBER,1);
	   gpio_direction_output(GPIO_NUMBER,1);
	   gpio_set_value(GPIO_NUMBER, 0);
	   gpio_unexport(GPIO_NUMBER);

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
	gpio_export(GPIO_NUMBER,1);
		   gpio_direction_output(GPIO_NUMBER,1);
		   gpio_set_value(GPIO_NUMBER, 1);
		   gpio_unexport(GPIO_NUMBER);
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
