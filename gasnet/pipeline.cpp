#include "pipeline.h"
#include "pygas.h"

#include <cstring> /* for memcpy */
#include <tuple>

using namespace std;

typedef tuple<gasnet_node_t,void*> MsgID;

/* Register a fragment of a message. If all fragments are in place,
 * return 1 and populate MSG and NBYTES with the message address
 * and size in bytes. */
int pygas_register_fragment(msg_info_t* msg_info, char* fragment,
        char** msg, int* nbytes)
{
    int msg_size = *nbytes = msg_info->total_bytes;
    *msg = (char*) malloc(msg_size);
    memcpy(msg, fragment, msg_size);
    return 1;
}
