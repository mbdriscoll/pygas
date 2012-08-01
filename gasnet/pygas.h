#ifndef _PYGAS_PYGAS_H_
#define _PYGAS_PYGAS_H_

#include "Python.h"
#include "gasnet.h"

/* make this code a little more UPC-like */
#define THREADS (gasnet_nodes())
#define MYTHREAD (gasnet_mynode())

/* This is the header for messages sent through GASNet AMs. 'nbytes'
 * should be set to the number of bytes in the message payload, which
 * is found after the header, i.e. at  &msg[sizeof(msg_info_t)]. */
typedef struct msg_info {
    gasnet_node_t sender;
    size_t nbytes;
    void *addr;
    short fragment_num;
    size_t total_bytes;
} msg_info_t;


/* The maximum payload per message. This is equivalent to the max
 * Medium AM size minus the header size. */
#define PYGAS_MAX_PAYLOAD (gasnet_AMMaxMedium()-sizeof(msg_info_t))


/* Active message handler id's. */
#define APPLY_DYNAMIC_REQUEST_HIDX 144
#define APPLY_DYNAMIC_REPLY_HIDX   145
#define RMALLOC_REQUEST_HIDX       146
#define RMALLOC_REPLY_HIDX         147

/* Redefine BLOCKUNTIL macro to include calls to Py_MakePendingCalls
 * so interpreter can make progress when blocked. */
// TODO check implications of this with gasnet team
#define PYGAS_GASNET_BLOCKUNTIL(cond) \
    while (!(cond)) {             \
        gasnet_AMPoll();          \
	    Py_MakePendingCalls();    \
    }

/* Controls how big messages are handled. */
// TODO autotune to find optimal threshold.
#define PYGAS_BIGMSG_PIPELINE 1
#define PYGAS_BIGMSG_RMALLOC (!(PYGAS_BIGMSG_PIPELINE))

#endif /* define _PYGAS_PYGAS_H_ */
