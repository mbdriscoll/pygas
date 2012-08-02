#ifndef _PYGAS_RPC_H_
#define _PYGAS_RPC_H_

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


/* The maximum payload per message. This is equivalent to the max
 * Medium AM size minus the header size. */
#define PYGAS_MAX_PAYLOAD (gasnet_AMMaxMedium()-sizeof(msg_info_t))

#endif /* _PYGAS_RPC_H_ */
