/**
 * @file	inotify.h
 * @author	Allek Mott <allekmott@gmail.com>
 * @date	11 September 2018
 * @brief	Interface for fstalk inotify adapter
 */

#ifndef __FSTALK_INOTIFY_H__
#define __FSTALK_INOTIFY_H__

#include "fstalk.h"

/*
 * Register a path to be stalked (watched).
 * @watch	struct fstalk_watch with path set to desired path
 * 
 * Returns pointer to modified watch on success, NULL on failure
 */
struct fstalk_watch *fstalk_register_watch(struct fstalk_watch *watch);

/*
 * Block & wait for an event to occur.
 *
 * Returns event structure on success, NULL on failure
 */
struct fstalk_event *fstalk_await_event();

#endif /* __FSTALK_INOTIFY_H__ */

