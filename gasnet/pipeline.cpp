#include "pygas.h"
#include "pipeline.h"

#include <cstring> /* for memcpy */
#include <tuple>

using namespace std;

typedef tuple<gasnet_node_t,void*> MsgID;

/* Register a fragment of a message. If all fragments are in place,
 * return 1 and populate MSG and NBYTES with the message address
 * and size in bytes. Caller is responsible for freeing MSG on
 * success. */
int pygas_register_fragment(char* fragment, char** msg)
{
    msg_info_t* frag_info = (msg_info_t*) &fragment[0];
    int frag_size = frag_info->nbytes + sizeof(msg_info_t);
    *msg = (char*) malloc(frag_size);
    memcpy(*msg, fragment, frag_size);
    return 1;
}
