#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/event.h>
#include <sys/time.h>
#include <pthread.h>

#include "simclist.h"

#include "server.h"
#include "util.h"

void
do_write (void *arg)
{
	printf("write msg\n");
	event *ev = arg;
	char *msg = ev->data;
	int to = ev->fd;
	int n = write (to, msg, strlen(msg));
	if (n == -1)
	{
		error ("Error writing socket: %s", strerror (errno));
		close (to);
	}
	free(msg);
	free(ev);
	//sleep(10);
}

void
do_read (void *arg)
{
	printf("read msg\n");
	enum { bufsize = 1024 };
	char buf[bufsize];
	char sentMsg[bufsize];
	int n;
	
	event *ev = arg;
	int from = ev->fd;
	free(ev);

	if ((n = read (from, buf, bufsize)) == -1)
	{
		error ("Error reading socket: %s", strerror (errno));
		close (from);
	}
	else if (n == 0)
	{
		error ("EOF reading socket");
		close (from);
	}
	struct kevent kev_client;
	EV_SET(&kev_client, from, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
	kevent(kq, &kev_client, 1, NULL, 0, NULL);
	buf[n-1] = 0;
	sprintf(sentMsg, "<%d>:%s\n", from, buf);
	list_iterator_start(&clientList);/* starting an iteration "session" */
	while (list_iterator_hasnext(&clientList)) { /* tell whether more values available */
		int to = (int)list_iterator_next(&clientList);
		if(to != from){
			char *buf = malloc(sizeof(char)*strlen(sentMsg));
			strcpy(buf, sentMsg);
			struct kevent kev_client;
			EV_SET(&kev_client, to, EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, buf);
			kevent(kq, &kev_client, 1, NULL, 0, NULL);
		}
	}
	list_iterator_stop(&clientList);
	
}

void
do_accept (void *arg)
{
	sockaddr_in sin;
	socklen_t sinsiz;
	int s;

	if ((s = accept (listener_fd, (struct sockaddr *)&sin, &sinsiz)) == -1)
		fatal ("Error in accept(): %s", strerror (errno));
	printf("Client is connected socketID:(%d)\n", s);
	
	list_append(&clientList, (void *)s);
	
	struct kevent kev_listener;
	EV_SET(&kev_listener, listener_fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
	kevent(kq, &kev_listener, 1, NULL, 0, NULL);
	
	struct kevent kev_client;
	EV_SET(&kev_client, s, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
	kevent(kq, &kev_client, 1, NULL, 0, NULL);
}