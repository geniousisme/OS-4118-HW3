#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/light.h>

SYSCALL_DEFINE1(set_light_intensity, struct light_intensity __user *,
	user_light_intensit)
{
	printk("set_light_intensity is called!");
	return 0;
}

SYSCALL_DEFINE1(get_light_intensity, struct light_intensity __user *,
	user_light_intensity)
{
	printk("get_light_intensity is called!");
	return 0;
}
