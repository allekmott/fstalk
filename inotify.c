/**
 * @file	inotify.c
 * @author	Allek Mott <allekmott@gmail.com>
 * @date	10 September 2018
 * @brief	Implementation of fstalk inotify adapter
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/inotify.h>

#include "fstalk.h"
#include "inotify.h"

#define EVENT_SIZE (sizeof(struct inotify_event) + FILENAME_MAX + 1)

static int INOTIFY_FD = -1;

static int get_inotify_fd() {
	if (INOTIFY_FD == -1) {
		INOTIFY_FD = inotify_init();
	}

	return INOTIFY_FD;
}

static enum fstalk_event_type to_fstalk_event_type(int in_event_type) {
	switch (in_event_type) {
	case IN_ACCESS:		return FSTALK_EVENT_ACCESS;
	case IN_CLOSE_WRITE:	return FSTALK_EVENT_CLOSE;
	case IN_CLOSE_NOWRITE:	return FSTALK_EVENT_CLOSE;
	case IN_OPEN:		return FSTALK_EVENT_OPEN;
	
	case IN_MOVED_FROM:
	case IN_MOVED_TO:
	case IN_MOVE_SELF:	return FSTALK_EVENT_MOVE;
	
	case IN_CREATE:		return FSTALK_EVENT_CREATE;
	
	case IN_DELETE:
	case IN_DELETE_SELF:	return FSTALK_EVENT_DELETE;
	
	case IN_ATTRIB:
	case IN_MODIFY:
	default:		return FSTALK_EVENT_MODIFY;
	}
}

static struct fstalk_event *to_fstalk_event(struct inotify_event *in_event) {
	struct fstalk_event *fs_event;

	fs_event = calloc(sizeof(struct fstalk_event), 1);
	if (fs_event == NULL) {
		return NULL;
	}

	fs_event->type = to_fstalk_event_type(in_event->mask);
	fs_event->time = time(NULL);

	memcpy(fs_event->target, in_event->name, in_event->len);
	if (fs_event->type == FSTALK_EVENT_MOVE) {
		memcpy(fs_event->target, in_event->name, in_event->len);
	}

	return fs_event;
}

struct fstalk_watch *fstalk_register_watch(struct fstalk_watch *watch) {
	int fd;
	int watch_desc;

	fd = get_inotify_fd();
	if (fd < 0) {
		return NULL;
	}	

	watch_desc = inotify_add_watch(fd, watch->path, IN_ALL_EVENTS);
	if (watch_desc < 0) {
		return NULL;
	}

	watch->ref = (void *) ((long) watch_desc);
	return watch;
}

struct fstalk_event *fstalk_await_event() {
	int fd;
	struct inotify_event *event;
	struct fstalk_event *fs_event;
	ssize_t n_bytes;

	fd = get_inotify_fd();
	if (fd < 0) {
		perror("unable to initialise inotify");
		return NULL;
	}

	event = (struct inotify_event *) calloc(EVENT_SIZE, 1);
	if (event == NULL) {
		perror("unable to allocate space for event");
		return NULL;
	}

	if ((n_bytes = read(fd, event, EVENT_SIZE)) < 0) {
		perror("unable to read from inotify");
		return NULL;
	}

	fs_event = to_fstalk_event(event);
	free(event);

	return fs_event;
}

