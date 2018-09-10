/**
 * @file	main.c
 * @author	Allek Mott
 * @email	<allekmott@gmail.com>
 * @brief	Entry point for fstalk cli
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(struct inotify_event) + FILENAME_MAX + 1)

static int usage(const char *cmd) {
	printf("Usage: %s <path>\n", cmd);
	return 0;
}

static size_t sizeof_inotify_event(const struct inotify_event *event) {
	return sizeof(int) + (3 * sizeof(uint32_t))
			+ ((event->len + 1)  * sizeof(char));
}

static void *await_event(int fd, void *event_buf) {
	ssize_t n_bytes;

	memset(event_buf, 0x00, EVENT_SIZE);

	if ((n_bytes = read(fd, event_buf, EVENT_SIZE)) < 0) {
		return NULL;
	}

	return event_buf;
}

static void fap_events(const int fd) {
	char event_buf[EVENT_SIZE];
	struct inotify_event *event;

	do {
		event = (struct inotify_event *) await_event(fd, event_buf);
		if (!event) {
			continue;
		}

		printf("%s\n", event->name);
	} while (1);
}

int main(int argc, const char *argv[]) {
	int inotify_fd;
	int watch_desc;

	const char *watch_path;

	if (argc < 2) {
		watch_path = ".";
	/*	return usage(argv[0]); */
	} else {
		watch_path = argv[1];
	}

	printf("to stalk: %s\n", watch_path);
	
	inotify_fd = inotify_init();
	if (inotify_fd < 0) {
		perror("unable to initialise inotify");
		return errno;
	}

	watch_desc = inotify_add_watch(inotify_fd, watch_path, IN_ALL_EVENTS);
	if (watch_desc < 0) {
		perror("unable to register watch");
		return errno;
	}

	fap_events(inotify_fd);

	close(inotify_fd);
	return 0;
}

