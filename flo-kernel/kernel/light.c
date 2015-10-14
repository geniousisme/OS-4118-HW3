#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/light.h>

struct light_intensity intensity;

SYSCALL_DEFINE1(set_light_intensity, struct light_intensity __user *,
	user_light_intensity)
{
	if (copy_from_user(&intensity, user_light_intensity,
		sizeof(struct light_intensity)))
		return -EINVAL;
	return 0;
}

SYSCALL_DEFINE1(get_light_intensity, struct light_intensity __user *,
	user_light_intensity)
{
	if (copy_to_user(user_light_intensity, &intensity,
		sizeof(struct light_intensity)))
		return -EINVAL;
	return 0;
}

SYSCALL_DEFINE1(light_evt_create, struct event_requirements __user *,
	intensity_params)
{
	return 0;
}

SYSCALL_DEFINE1(light_evt_wait, int, event_id)
{
	return 0;
}

SYSCALL_DEFINE1(light_evt_signal, struct light_intensity __user *,
	user_light_intensity)
{
	return 0;
}

SYSCALL_DEFINE1(light_evt_destroy, int, event_id)
{
	return 0;
}
