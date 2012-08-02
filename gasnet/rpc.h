#ifndef _PYGAS_RPC_H_
#define _PYGAS_RPC_H_

#include "pygas.h"

/* This is the header for messages sent through GASNet AMs. 'nbytes'
 * should be set to the number of bytes in the message payload, which
 * is found after the header, i.e. at  &msg[sizeof(msg_info_t)]. */
typedef struct msg_info {
    gasnet_node_t sender;
    size_t nbytes;
    void *addr;
    short fragment_num;
    size_t total_payload_bytes;
} msg_info_t;

static PyObject *rpc_handler;

int
pygas_async_request_handler(void* request);

void
pygas_rpc_request_handler(gasnet_token_t token, char* fragment, size_t frag_size);

void
pygas_rpc_reply_handler(gasnet_token_t token, char* fragment, size_t frag_size);

extern PyObject *
set_rpc_handler(PyObject *dummy, PyObject *args);

extern PyObject *
pygas_gasnet_rpc(PyObject *self, PyObject *args);

/* The maximum payload per message. This is equivalent to the max
 * Medium AM size minus the header size. */
#define PYGAS_MAX_PAYLOAD (gasnet_AMMaxMedium()-sizeof(msg_info_t))

#endif /* _PYGAS_RPC_H_ */
