/**
 * @file	fstalk.c
 * @author	Allek Mott <allekmott@gmail.com>
 * @date	11 September 2018
 * @brief	Entry point for fstalk CLI
 */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "fstalk.h"
#include "inotify.h"

static int usage(const char *cmd) {
	printf("Usage: %s <path>\n", cmd);
	return 0;
}

static const char *S_FSTALK_EVENT_TYPES[] = {
	"open", "modify", "close", "access", "create", "delete", "move"
};
#define fstalk_s_event_type(type) (S_FSTALK_EVENT_TYPES[(type)])

int main(int argc, const char *argv[]) {
	struct fstalk_watch watch;
	struct fstalk_event *event;
	const char *filename;

	int ret;

	if (argc < 2) {
		return usage(argv[0]);
	}

	ret = 0;
	filename = argv[1];

	printf("to stalk: %s\n", filename);

	watch.path = filename;
	if (fstalk_register_watch(&watch) == NULL) {
		ret = errno;
		perror("unable to register watch");

		goto done;
	}

	do {
		event = fstalk_await_event();
		if (event == NULL) {
			ret = errno;
			perror("unable to query event");
			
			goto done;
		}

		printf("%s %s%s\n", fstalk_s_event_type(event->type),
				filename, event->target);
		free(event);
	} while (1);

done:
	return ret;
}

