#include "rmalloc.h"

typedef struct rmalloc_request {
    void** sync_addr;
    size_t size;
} rmalloc_request_t;

typedef struct rmalloc_reply{
    void** sync_addr;
    void* answer;
} rmalloc_reply_t;

void* rmalloc(gasnet_node_t dest, size_t size) {
    void* answer = NULL;
    rmalloc_request_t request = { &answer, size };

    gasnet_AMRequestMedium0(dest, RMALLOC_REQUEST_HIDX, &request, sizeof(rmalloc_request_t));
    PYGAS_GASNET_BLOCKUNTIL(answer != NULL);

    return answer;
}

void
pygas_rmalloc_request_handler(gasnet_token_t token, char* msg, size_t msg_size) {
    assert(msg_size == sizeof(rmalloc_request_t));

    rmalloc_request_t* request = (rmalloc_request_t*) msg;
    rmalloc_reply_t reply;
    reply.sync_addr = request->sync_addr;
    reply.answer = malloc(request->size);

    gasnet_AMReplyMedium0(token, RMALLOC_REPLY_HIDX, &reply, sizeof(rmalloc_reply_t));
}

void
pygas_rmalloc_reply_handler(gasnet_token_t token, char* msg, size_t msg_size) {
    assert(msg_size == sizeof(rmalloc_reply_t));

    rmalloc_reply_t* reply = (rmalloc_reply_t*) msg;
    *(reply->sync_addr) = reply->answer;
}
