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

#endif /* define _PYGAS_PYGAS_H_ */
