#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include "lir941r_user.h"

#define NUM_OF_CHANNELS 4

static volatile int run_program = 1;

static int fds[NUM_OF_CHANNELS];
static unsigned int ch_data[NUM_OF_CHANNELS];

void intHandler(int n)
{
	run_program = 0;
}

int main()
{
	char devpath[14];
	unsigned int buf = 0;
	ssize_t rdres = 0;
	int i;

	printf("Starting program...\n");

	for (i = 0; i < NUM_OF_CHANNELS; ++i) {
		sprintf(devpath, "/dev/lir941r-%d", i);
		devpath[14] = 0;

		printf("Open %s ", devpath);

		fds[i] = open(devpath, O_RDWR);

		if (fds[i] == -1) {
			printf(" -- failed! Error: %s\n", strerror(errno));
		} else {
			printf(" -- Ok\n");
		}
	}

	printf("\n");

	for (i = 0; i < NUM_OF_CHANNELS; i++) {
		if (ioctl(fds[i], LIR941_CHANNEL_DATAWIDTH, 16) != 0) { // set sensor data width
			printf("Failed to set LIR941_CHANNEL_DATAWIDTH, fd = %d, devnode = %d, error: %s\n", fds[i], i, strerror(errno));
		}

		printf("Starting data acquisition for channel %d\n", i);

		ioctl(fds[i], LIR941_START_CHANNEL_POLLING, 0);
	}

	signal(SIGINT, intHandler);

	printf("\n");

	while (run_program) {
		for (i = 0; i < NUM_OF_CHANNELS; i++) {
			if (fds[i] != -1) {
				rdres = read(fds[i], (void*)&buf, sizeof(buf));

				if (rdres == sizeof(buf)) {
					ch_data[i] = buf;
					continue;
				}
			}
 
			ch_data[i] = 0;
		}

		printf("\rCH0 RAW DATA = %d    CH1 RAW DATA = %d    CH2 RAW DATA = %d    CH3 RAW DATA = %d"
				, ch_data[0], ch_data[1], ch_data[2], ch_data[3]);

		fflush(stdout);

		sleep(1);
	}

	printf("\n\n");

	for (i = 0; i < NUM_OF_CHANNELS; ++i) {
		ioctl(fds[i], LIR941_STOP_CHANNEL_POLLING, 0);

		if (fds[i] != -1) {
			printf("Close devnode %d\n", i);
			close(fds[i]);
		}
	}

	printf("\nProgram finished\n");
}

