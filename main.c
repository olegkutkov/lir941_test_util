#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "lir941r_user.h"

#define NUM_OF_CHANNELS 4

static volatile int run_program = 1;

static int fds[NUM_OF_CHANNELS];
static unsigned short ch_data[NUM_OF_CHANNELS];

static const unsigned short count_per_angle = 65535 / 360;


void intHandler(int n)
{
	run_program = 0;
}

int main()
{
	char devpath[14];
	unsigned int buf = 0;
	ssize_t rdres = 0;
	int i, acnt = 0;

	double jd;

	struct ln_lnlat_posn observer;

	struct lnh_equ_posn  hequ;
	struct ln_equ_posn equ_data;
	struct ln_hrz_posn altaz_data;

	observer.lat = lat;
	observer.lng = lon;

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

		ioctl(fds[i], LIR941_CHANNEL_SPEED, 66);  //  sys_clk = 33000000,  33000000 / 66 = 500000 = 0.5 Mhz
		ioctl(fds[i], LIR941_CHANNEL_PAUSE, 1155);   // 35 us, (35 / 33.3 * 1000) = 1155

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
					ch_data[i] = ((unsigned short)buf) / count_per_angle;
					continue;
				}
			}
 
			ch_data[i] = 0;
		}

		printf("CH0 RAW DATA = %d    CH1 RAW DATA = %d  CH2 RAW DATA = %d    CH3 RAW DATA = %d\n"
				, ch_data[0], ch_data[1], ch_data[2], ch_data[3]);

		usleep(500000);
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

	return 0;
}

