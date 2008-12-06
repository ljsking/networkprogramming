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
#include "simclist.h"

#include "util.h"

typedef struct in_addr in_addr;
typedef struct sockaddr_in sockaddr_in;
typedef struct servent servent;
typedef struct timespec timespec;

typedef void (action) (register struct kevent const *const kep);

/* Event Control Block (ecb) */
typedef struct {
	action		*do_read;
	action		*do_write;
	char		*buf;
	unsigned	bufsiz;
} ecb;

char const *pname;
static struct kevent *ke_vec = NULL;
static unsigned ke_vec_alloc = 0;
static unsigned ke_vec_used = 0;
static char const protoname[] = "tcp";
static char const servname[] = "echo";
static list_t clientList;

static void
usage (void)
{
	fatal ("Usage `%s [-p port]'", pname);
}

static void
ke_change (register int const ident,
	   register int const filter,
	   register int const flags,
	   register void *const udata)
{
	enum { initial_alloc = 64 };
	register struct kevent *kep;

	if (!ke_vec_alloc){
		ke_vec_alloc = initial_alloc;
		ke_vec = (struct kevent *) xmalloc(ke_vec_alloc * sizeof (struct kevent));
	}else if (ke_vec_used == ke_vec_alloc){
		ke_vec_alloc <<= 1;
		ke_vec =
		(struct kevent *) xrealloc (ke_vec,
				ke_vec_alloc * sizeof (struct kevent));
	}

	kep = &ke_vec[ke_vec_used++];

	kep->ident = ident;
	kep->filter = filter;
	kep->flags = flags;
	kep->fflags = 0;
	kep->data = 0;
	kep->udata = udata;
}

static void
do_write (register struct kevent const *const kep)
{
	register int n;
	register ecb *const ecbp = (ecb *) kep->udata;
	printf("write to %d %s\n", kep->ident, ecbp->buf);
	n = write (kep->ident, ecbp->buf, ecbp->bufsiz);
	if (n == -1)
	{
		error ("Error writing socket: %s", strerror (errno));
		close (kep->ident);
	}
	free(ecbp->buf);
	free(ecbp);
	ke_change (kep->ident, EVFILT_WRITE, EV_DISABLE, kep->udata);
}

static void
do_read (register struct kevent const *const kep)
{
	enum { bufsize = 1024 };
	auto char buf[bufsize];
	auto char sentMsg[bufsize];
	register int n;
	int from, to;
	ecb *ecbp = NULL;

	if ((n = read (kep->ident, buf, bufsize)) == -1)
	{
		error ("Error reading socket: %s", strerror (errno));
		close (kep->ident);
		if(kep->udata)
			free (kep->udata);
	}
	else if (n == 0)
	{
		error ("EOF reading socket");
		close (kep->ident);
		if(kep->udata)
			free (kep->udata);
	}
	from = kep->ident;
	sprintf(sentMsg, "<%d>:%s", from, buf);
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

static void
do_accept (register struct kevent const *const kep)
{
	auto sockaddr_in sin;
	auto socklen_t sinsiz;
	register int s;
	register ecb *ecbp;

	if ((s = accept (kep->ident, (struct sockaddr *)&sin, &sinsiz)) == -1)
		fatal ("Error in accept(): %s", strerror (errno));

	ecbp = (ecb *) xmalloc (sizeof (ecb));
	ecbp->do_read = do_read;
	ecbp->do_write = do_write;
	ecbp->buf = NULL;
	ecbp->bufsiz = 0;

	printf("Client is connected socketID:(%d)\n", s);
	list_append(& clientList, (void *)s);
	ke_change (s, EVFILT_READ, EV_ADD | EV_ENABLE, ecbp);
	ke_change (s, EVFILT_WRITE, EV_ADD | EV_DISABLE, ecbp);
}

static void event_loop (register int const kq)
    __attribute__ ((__noreturn__));

static void
event_loop (register int const kq)
{
	for (;;)
	{
		register int n;
		register struct kevent const *kep;
		n = kevent (kq, ke_vec, ke_vec_used, ke_vec, ke_vec_alloc, NULL);
		ke_vec_used = 0;  /* Already processed all changes.  */
		if (n == -1)
			fatal ("Error in kevent(): %s", strerror (errno));
		if (n == 0)
			fatal ("No events received!");

		for (kep = ke_vec; kep < &ke_vec[n]; kep++)
		{
			register ecb const *const ecbp = (ecb *) kep->udata;
			if (kep->filter == EVFILT_READ)
				(*ecbp->do_read) (kep);
			else
				(*ecbp->do_write) (kep);
		}
	}
}

int
main (register int const argc, register char *const argv[])
{
	auto in_addr listen_addr;
	register int optch;
	auto int one = 1;
	register int portno = 0;
	register int option_errors = 0;
	register int server_sock;
	auto sockaddr_in sin;
	register servent *servp;
	auto ecb listen_ecb;
	register int kq;
	
	list_init(&clientList);

	pname = strrchr (argv[0], '/');
	pname = pname ? pname+1 : argv[0];

	listen_addr.s_addr = htonl (INADDR_ANY);  /* Default.  */

	while ((optch = getopt (argc, argv, "p:")) != EOF)
	{
		switch (optch)
		{
		case 'p':
			if (strlen (optarg) == 0 || !all_digits (optarg)){
				error ("Invalid argument for -p option: %s", optarg);
				option_errors++;
			}
			portno = atoi (optarg);
			if (portno == 0 || portno >= (1u << 16)){
				error ("Invalid argument for -p option: %s", optarg);
				option_errors++;
			}
			break;
		default:
			error ("Invalid option: -%c", optch);
			option_errors++;
		}
	}

	if (option_errors || optind != argc)
		usage ();

	if (portno == 0)
	{
		if ((servp = getservbyname (servname, protoname)) == NULL)
			fatal ("Error getting port number for service `%s': %s",
					servname, strerror (errno));
					portno = ntohs (servp->s_port);
	}

	if ((server_sock = socket (PF_INET, SOCK_STREAM, 0)) == -1)
		fatal ("Error creating socket: %s", strerror (errno));

	if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one) == -1)
		fatal ("Error setting SO_REUSEADDR for socket: %s", strerror (errno));

	memset (&sin, 0, sizeof sin);
	sin.sin_family = AF_INET;
	sin.sin_addr = listen_addr;
	sin.sin_port = htons (portno);

	if (bind (server_sock, (const struct sockaddr *)&sin, sizeof sin) == -1)
		fatal ("Error binding socket: %s", strerror (errno));

	if (listen (server_sock, 20) == -1)
		fatal ("Error listening to socket: %s", strerror (errno));

	if ((kq = kqueue ()) == -1)
		fatal ("Error creating kqueue: %s", strerror (errno));

	listen_ecb.do_read = do_accept;
	listen_ecb.do_write = NULL;
	listen_ecb.buf = NULL;
	listen_ecb.bufsiz = 0;

	ke_change (server_sock, EVFILT_READ, EV_ADD | EV_ENABLE, &listen_ecb);

	event_loop (kq);
}