#include "pipeline.h"

#include <cstring> /* for memcpy */
#include <map> /* for map */
#include <bitset> /* for bitset */

#include "rpc.h"

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
        this->data = (char *) malloc(info->total_payload_bytes + sizeof(msg_info_t));

        int num_fragments = info->total_payload_bytes / PYGAS_MAX_PAYLOAD + 1;
        for(int i = 0; i < num_fragments; i++)
            this->fragments[i] = 1;
    }

    ~MsgBuf() {}

    char* slot_for_fragment_num(msg_info_t* frag_info) {
        return this->data +
               sizeof(msg_info_t) +
               frag_info->fragment_num * PYGAS_MAX_PAYLOAD;
    }

    bool ready() {
        return !this->fragments.any();
    }

    void add_fragment(char* fragment) {
        msg_info_t* frag_info = (msg_info_t*) fragment;
        char* slot = this->slot_for_fragment_num(frag_info);
        memcpy(slot, fragment+sizeof(msg_info_t), frag_info->nbytes);
        this->fragments[frag_info->fragment_num] = 0;
    }

    char* prepare_msg(msg_info_t* frag_info) {
        msg_info_t* msg_info = (msg_info_t*) this->data;
        memcpy(msg_info, frag_info, sizeof(msg_info_t));
        msg_info->nbytes = frag_info->total_payload_bytes;
        msg_info->fragment_num = 0;
        return this->data;
    }
};


map<MsgID,MsgBuf*> recv_buf;
gasnet_hsl_t recv_buf_lock = GASNET_HSL_INITIALIZER;

/* Register a fragment of a message. If all fragments are in place,
 * return 1 and populate MSG with the message address and size in
 * bytes. Caller is responsible for freeing MSG on success. */
int pygas_register_fragment(char* fragment, char** msg)
{
    msg_info_t* frag_info = (msg_info_t*) &fragment[0];

    // skip pipelining for small messages
    if (frag_info->nbytes == frag_info->total_payload_bytes) {
        int frag_size = frag_info->nbytes + sizeof(msg_info_t);
        *msg = (char*) malloc(frag_size);
        memcpy(*msg, fragment, frag_size);
        return 1;
    }

    MsgBuf* buf;
    MsgID mid(frag_info->sender, frag_info->addr);

    int ready = 0;
    gasnet_hsl_lock(&recv_buf_lock);
    {
        if (recv_buf.find(mid) == recv_buf.end())
            buf = recv_buf[mid] = new MsgBuf(frag_info);
        else
            buf = recv_buf[mid];

        buf->add_fragment(fragment);

        if ((ready = buf->ready())) {
            *msg = buf->prepare_msg(frag_info);
            recv_buf.erase(mid);
            delete buf;
        }
    }
    gasnet_hsl_unlock(&recv_buf_lock);

    return ready;
}
