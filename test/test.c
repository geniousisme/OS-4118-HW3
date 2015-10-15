#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "light.h"

#define PRINT_ERRNO() (printf("error: %s\n", strerror(errno)))

#define LOW 1000
#define MEDIUM 2000
#define HIGH 3000
#define N 3
#define SLEEP (60 * 1000 * 1000)
#define WINDOW 20

void wait_on_light_event(int event_id, char *msg)
{
	pid_t pid = fork();

	if (pid < 0) {
		PRINT_ERRNO();
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		printf("process %d waits on event %d\n", getpid(), event_id);
		syscall(381, event_id);
		printf("%d detected a %s intensity event\n", getpid(), msg);
		exit(EXIT_SUCCESS);
	}
}

int main(void)
{
	int i, id_low, id_medium, id_high;
	struct event_requirements req = {
		.frequency = WINDOW / 2
	};

	req.req_intensity = LOW;
	id_low = syscall(380, &req);
	req.req_intensity = MEDIUM;
	id_medium = syscall(380, &req);
	req.req_intensity = HIGH;
	id_high = syscall(380, &req);

	for (i = 0; i < N; i++) {
		wait_on_light_event(id_low, "low");
		wait_on_light_event(id_medium, "medium");
		wait_on_light_event(id_high, "high");
	}
	usleep(SLEEP);
	syscall(383, id_low);
	syscall(383, id_medium);
	syscall(383, id_high);
	return 0;
}
