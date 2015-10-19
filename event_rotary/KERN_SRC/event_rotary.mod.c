#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xa866555f, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x9914b780, __VMLINUX_SYMBOL_STR(input_unregister_device) },
	{ 0xdd905392, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xb1882998, __VMLINUX_SYMBOL_STR(input_free_device) },
	{ 0xfe990052, __VMLINUX_SYMBOL_STR(gpio_free) },
	{ 0xaa79c3d3, __VMLINUX_SYMBOL_STR(input_register_device) },
	{ 0x6c1b09ab, __VMLINUX_SYMBOL_STR(gpiod_set_debounce) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x8e9c7933, __VMLINUX_SYMBOL_STR(gpiod_to_irq) },
	{ 0x403f9529, __VMLINUX_SYMBOL_STR(gpio_request_one) },
	{ 0x35bd133, __VMLINUX_SYMBOL_STR(input_set_abs_params) },
	{ 0xd78a5654, __VMLINUX_SYMBOL_STR(input_allocate_device) },
	{ 0xaab1b144, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0x763123ae, __VMLINUX_SYMBOL_STR(dev_get_drvdata) },
	{ 0xf7802486, __VMLINUX_SYMBOL_STR(__aeabi_uidivmod) },
	{ 0x3e9247e0, __VMLINUX_SYMBOL_STR(input_event) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x5f419491, __VMLINUX_SYMBOL_STR(gpiod_get_raw_value) },
	{ 0xf816c866, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "25567DEACD7C937CA259792");
