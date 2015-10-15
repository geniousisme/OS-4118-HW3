#include <linux/idr.h>
#include <linux/light.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>

struct light_intensity intensity;
struct idr *event_id_table;
spinlock_t idr_lock;
int first_event = 0;
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
 * The calling process should provide memory in userspace to return the intensity.
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
	int event_id;
	int result;
	struct event_requirements *toadd = kmalloc(sizeof(struct event_requirements), GFP_KERNEL);
	if(copy_from_user(toadd, intensity_params,
		sizeof(struct event_requirements)))
		return -EINVAL;
	if(first_event == 0)
		idr_init(event_id_table);
	do{	
		if(idr_pre_get(event_id_table, GFP_KERNEL) ==0)
			return -ENOMEM;	
		spin_lock(&idr_lock);
		result = idr_get_new(event_id_table, toadd, &event_id);
		spin_unlock(&idr_lock);
	}while(result == -EAGAIN);
	return event_id;
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
	int id;
	if (copy_from_user(id, event_id,
		sizeof(int)))
		return -EINVAL;
	spin_lock(&idr_lock);
	if(idr_find(event_id_table, id))
		idr_remove(event_id_table, id);
	spin_unlock(&idr_lock);
	return 0;
}
