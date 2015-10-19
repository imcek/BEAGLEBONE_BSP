/*
===============================================================================
Driver Name		:		event_rotary
Author			:		RECEP YIGITBASI
License			:		GPL
Description		:		LINUX DEVICE DRIVER PROJECT
===============================================================================
*/

#include"event_rotary.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("RECEP YIGITBASI");

//static struct timer_list my_timer;
//
//void my_timer_callback( unsigned long data )
//{
//  printk( "my_timer_callback called (%ld).\n", jiffies );
//  gpio_set_value(66, 1);
//}

struct event_rotary_private {
	struct input_dev *dev;
	int irq;
	unsigned int irq_a;
	unsigned int irq_b;

	unsigned int axis;
	unsigned int pos;

	unsigned char dir;	/* 0 - clockwise, 1 - CCW */

	bool armed;
	char last_stable;

};

struct rotary_encoder_data {
	unsigned int steps;
	unsigned int axis;
	unsigned int gpio_a;
	unsigned int gpio_b;
	unsigned int inverted_a;
	unsigned int inverted_b;
	bool relative_axis;
	bool rollover;
	bool half_period;
};

unsigned int gpio_button=BUTTON_1;
unsigned int irq_button;

struct event_rotary_private *event_rotary_priv;
struct rotary_encoder_data * rot_data;
short int irq_any_gpio    = 0;

static int event_rotary_open(struct input_dev *dev)
{

	struct event_rotary_private *priv;

	priv= input_get_drvdata(dev);

	PINFO("in input open() function \n");
	return 0;
}

static void event_rotary_close(struct input_dev *dev)
{
	struct event_rotary_private *priv;

	priv= input_get_drvdata(dev);

	PINFO("in input close() function \n");
}

//static irqreturn_t r_irq_handler(int irq, void *dev_id, struct pt_regs *regs) {
//
//   unsigned long flags;
//
//   // disable hard interrupts (remember them in flag 'flags')
//   local_irq_save(flags);
//
//   // NOTE:
//   // Anonymous Sep 17, 2013, 3:16:00 PM:
//   // You are putting printk while interupt are disabled. printk can block.
//   // It's not a good practice.
//   //
//   // hardware.coder:
//   // http://stackoverflow.com/questions/8738951/printk-inside-an-interrupt-handler-is-it-really-that-bad
//
//   input_report_key(event_rotary_priv->dev, KEY_UP, 1); /* keypress */
//   input_report_key(event_rotary_priv->dev, KEY_UP, 0); /* keypress */
//   input_sync(event_rotary_priv->dev);
//
//   printk(KERN_NOTICE "Interrupt [%d] for device %s was triggered !.\n",
//          irq, (char *) dev_id);
//
//   // restore hard interrupts
//   local_irq_restore(flags);
//
//   return IRQ_HANDLED;
//}

static void initialize_rotary_device(void){
	rot_data->axis = ABS_X;
	rot_data->steps = 24;
	rot_data->relative_axis = false;
	rot_data->gpio_a = GPIO_67;
	rot_data->gpio_b = GPIO_68;
	rot_data->half_period = true; // 2 adinmda 1 calisir.
	/* rollover = true;
	 * 0-steps araliginda doner. steps den sonra 0 a gecer*/
	/* rollover = false;
	 * 0-steps araliginda doner. steps de ve 0 da kalir
	 */
	rot_data->rollover= false;
	rot_data->inverted_a	= 0;
	rot_data->inverted_b	= 0;
}


//void gpio_config(void){
//	if (gpio_request(GPIO_67, GPIO_ANY_GPIO_DESC)) {
//	  printk("GPIO request faiure: %s\n", GPIO_ANY_GPIO_DESC);
//	  return;
//	}
//
//	if ( (irq_any_gpio = gpio_to_irq(GPIO_67)) < 0 ) {
//	  printk("GPIO to IRQ mapping faiure %s\n", GPIO_ANY_GPIO_DESC);
//	  return;
//	}
//
//	printk(KERN_NOTICE "Mapped int %d\n", irq_any_gpio);
//
//	if (request_irq(irq_any_gpio,
//				   (irq_handler_t ) r_irq_handler,
//				   IRQF_TRIGGER_FALLING,
//				   GPIO_ANY_GPIO_DESC,
//				   GPIO_ANY_GPIO_DEVICE_DESC)) {
//	  printk("Irq Request failure\n");
//	  return;
//	}
//}

//void r_int_release(void) {
//   free_irq(irq_any_gpio, GPIO_ANY_GPIO_DEVICE_DESC);
//   gpio_free(GPIO_67);
//   return;
//}

static int rotary_encoder_get_state(void)
{
	int a = !!gpio_get_value(rot_data->gpio_a);
	int b = !!gpio_get_value(rot_data->gpio_b);

	a ^= rot_data->inverted_a;
	b ^= rot_data->inverted_b;

	return ((a << 1) | b);
}

static void rotary_encoder_report_event(void)
{

	if (rot_data->relative_axis) {
		input_report_rel(event_rotary_priv->dev,
				 rot_data->axis, event_rotary_priv->dir ? -1 : 1);
		PINFO("ROTARY ENCODER DIR:%d\n",event_rotary_priv->dir);
	} else {
		unsigned int pos = event_rotary_priv->pos;

		if (event_rotary_priv->dir) {
			/* turning counter-clockwise */
			if (rot_data->rollover)
				pos += rot_data->steps;
			if (pos)
				pos--;
		} else {
			/* turning clockwise */
			if (rot_data->rollover || pos < rot_data->steps)
				pos++;
		}

		if (rot_data->rollover)
			pos %= rot_data->steps;

		event_rotary_priv->pos = pos;
		PINFO("ENCODER POSITION:%d\n", pos);
//		gpio_set_value(66, 0);
//		int res = mod_timer( &my_timer, jiffies + msecs_to_jiffies(5000) );
//			if (res) printk("Error in mod_timer\n");
		input_report_abs(event_rotary_priv->dev, rot_data->axis, event_rotary_priv->pos);
	}

	input_sync(event_rotary_priv->dev);
}

static void button_report_event(void){
	input_report_key(event_rotary_priv->dev, KEY_ENTER, 1); /* keypress */
	input_report_key(event_rotary_priv->dev, KEY_ENTER, 0); /* keypress */
	input_sync(event_rotary_priv->dev);
//	gpio_set_value(66, 0);
//	int res = mod_timer( &my_timer, jiffies + msecs_to_jiffies(5000) );
//	if (res) printk("Error in mod_timer\n");
}

static irqreturn_t button_irq(int irq, void *dev_id){
	unsigned long flags;
	// disable hard interrupts (remember them in flag 'flags')
	local_irq_save(flags);
	PINFO("BUTTON IRQ RECEIVED\n");
	button_report_event();
	// restore hard interrupts
	local_irq_restore(flags);
	return IRQ_HANDLED;
}

static irqreturn_t rotary_encoder_half_period_irq(int irq, void *dev_id)
{
	int state;

	state = rotary_encoder_get_state();

	switch (state) {
	case 0x00:
	case 0x03:
		if (state != event_rotary_priv->last_stable) {
			rotary_encoder_report_event();
			event_rotary_priv->last_stable = state;
		}
		break;

	case 0x01:
	case 0x02:
		event_rotary_priv->dir = (event_rotary_priv->last_stable + state) & 0x01;
		break;
	}

	return IRQ_HANDLED;
}

static irqreturn_t rotary_encoder_irq(int irq, void *dev_id)
{
	int state;

	state = rotary_encoder_get_state();

	switch (state) {
	case 0x0:
		if (event_rotary_priv->armed) {
			rotary_encoder_report_event();
			event_rotary_priv->armed = false;
		}
		break;

	case 0x1:
	case 0x2:
		if (event_rotary_priv->armed)
			event_rotary_priv->dir = state - 1;
		break;

	case 0x3:
		event_rotary_priv->armed = true;
		break;
	}

	return IRQ_HANDLED;
}

static int __init event_rotary_init(void)
{
	/* TODO Auto-generated Function Stub */

	int res;
	int err;
	irq_handler_t handler;
	irq_handler_t button_handler;
	event_rotary_priv = kzalloc(sizeof(struct event_rotary_private),GFP_KERNEL);
	rot_data = kzalloc(sizeof(struct rotary_encoder_data),GFP_KERNEL);
	event_rotary_priv->dev = input_allocate_device();

	if (!event_rotary_priv || !event_rotary_priv->dev) {
			err = -ENOMEM;
			goto exit_free_mem;
	}

	event_rotary_priv->dev->name = DRIVER_NAME;
	event_rotary_priv->dev->open = event_rotary_open; 	
	event_rotary_priv->dev->close = event_rotary_close;

	initialize_rotary_device();

	if (rot_data->relative_axis) {
		event_rotary_priv->dev->evbit[0] = BIT_MASK(EV_REL) | BIT_MASK(EV_KEY);
		event_rotary_priv->dev->relbit[0] = BIT_MASK(rot_data->axis);
	} else {
		event_rotary_priv->dev->evbit[0] = BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
		input_set_abs_params(event_rotary_priv->dev,
					 rot_data->axis, 0, rot_data->steps, 0, 1);
	}
	set_bit(KEY_ENTER,  event_rotary_priv->dev->keybit);
	/* request the GPIOs */
	err = gpio_request_one(rot_data->gpio_a, GPIOF_IN, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO %d\n", rot_data->gpio_a);
		goto exit_free_mem;
	}

	err = gpio_request_one(rot_data->gpio_b, GPIOF_IN, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO %d\n", rot_data->gpio_b);
		goto exit_free_gpio_a;
	}

	err = gpio_request_one(gpio_button, GPIOF_IN, GPIO_ANY_GPIO_DESC);
	if (err) {
		printk("unable to request GPIO Button%d\n", gpio_button);
		goto exit_free_gpio_b;
	}

	event_rotary_priv->irq_a = gpio_to_irq(rot_data->gpio_a);
	event_rotary_priv->irq_b = gpio_to_irq(rot_data->gpio_b);
	irq_button = gpio_to_irq(gpio_button);

	/* request the IRQs */
	if (rot_data->half_period) {
		handler = &rotary_encoder_half_period_irq;
		event_rotary_priv->last_stable = rotary_encoder_get_state();
	} else {
		handler = &rotary_encoder_irq;
	}
	button_handler = &button_irq;

	err = request_irq(event_rotary_priv->irq_a, handler,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			  GPIO_ANY_GPIO_DESC, GPIO_ANY_GPIO_DEVICE_DESC);
	if (err) {
		printk("unable to request IRQ %d\n", event_rotary_priv->irq_a);
		goto exit_free_gpio_button;
	}
	err = request_irq(event_rotary_priv->irq_b, handler,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			  GPIO_ANY_GPIO_DESC, GPIO_ANY_GPIO_DEVICE_DESC);

	if (err) {
		printk("unable to request IRQ %d\n", event_rotary_priv->irq_b);
		goto exit_free_irq_a;
	}

	err = request_irq(irq_button, button_handler,
					  IRQF_TRIGGER_FALLING,
					  GPIO_ANY_GPIO_DESC, GPIO_ANY_GPIO_DEVICE_DESC);

	if (err) {
			printk("unable to request IRQ %d\n", irq_button);
			goto exit_free_irq_b;
		}
	gpio_set_debounce(gpio_button, BUTTON_DEBOUNCE) ; //in usec
	res = input_register_device(event_rotary_priv->dev);
	if (err) {
		printk("failed to register input device\n");
		goto exit_free_irq_button;
	}
	PINFO("INIT\n");

//	printk("Timer module installing\n");
//
//	// my_timer.function, my_timer.data
//	setup_timer( &my_timer, my_timer_callback, 0 );
//
////	printk( "Starting timer to fire in 200ms (%ld)\n", jiffies );
////	res = mod_timer( &my_timer, jiffies + msecs_to_jiffies(5000) );
////	if (res) printk("Error in mod_timer\n");
//
//	res = gpio_request_one(66, GPIOF_OUT_INIT_LOW, GPIO_ANY_GPIO_DESC);
//	if (res) {
//		printk("unable to request GPIO Button%d\n", 66);
//	}else{
//		res= gpio_direction_output(66, 0);
//	}
//	gpio_set_value(66, 0);

	return 0;
exit_free_irq_button:
	free_irq(irq_button, event_rotary_priv);
exit_free_irq_b:
	free_irq(event_rotary_priv->irq_b, event_rotary_priv);
exit_free_irq_a:
	free_irq(event_rotary_priv->irq_a, event_rotary_priv);
exit_free_gpio_button:
	gpio_free(gpio_button);
exit_free_gpio_b:
	gpio_free(rot_data->gpio_b);
exit_free_gpio_a:
	gpio_free(rot_data->gpio_a);
exit_free_mem:
	input_free_device(event_rotary_priv->dev);
	kfree(event_rotary_priv);
	return err;
}

static void __exit event_rotary_exit(void)
{	
	/* TODO Auto-generated Function Stub */
	free_irq(event_rotary_priv->irq_a, GPIO_ANY_GPIO_DEVICE_DESC);
	free_irq(event_rotary_priv->irq_b, GPIO_ANY_GPIO_DEVICE_DESC);
	free_irq(irq_button, GPIO_ANY_GPIO_DEVICE_DESC);
	gpio_free(rot_data->gpio_a);
	gpio_free(rot_data->gpio_b);
	gpio_free(gpio_button);
//	gpio_free(66);
	input_unregister_device(event_rotary_priv->dev);
	kfree(event_rotary_priv);
	kfree(rot_data);
}

module_init(event_rotary_init);
module_exit(event_rotary_exit);

