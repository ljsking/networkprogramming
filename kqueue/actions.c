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
	register int n;
	element *kep = arg;
	register ecb *const ecbp = kep->ecbp;
	//printf("write to %d %s\n", kep->ident, ecbp->buf);
	n = write (kep->fd, ecbp->buf, ecbp->bufsiz);
	if (n == -1)
	{
		error ("Error writing socket: %s", strerror (errno));
		close (kep->fd);
	}
	free(ecbp->buf);
	free(ecbp);
	ke_change (kep->fd, EVFILT_WRITE, EV_DISABLE, kep->ecbp);
}

void
do_read (void *arg)
{
	enum { bufsize = 1024 };
	auto char buf[bufsize];
	auto char sentMsg[bufsize];
	register int n;
	
	event *ev = arg;
	int from = ev->fd;
	free(ev);
	
	printf("try to read from %d\n", from);

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
	//printf("read from client %s\n", buf);
	buf[n-1] = 0;
	printf("<%d>:%s %d\n", from, buf, n);
	//list_iterator_start(&clientList);/* starting an iteration "session" */
	//while (list_iterator_hasnext(&clientList)) { /* tell whether more values available */
	/*	to = (int)list_iterator_next(&clientList);
		if(to != from){
			ecbp = xmalloc(sizeof(ecb));
			ecbp->do_read = do_read;
			ecbp->do_write = do_write;
			n = sizeof(sentMsg);
			ecbp->buf = (char *) xmalloc (n);
			strcpy(ecbp->buf, sentMsg);
			ecbp->bufsiz = n;
			ke_change (to, EVFILT_WRITE, EV_ENABLE, ecbp);
		}
	}
	list_iterator_stop(&clientList);*/
	
}

void
do_accept (void *arg)
{
	auto sockaddr_in sin;
	auto socklen_t sinsiz;
	register int s;
	register ecb *ecbp;

	if ((s = accept (listener_fd, (struct sockaddr *)&sin, &sinsiz)) == -1)
		fatal ("Error in accept(): %s", strerror (errno));
	printf("Client is connected socketID:(%d)\n", s);
	struct kevent kev_listener;
	EV_SET(&kev_listener, listener_fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
	kevent(kq, &kev_listener, 1, NULL, 0, NULL);
	struct kevent kev_client;
	EV_SET(&kev_client, s, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
	kevent(kq, &kev_client, 1, NULL, 0, NULL);
}