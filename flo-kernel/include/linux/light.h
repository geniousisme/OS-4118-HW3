/*
 *Define time interval (ms)
 */
#define TIME_INTERVAL  200

/*
 * The data structure for passing light intensity data to the
 * kernel and storing the data in the kernel.
 */
struct light_intensity {
	int cur_intensity; /* scaled intensity as read from the light sensor */
};
