#include "rpc.h"

static PyObject *rpc_handler = NULL;

int
pygas_async_request_handler(void* request) {
    msg_info_t* request_info = (msg_info_t*) request;
    gasnet_node_t dest = request_info->sender;

    PyObject *result = PyObject_CallFunction(rpc_handler, "(s#)", (char*) request+sizeof(msg_info_t), request_info->nbytes);
    assert(PyBytes_Check(result));

    Py_ssize_t total_payload_bytes;
    char *data;
    PyBytes_AsStringAndSize(result, &data, &total_payload_bytes);
    if (PYGAS_BIGMSG_PIPELINE) {
        char reply[min(gasnet_AMMaxMedium(), sizeof(msg_info_t)+total_payload_bytes)];
        msg_info_t *reply_info = (msg_info_t*) &reply[0];
        reply_info->sender = MYTHREAD;
        reply_info->addr = request_info->addr;
        reply_info->total_payload_bytes = total_payload_bytes;

        int offset;
        for(offset = 0; offset < total_payload_bytes; offset += PYGAS_MAX_PAYLOAD) {
            int frag_payload_bytes = reply_info->nbytes = min(total_payload_bytes-offset, PYGAS_MAX_PAYLOAD);
            reply_info->fragment_num = offset / PYGAS_MAX_PAYLOAD;
            memcpy(&reply[sizeof(msg_info_t)], data+offset, frag_payload_bytes);
            gasnet_AMRequestMedium0(dest, RPC_REPLY_HIDX, reply, sizeof(msg_info_t)+frag_payload_bytes);
        }
    } else /* PYGAS_BIGMSG_RMALLOC */ {
        size_t total_bytes = sizeof(msg_info_t) + total_payload_bytes;
        char reply[total_bytes];
        msg_info_t *reply_info = (msg_info_t*) reply;
        reply_info->sender = MYTHREAD;
        reply_info->addr = request_info->addr;
        reply_info->total_payload_bytes = total_payload_bytes;
        reply_info->fragment_num = 0;
        reply_info->nbytes = total_payload_bytes;
        memcpy(reply+sizeof(msg_info_t), data, total_payload_bytes);

        long int raddr = rmalloc(dest, total_bytes);
        gasnet_AMRequestLong0(dest, RPC_REPLY_HIDX, reply, total_bytes, (void*) raddr);
    }

    //free(request);
    return 0;
}

void
pygas_rpc_request_handler(gasnet_token_t token, char* fragment, size_t frag_size)
{
    assert(frag_size == sizeof(msg_info_t)+((msg_info_t*)&fragment[0])->nbytes);

    char *msg;
    if (pygas_register_fragment(fragment, &msg))
        Py_AddPendingCall(pygas_async_request_handler, msg);
}

void
pygas_rpc_reply_handler(gasnet_token_t token, char* fragment, size_t frag_size)
{
    assert(frag_size == sizeof(msg_info_t)+((msg_info_t*)fragment)->nbytes);

    char *msg;
    if (pygas_register_fragment(fragment, &msg)) {
        msg_info_t* msg_info = (msg_info_t*) &msg[0];
        // write val to end PYGAS_GASNET_BLOCKUNTIL
        *((char**) msg_info->addr) = msg;
    }
}

PyObject *
set_rpc_handler(PyObject *dummy, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);         /* Add a reference to new callback */
        Py_XDECREF(rpc_handler);  /* Dispose of previous callback */
        rpc_handler = temp;       /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

PyObject *
pygas_gasnet_rpc(PyObject *self, PyObject *args)
{
    char *data;
    int dest = 0, total_payload_bytes = 0;
    PyArg_ParseTuple(args, "is#", &dest, &data, &total_payload_bytes);

    volatile char* reply = NULL;
    if (PYGAS_BIGMSG_PIPELINE) {
        char request[min(gasnet_AMMaxMedium(), sizeof(msg_info_t)+total_payload_bytes)];
        msg_info_t *request_info = (msg_info_t*) &request[0];
        request_info->sender = MYTHREAD;
        request_info->addr = (void*) &reply;
        request_info->total_payload_bytes = total_payload_bytes;

        int offset;
        for(offset = 0; offset < total_payload_bytes; offset += PYGAS_MAX_PAYLOAD) {
            int frag_payload_bytes = min(total_payload_bytes-offset, PYGAS_MAX_PAYLOAD);
            request_info->nbytes = frag_payload_bytes;
            request_info->fragment_num = offset / PYGAS_MAX_PAYLOAD;
            memcpy(&request[sizeof(msg_info_t)], data+offset, request_info->nbytes);
            gasnet_AMRequestMedium0(dest, RPC_REQUEST_HIDX, &request,
                    sizeof(msg_info_t)+request_info->nbytes);
        }
    } else /* PYGAS_BIGMSG_RMALLOC */ {
        size_t total_bytes = sizeof(msg_info_t) + total_payload_bytes;
        char request[total_bytes];
        msg_info_t *request_info = (msg_info_t*) request;
        request_info->sender = MYTHREAD;
        request_info->addr = (void*) &reply;
        request_info->total_payload_bytes = total_payload_bytes;
        request_info->nbytes = total_payload_bytes;
        request_info->fragment_num = 0;
        memcpy(request+sizeof(msg_info_t), data, total_payload_bytes);

        long int raddr = rmalloc(dest, total_bytes);
        gasnet_AMRequestLong0(dest, RPC_REQUEST_HIDX, request, total_bytes, (void*) raddr);
    }

    PYGAS_GASNET_BLOCKUNTIL(reply != NULL);

    msg_info_t* reply_info = (msg_info_t*) &reply[0];
    PyObject *result = Py_BuildValue("s#", &reply[sizeof(msg_info_t)], reply_info->nbytes);

    //free(reply);
    return result;
}

