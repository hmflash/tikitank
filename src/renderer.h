#pragma once

struct renderer {
	const char      *name;
	int              fd;
	char            *buf1;
	char            *buf2;
	size_t           buflen;

	int (*writefn)(int, const char*, size_t);

	pthread_t        thread;
	pthread_mutex_t  mutex;
	pthread_cond_t   cond;
	volatile int     exit;
};

/*
 * Simple double buffering wrapper.
 * 
 * Constructed with two buffers, a file descriptor
 * and a write function.
 *
 * On success, start is set to be the initial
 * buffer to write data into.
 *
 * Returns 0 on success and -1 on failure.
 */
int renderer_init(struct renderer* r,
                  const char* name,
                  int fd,
                  char* buf1,
                  char* buf2,
                  size_t buflen,
                  int (*writefn)(int, const char*, size_t),
                  char** start);

/*
 * Perform a buffer swap.
 *
 * The write function pointer gets called with the
 * completed memory buffer and the buffer used for
 * the previous write is provided to the user for
 * subsequent writes.
 *
 * Returns 0 on success and -1 if the frame was dropped.
 */
int renderer_swap(struct renderer* r, char** buf);

/*
 * Release all internal resources.
 *
 * Does NOT close the file descriptor or free the buffers.
 */
void renderer_destroy(struct renderer* r);