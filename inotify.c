/**
 * @file	inotify.c
 * @author	Allek Mott <allekmott@gmail.com>
 * @date	10 September 2018
 * @brief	Implementation of fstalk inotify adapter
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/inotify.h>

#include "fstalk.h"
#include "inotify.h"

#define EVENT_SIZE (sizeof(struct inotify_event) + FILENAME_MAX + 1)
#define MAX_MOVE_WAIT 50

static int INOTIFY_FD = -1;

struct event_cache_node {
	struct event_cache_node *next;
	struct inotify_event *event;
};

static struct event_cache_node *event_cache = NULL;

static int get_inotify_fd() {
	if (INOTIFY_FD == -1) {
		INOTIFY_FD = inotify_init();
	}

	return INOTIFY_FD;
}

static struct event_cache_node *cache_get_last_node() {
	struct event_cache_node *node;

	if (event_cache == NULL) {
		return NULL;
	}

	for (node = event_cache; node->next != NULL; node = node->next) ;
	
	return node;
}

#define cache_available() (event_cache != NULL)

static int cache_add(struct inotify_event *event) {
	struct event_cache_node *node, *last_node;

	node = calloc(sizeof(struct event_cache_node), 1);
	if (node == NULL) {
		return errno;
	}

	node->event = event;

	last_node = cache_get_last_node();
	if (last_node == NULL) {
		last_node = node;
	} else {
		last_node->next = node;
	}

	return 0;
}

static struct inotify_event *cache_remove() {
	struct event_cache_node *node;
	struct inotify_event *event;

	if (!cache_available()) {
		node = NULL;
	} else {
		node = event_cache;
		event_cache = event_cache->next;
	}

	event = node->event;
	free(node);

	return event;
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

static struct inotify_event *await_inotify_event() {
	int fd;
	struct inotify_event *event;
	size_t n_bytes;

	if (cache_available()) {
		return cache_remove();
	}

	fd = get_inotify_fd();
	if (fd < 0) {
		perror("unable to initialise inotify");
		return NULL;
	}

	event = (struct inotify_event *) calloc(EVENT_SIZE, 1);
	if (event == NULL) {
		perror("unable to allocate memory for event");
		return NULL;
	}

	if ((n_bytes = read(fd, event, EVENT_SIZE)) == 0) {
		perror("unable to read from inotify");
		return NULL;
	}

	return event;
}

#define is_move(e) ((e)->mask | IN_MOVE)

static struct inotify_event *await_subsequent_move(uint32_t cookie) {
	int n_other_events;
	struct inotify_event *event;

	n_other_events = 0;

	do {
		event = await_inotify_event();
		if (event == NULL) {
			return NULL;
		}

		printf("%08x\n", event->mask);

		if (event->cookie == cookie) {
			return event;
		} else {
			if (cache_add(event) != 0) {
				perror("unable to append event to cache");
			}
		}

		++n_other_events;
	} while (n_other_events < MAX_MOVE_WAIT);

	return NULL;
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
	struct inotify_event *event, *event_partner;
	struct fstalk_event *fs_event;

	event = await_inotify_event();
	if (event == NULL) {
		perror("NULL event. wack.");
		return NULL;
	}

	if (is_move(event)) {
		event_partner = await_subsequent_move(event->cookie);
		if (event_partner == NULL) {
			perror("unable to obtain move details");

			free(event);
			return NULL;
		}

		fs_event = to_fstalk_event(event);
		memcpy(fs_event->destination,
				event_partner->name,
				event_partner->len);
		return fs_event;
	} else {
		fs_event = to_fstalk_event(event);
		free(event);

		return fs_event;
	}
}

