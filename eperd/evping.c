/* Standalone version of the event-based ping. */

#include "libbb.h"
#include <syslog.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include "eperd.h"

static void done(void *state UNUSED_PARAM)
{
	fprintf(stderr, "And we are done\n");
	exit(0);
}

int evping_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int evping_main(int argc UNUSED_PARAM, char **argv)
{
	int r;
	void *state;

	/* Create libevent event base */
	EventBase= event_base_new();
	if (!EventBase)
	{
		fprintf(stderr, "evping_base_new failed\n");
		exit(1);
	}

	state= ping_ops.init(argc, argv, done);
	if (!state)
	{
		fprintf(stderr, "evping_ops.init failed\n");
		exit(1);
	}
	ping_ops.start(state);

	r= event_base_loop(EventBase, 0);
	if (r != 0)
	{
		fprintf(stderr, "evping_base_loop failed\n");
		exit(1);
	}
	return 0; /* not reached */
}
