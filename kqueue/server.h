#ifndef STRUCTURES_H
#define STRUCTURES_H
typedef struct in_addr in_addr;
typedef struct sockaddr_in sockaddr_in;
typedef struct servent servent;
typedef struct timespec timespec;

typedef void (action) (void *arg);

/* Event Control Block (ecb) */
typedef struct {
	action		*do_read;
	action		*do_write;
	char		*buf;
	unsigned	bufsiz;
} ecb;

typedef struct {
	ecb 	 	*ecbp;
	short 		filter; 
	uintptr_t	fd;
} element;

typedef struct {
	action 	*do_action;
	int		fd;
} event;

void
ke_change (register int const ident,
	   	register int const filter,
	   	register int const flags,
		register void *const udata);

extern list_t clientList;
extern list_t eventList;
extern int kq;
extern int listener_fd;
#endif