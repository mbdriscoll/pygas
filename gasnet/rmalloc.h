#ifndef _PYGAS_RMALLOC_H_
#define _PYGAS_RMALLOC_H_

#include "pygas.h"

void* rmalloc(gasnet_node_t dest, size_t size);

void
pygas_rmalloc_request_handler(gasnet_token_t token, char* msg, size_t msg_size);

void
pygas_rmalloc_reply_handler(gasnet_token_t token, char* msg, size_t msg_size);

#endif /* _PYGAS_RMALLOC_H_ */
