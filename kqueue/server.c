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

#define NUM_THREADS 3

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

pthread_mutex_t 	mutex[NUM_THREADS];
pthread_cond_t 		cond_variable[NUM_THREADS];
int					current_working_thread;

static void
usage (void)
{
	fatal ("Usage `%s [-p port]'", pname);
}

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
		int i;
		if(list_size(&eventList)>2)
			current_working_thread=(current_working_thread+1)%NUM_THREADS;
		for(i=0;i<list_size(&eventList);i++){
			pthread_mutex_lock(&mutex[current_working_thread]);
			pthread_cond_signal(&cond_variable[current_working_thread]);
			pthread_mutex_unlock(&mutex[current_working_thread]);
		}
	}
}

void *thread_func(void *arg){
	int num = (int)arg;
	for (;;)
	{
		pthread_mutex_lock(&mutex[num]);
		pthread_cond_wait(&cond_variable[num], &mutex[num]);
		pthread_mutex_unlock(&mutex[num]);
		printf("start working %d thread\n", num);
		if(list_size(&eventList)>0){
			event *ev = list_extract_at(&eventList, 0);
			(*ev->do_action) (ev);
		}
		printf("end working %d thread\n", num);
	}
}

int
main (register int const argc, register char *const argv[])
{
	in_addr listen_addr;
	int optch;
	int one = 1;
	int portno = 0;
	int option_errors = 0;
	auto sockaddr_in sin;
	register servent *servp;
	
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
		portno = 1234;

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

	struct kevent kev_listener;
	EV_SET(&kev_listener, listener_fd, EVFILT_READ, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 0, NULL);
	kevent(kq, &kev_listener, 1, NULL, 0, NULL);
	
	int i;
	pthread_t p_thread;
	for(i = 0;i<NUM_THREADS;i++){
		pthread_mutex_init(&mutex[i], NULL);
		pthread_cond_init (&cond_variable[i], NULL);
		pthread_create(&p_thread, NULL, thread_func, (void *)i);
	}
	current_working_thread = 0;
	event_loop ();
}