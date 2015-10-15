#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "light.h"

#define PRINT_ERRNO() (printf("error: %s\n", strerror(errno)))

#define LOW 0
#define MEDIUM 100
#define HIGH 150
#define N 1
#define SLEEP (60 * 1000 * 1000)
#define WINDOW 20

char *msg(int intensity)
{
	if (intensity == LOW)
		return "low";
	else if (intensity == MEDIUM)
		return "medium";
	else if (intensity == HIGH)
		return "high";
	return "unknown";
}

void wait_on_light_event(int intensity)
{
	pid_t pid = fork();

	if (pid < 0) {
		PRINT_ERRNO();
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		int event_id;
		struct event_requirements req = {
			.req_intensity = intensity,
			.frequency = WINDOW / 2
		};

		printf("process %d starts\n", getpid());
		event_id = syscall(380, &req);
		printf("process %d waits on event %d\n", getpid(), event_id);
		syscall(381, event_id);
		printf("%d detected a %s intensity event\n", getpid(),
							msg(intensity));
		exit(EXIT_SUCCESS);
	}
}

int main(void)
{
	int i;

	for (i = 0; i < N; i++) {
		wait_on_light_event(LOW);
		wait_on_light_event(MEDIUM);
		wait_on_light_event(HIGH);
	}
	usleep(SLEEP);
	return 0;
}
