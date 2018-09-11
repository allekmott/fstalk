/**
 * @file	fstalk.h
 * @author	Allek Mott <allekmott@gmail.com>
 * @date	11 September 2018
 * @brief	Core fstalk defintions and data structures
 */

#ifndef __FSTALK_H__
#define __FSTALK_H__

#include <stdio.h>
#include <time.h>

/* Various types of filesystem events */
enum fstalk_event_type {
	FSTALK_EVENT_OPEN,	/* file opened */
	FSTALK_EVENT_MODIFY,	/* file modified */
	FSTALK_EVENT_CLOSE,	/* file closed */
	FSTALK_EVENT_ACCESS,	/* file accessed */
	FSTALK_EVENT_CREATE,	/* file created */
	FSTALK_EVENT_DELETE,	/* file deleted */
	FSTALK_EVENT_MOVE,	/* file moved */
};

/*
 * General event structure
 *
 * @type	Type of event that occured
 * @time	Time the event occured
 * @target	Name of file involved in event
 * @destination	(Only used in MOVEs) Filename to which target was relocated
 */
struct fstalk_event {
	enum fstalk_event_type type;
	time_t time;
	char target[FILENAME_MAX];
	char destination[FILENAME_MAX];
};

/*
 * General structure for watch handle
 *
 * @path	Path being watched
 * @ref		(Optional) reference to watch, provided upon registration
 */
struct fstalk_watch {
	const char *path;
	void *ref;
};

/*
 * Print string representation of an event to stdout
 *
 * @event	fstalk_event to be printed
 */
void fstalk_print_event(const struct fstalk_event *event);

#endif /* __FSTALK_H__ */

