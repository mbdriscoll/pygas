#include "pygas.h"
#include "pipeline.h"

#include <cstring> /* for memcpy */
#include <map> /* for map */
#include <bitset> /* for bitset */

using namespace std;

class MsgID {
public:
    gasnet_node_t sender;
    void* addr;

    MsgID(gasnet_node_t sender, void* addr) :
        sender(sender), addr(addr) { }

    bool operator<(const MsgID& o) const {
        return this->sender < o.sender &&
            this->addr < o.addr;
    }

    bool operator==(const MsgID& o) const {
        return this->sender == o.sender &&
            this->addr == o.addr;
    }
};

class MsgBuf {
public:
    char* data;
    bitset<1024> fragments;

    MsgBuf(msg_info_t* info) {
        this->data = (char *) malloc(info->total_bytes + sizeof(msg_info_t));

        int num_fragments = info->total_bytes / PYGAS_MAX_PAYLOAD + 1;
        for(int i = 0; i < num_fragments; i++)
            this->fragments[i] = 1;
    }

    char* slot_for_fragment_num(msg_info_t* frag_info) {
        return this->data +
               sizeof(msg_info_t) +
               frag_info->fragment_num * PYGAS_MAX_PAYLOAD;
    }

    bool ready() {
        return !this->fragments.any();
    }
};


map<MsgID,MsgBuf*> recv_buf;

/* Register a fragment of a message. If all fragments are in place,
 * return 1 and populate MSG with the message address and size in
 * bytes. Caller is responsible for freeing MSG on success. */
int pygas_register_fragment(char* fragment, char** msg)
{
    msg_info_t* frag_info = (msg_info_t*) &fragment[0];

    // skip pipelining for small messages
    if (frag_info->nbytes == frag_info->total_bytes) {
        int frag_size = frag_info->nbytes + sizeof(msg_info_t);
        *msg = (char*) malloc(frag_size);
        memcpy(*msg, fragment, frag_size);
        return 1;
    }

    MsgBuf* buf;
    MsgID mid(frag_info->sender, frag_info->addr);

    if (recv_buf.find(mid) == recv_buf.end()) {
        buf = recv_buf[mid] = new MsgBuf(frag_info);
    } else {
        buf = recv_buf[mid];
    }
    char* slot = buf->slot_for_fragment_num(frag_info);
    memcpy(slot, fragment+sizeof(msg_info_t), frag_info->nbytes);
    buf->fragments[frag_info->fragment_num] = 0;

    if (buf->ready()) {
        msg_info_t* msg_info = (msg_info_t*) buf->data;
        memcpy(msg_info, frag_info, sizeof(msg_info_t));
        msg_info->nbytes = frag_info->total_bytes;
        msg_info->fragment_num = 0;
        recv_buf.erase(mid);
        *msg = buf->data;
        return 1;
    } else {
        return 0;
    }

    // todo free things ?
}
