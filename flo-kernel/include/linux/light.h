#define TIME_INTERVAL  200 /* (ms) */
#define NOISE 10
#define WINDOW 20

/*
 * The data structure for passing light intensity data to the
 * kernel and storing the data in the kernel.
 */
struct light_intensity {
	int cur_intensity; /* scaled intensity as read from the light sensor */
};

/*
 * Defines a light event.
 *
 * Event is defined by a required intensity and frequency.
 */
struct event_requirements {
	/* scaled value of light intensity in centi-lux */
	int req_intensity;
	/* number of samples with intensity-noise > req_intensity */
	int frequency;
};
