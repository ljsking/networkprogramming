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

#include "actions.h"
#include "util.h"

#include "server.h"

char const *pname;
static struct kevent *ke_vec = NULL;
static unsigned ke_vec_alloc = 0;
static unsigned ke_vec_used = 0;
static char const protoname[] = "tcp";
static char const servname[] = "echo";
list_t clientList;
list_t eventList;
int kq;
int listener_fd;

static void
usage (void)
{
	fatal ("Usage `%s [-p port]'", pname);
}

void
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

static void event_loop ()
    __attribute__ ((__noreturn__));

static void
event_loop ()
{
	for (;;)
	{
		register int n;
		struct kevent received_event;
		n = kevent(kq, NULL, 0, &received_event, 1, NULL);
		if (n == -1)
			fatal ("Error in kevent(): %s", strerror (errno));
		if (n == 0)
			fatal ("No events received!");
		printf("get a event\n");
		event *ev = malloc(sizeof(ev));
		ev->fd = received_event.ident;
		ev->data = received_event.udata;
		if(received_event.filter == EVFILT_READ){
			if(received_event.ident == listener_fd)
				ev->do_action = do_accept;
			else
				ev->do_action = do_read;
		}
		else
			ev->do_action = do_write;
		list_append(&eventList, ev);
	}
}

void *thread_func(){
	for (;;)
	{
		if(list_size(&eventList)>0){
			event *ev = list_extract_at(&eventList, 0);
			(*ev->do_action) (ev);
		}
		sleep(1);
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
	auto sockaddr_in sin;
	register servent *servp;
	auto ecb listen_ecb;
	
	list_init(&clientList);
	list_init(&eventList);

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
		portno = 1234;
	}

	if ((listener_fd = socket (PF_INET, SOCK_STREAM, 0)) == -1)
		fatal ("Error creating socket: %s", strerror (errno));

	if (setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one) == -1)
		fatal ("Error setting SO_REUSEADDR for socket: %s", strerror (errno));

	memset (&sin, 0, sizeof sin);
	sin.sin_family = AF_INET;
	sin.sin_addr = listen_addr;
	sin.sin_port = htons (portno);

	if (bind (listener_fd, (const struct sockaddr *)&sin, sizeof sin) == -1)
		fatal ("Error binding socket: %s", strerror (errno));

	if (listen (listener_fd, 20) == -1)
		fatal ("Error listening to socket: %s", strerror (errno));

	if ((kq = kqueue ()) == -1)
		fatal ("Error creating kqueue: %s", strerror (errno));

	//ke_change (server_sock, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, &listen_ecb);
	
	struct kevent kev_listener;
	EV_SET(&kev_listener, listener_fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
	kevent(kq, &kev_listener, 1, NULL, 0, NULL);
	
	pthread_t p_thread;
	pthread_create(&p_thread, NULL, thread_func, (void *)NULL);

	event_loop ();
}