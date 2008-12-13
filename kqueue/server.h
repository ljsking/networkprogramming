#ifndef STRUCTURES_H
#define STRUCTURES_H
typedef struct in_addr in_addr;
typedef struct sockaddr_in sockaddr_in;
typedef struct servent servent;
typedef struct timespec timespec;

typedef void (action) (void *arg);

typedef struct {
	action 	*do_action;
	int		fd;
	void 	*data;
} event;

extern list_t clientList;
extern list_t eventList;
extern int kq;
extern int listener_fd;
#endif