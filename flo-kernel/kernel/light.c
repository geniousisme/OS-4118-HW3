#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/light.h>

struct light_intensity intensity;
/* light history, use light_history[(history_count++) % WINDOW] to update */
int light_history[WINDOW], history_count = 0;

int event_check(struct event_requirements req)
{
	/* given a requirement, check if it's true based on the history */
	/* should hold the lock for history */
	int i, surpassed = 0;

	for (i = 0; i < history_count && i < WINDOW; i++)
		if (light_history[i] >= req.req_intensity - NOISE)
			surpassed++;

	return surpassed >= req.frequency ? 1 : 0;
}

/*
 * Set current ambient intensity in the kernel.
 *
 * The parameter user_light_intensity is the pointer to the address
 * where the sensor data is stored in user space. Follow system call
 * convention to return 0 on success and the appropriate error value
 * on failure.
 *
 * syscall number 378
 */
SYSCALL_DEFINE1(set_light_intensity, struct light_intensity __user *,
	user_light_intensity)
{
	if (copy_from_user(&intensity, user_light_intensity,
		sizeof(struct light_intensity)))
		return -EINVAL;
	return 0;
}

/*
 * Retrive the scaled intensity set in the kernel.
 *
 * The same convention as the previous system call but
 * you are reading the value that was just set.
 * Handle error cases appropriately and return values according to convention.
 * The calling process should provide memory in userspace to return the
 * intensity.
 *
 * syscall number 379
 */
SYSCALL_DEFINE1(get_light_intensity, struct light_intensity __user *,
	user_light_intensity)
{
	if (copy_to_user(user_light_intensity, &intensity,
		sizeof(struct light_intensity)))
		return -EINVAL;
	return 0;
}

/*
 * Create an event based on light intensity.
 *
 * If frequency exceeds WINDOW, cap it at WINDOW.
 * Return an event_id on success and the appropriate error on failure.
 *
 * system call number 380
 */
SYSCALL_DEFINE1(light_evt_create, struct event_requirements __user *,
	intensity_params)
{
	return 0;
}

/*
 * Block a process on an event.
 *
 * It takes the event_id as parameter. The event_id requires verification.
 * Return 0 on success and the appropriate error on failure.
 *
 * system call number 381
 */
SYSCALL_DEFINE1(light_evt_wait, int, event_id)
{
	return 0;
}

/*
 * The light_evt_signal system call.
 *
 * Takes sensor data from user, stores the data in the kernel,
 * and notifies all open events whose
 * baseline is surpassed.  All processes waiting on a given event
 * are unblocked.
 *
 * Return 0 success and the appropriate error on failure.
 *
 * system call number 382
 */
SYSCALL_DEFINE1(light_evt_signal, struct light_intensity __user *,
	user_light_intensity)
{
	return 0;
}

/*
 * Destroy an event using the event_id.
 *
 * Return 0 on success and the appropriate error on failure.
 *
 * system call number 383
 */
SYSCALL_DEFINE1(light_evt_destroy, int, event_id)
{
	return 0;
}
