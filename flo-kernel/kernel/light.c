#include <linux/idr.h>
#include <linux/light.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

static struct light_intensity intensity = { .cur_intensity = 0 };
static int light_history[WINDOW], history_count;

spinlock_t idr_lock;

DEFINE_IDR(events);
DEFINE_RWLOCK(history_rwlock); /* see LKD page 188 for reader writer lock */
DEFINE_RWLOCK(intensity_rwlock);
DECLARE_WAIT_QUEUE_HEAD(queue);

static int event_check(int event_id)
{
	/* given a event_id, check if it's true based on the history */
	/* also return true if the event has been removed */
	int i, surpassed = 0;
	struct event_requirements *req;

	spin_lock(&idr_lock);
	req = idr_find(&events, event_id);
	spin_unlock(&idr_lock);

	/* Check if the id was removed */
	/* This is correct as long as we don't reuse event_id */
	if (req == NULL)
		return 1;

	read_lock(&history_rwlock);
	for (i = 0; i < WINDOW && i < history_count; i++)
		if (light_history[i] > req->req_intensity - NOISE)
			surpassed++;
	read_unlock(&history_rwlock);

	return surpassed >= req->frequency;
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
	write_lock(&intensity_rwlock);
	if (copy_from_user(&intensity, user_light_intensity,
		sizeof(struct light_intensity))) {
		write_unlock(&intensity_rwlock);
		return -EINVAL;
	}
	write_unlock(&intensity_rwlock);
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
	read_lock(&intensity_rwlock);
	if (copy_to_user(user_light_intensity, &intensity,
		sizeof(struct light_intensity))) {
		read_unlock(&intensity_rwlock);
		return -EINVAL;
	}
	read_unlock(&intensity_rwlock);
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
	int event_id, ret;
	static int firsttime = 1, max_id;
	struct event_requirements *req;

	req = kmalloc(sizeof(struct event_requirements), GFP_KERNEL);
	if (req == NULL)
		return -ENOMEM;

	if (copy_from_user(req, intensity_params,
			sizeof(struct event_requirements))) {
		kfree(req);
		return -EINVAL;
	}

	spin_lock(&idr_lock);
	if (firsttime) {
		idr_init(&events);
		firsttime = 0;
	}
	spin_unlock(&idr_lock);

	/* see LKD page 102 */
	/* asssign strictly increasing event_id */
	do {
		/* cannot hold the lock while allocating memory */
		if (!idr_pre_get(&events, GFP_KERNEL)) {
			kfree(req);
			return -ENOSPC;
		}
		spin_lock(&idr_lock);
		ret = idr_get_new_above(&events, req, max_id, &event_id);
		if (ret == -EAGAIN)
			spin_unlock(&idr_lock);
	} while (ret == -EAGAIN);

	if (!ret)
		max_id += 1;
	spin_unlock(&idr_lock);

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
	/* see LKD page 59 */
	DEFINE_WAIT(wait);

	add_wait_queue(&queue, &wait);
	while (!event_check(event_id)) {
		prepare_to_wait(&queue, &wait, TASK_INTERRUPTIBLE);
		if (signal_pending(current))
			break;
		schedule();
	}
	finish_wait(&queue, &wait);

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
	struct light_intensity intensity;

	if (copy_from_user(&intensity, user_light_intensity,
			sizeof(struct light_intensity)))
		return -EINVAL;

	write_lock(&history_rwlock);
	light_history[(history_count++) % WINDOW] = intensity.cur_intensity;
	write_unlock(&history_rwlock);

	/* an optimization is to wake up only if the new intensity is larger */
	wake_up(&queue);

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
	struct event_requirements *req;

	spin_lock(&idr_lock);
	req = idr_find(&events, event_id);
	spin_unlock(&idr_lock);

	/* unlock and lock would be efficient if we are trying to destroy */
	/* a lot of events that are not exist */
	if (req != NULL) {
		spin_lock(&idr_lock);
		/* idr_remove() will free req */
		idr_remove(&events, event_id);
		spin_unlock(&idr_lock);

		/* this is inefficient since it is sufficient to  wake up */
		/* the processes that are waiting on this event */
		wake_up(&queue);
	}
	return 0;
}
