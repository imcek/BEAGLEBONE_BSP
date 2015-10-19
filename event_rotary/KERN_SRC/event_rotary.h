
#define DRIVER_NAME "event_rotary"
#define PDEBUG(fmt,args...) printk(KERN_DEBUG"%s:"fmt,DRIVER_NAME, ##args)
#define PERR(fmt,args...) printk(KERN_ERR"%s:"fmt,DRIVER_NAME,##args)
#define PINFO(fmt,args...) printk(KERN_INFO"%s:"fmt,DRIVER_NAME, ##args)
#include<linux/init.h>
#include<linux/input.h>
#include<linux/interrupt.h>
#include<linux/module.h>
#include<linux/slab.h>
#include<linux/gpio.h>
#include<linux/types.h>
#include <linux/timer.h>

void gpio_config(void);
void r_int_release(void);
static void initialize_rotary_device(void);
static int rotary_encoder_get_state(void);
static void rotary_encoder_report_event(void);
static void button_report_event(void);

#define GPIO_67 67
#define GPIO_68 68
#define BUTTON_1 65
#define BUTTON_DEBOUNCE 10000

#define GPIO_ANY_GPIO_DESC           "Some gpio pin description"
#define GPIO_ANY_GPIO_DEVICE_DESC    "some_device"
