/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1986. */

/*
 * B editor -- Definitions for queues of nodes.
 */

typedef struct queue *queue;

struct queue {
	HEADER;
	node q_data;
	queue q_link;
};

#define Qnil ((queue) Vnil)
#ifdef lint
Visible queue qcopy();
Visible Procedure qrelease();
#else
#define qcopy(q) ((queue)copy((value)q))
#define qrelease(q) release((value)q)
#endif
#define emptyqueue(q) (!(q))

node queuebehead();
