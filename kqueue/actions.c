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
	int from, to;
	element *kep = arg;
	ecb *ecbp = NULL;

	if ((n = read (kep->fd, buf, bufsize)) == -1)
	{
		error ("Error reading socket: %s", strerror (errno));
		close (kep->fd);
		free (kep->ecbp);
	}
	else if (n == 0)
	{
		error ("EOF reading socket");
		close (kep->fd);
		free (kep->ecbp);
	}
	from = kep->fd;
	//printf("read from client %s\n", buf);
	sprintf(sentMsg, "<%d>:%s\n", from, buf);
	list_iterator_start(&clientList);/* starting an iteration "session" */
	while (list_iterator_hasnext(&clientList)) { /* tell whether more values available */
		to = (int)list_iterator_next(&clientList);
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
	list_iterator_stop(&clientList);
}

void
do_accept ()
{
	auto sockaddr_in sin;
	auto socklen_t sinsiz;
	register int s;
	register ecb *ecbp;

	if ((s = accept (listener_fd, (struct sockaddr *)&sin, &sinsiz)) == -1)
		fatal ("Error in accept(): %s", strerror (errno));

	/*ecbp = (ecb *) xmalloc (sizeof (ecb));
	ecbp->do_read = do_read;
	ecbp->do_write = do_write;
	ecbp->buf = NULL;
	ecbp->bufsiz = 0;*/

	printf("Client is connected socketID:(%d)\n", s);
	//list_append(& clientList, (void *)s);
	//ke_change (s, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, ecbp);
	//ke_change (s, EVFILT_WRITE, EV_ADD | EV_DISABLE | EV_ONESHOT, ecbp);
	struct kevent kev_listener;
	EV_SET(&kev_listener, listener_fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
	kevent(kq, &kev_listener, 1, NULL, 0, NULL);
}