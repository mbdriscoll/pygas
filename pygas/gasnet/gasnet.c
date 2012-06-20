#include "Python.h"

/* be backwards compatible with CObjects */
#if PY_MAJOR_VERSION <= 2 && PY_MINOR_VERSION < 7
    #include "capsulethunk.h"
#endif

#define GASNET_PAR
#include "gasnet.h"

/* make this code a little more UPC-like */
#define THREADS (gasnet_nodes())
#define MYTHREAD (gasnet_mynode())

#define APPLY_DYNAMIC_REQUEST_HIDX 144
#define APPLY_DYNAMIC_REPLY_HIDX   145
// adding new hidx? be sure to update gasnet_handerentry_t count

// TODO check implications of this with gasnet team
#define PYGASNET_BLOCKUNTIL(cond) \
    while (!(cond)) {             \
        gasnet_AMPoll();          \
	Py_MakePendingCalls();    \
    }                    

/* Reimplementation of gasnet_barrier_wait that allows pending
 * Python calls created by incoming AMs to be serviced by the
 * interpreter while waiting.
 * XXX: spins. check with gasnet team for advice here.
 * TODO: support for other gasnet_barrier_try return codes. */
#define PYGASNET_BARRIER_WAIT(id, flags) \
    while (gasnet_barrier_try((id), (flags)) != GASNET_OK) { \
        Py_MakePendingCalls(); \
    }

typedef struct msg_info {
    size_t nbytes;
    char* data;
    gasnet_node_t sender;
    void* addr0;
    void* addr1;
} msg_info_t;
  
static PyObject *
py_gasnet_init(PyObject *self, PyObject *args)
{
    PyEval_InitThreads();

    int argc = 2;
    char **argv = (char**) malloc(argc * sizeof(char*));
    argv[0] = "2";
    argv[1] = "3";

    int status = gasnet_init(&argc, &argv);

    return Py_BuildValue("i", status);
}

static PyObject *
py_gasnet_apply_dynamic(PyObject *self, PyObject *args)
{
    char *data;
    int dest = 0, nbytes = 0;
    PyArg_ParseTuple(args, "is#", &dest, &data, &nbytes);

    char *msg = NULL;
    uint32_t addr_lo = ((uint64_t) &msg) >> 0;
    uint32_t addr_hi = ((uint64_t) &msg) >> 32;
    gasnet_AMRequestMedium2(dest, APPLY_DYNAMIC_REQUEST_HIDX, data, nbytes, addr_lo, addr_hi);
    //printf("thread %d blockuntil 0x%lx\n", gasnet_mynode(), &msg);
    PYGASNET_BLOCKUNTIL(msg != NULL);

    msg_info_t* msg_info = (msg_info_t*) &msg[0];
    PyObject * result = Py_BuildValue("s#", msg_info->data, msg_info->nbytes);

    return result;
}

static PyObject *apply_dynamic_handler = NULL;

static PyObject *
set_apply_dynamic_handler(PyObject *dummy, PyObject *args)
{
    PyObject *result = NULL;
    PyObject *temp;

    if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
        if (!PyCallable_Check(temp)) {
            PyErr_SetString(PyExc_TypeError, "parameter must be callable");
            return NULL;
        }
        Py_XINCREF(temp);         /* Add a reference to new callback */
        Py_XDECREF(apply_dynamic_handler);  /* Dispose of previous callback */
        apply_dynamic_handler = temp;       /* Remember new callback */
        /* Boilerplate to return "None" */
        Py_INCREF(Py_None);
        result = Py_None;
    }
    return result;
}

int
pygas_async_request_handler(char* msg) {
    msg_info_t* msg_info = (msg_info_t*) &msg[0];

    if (!PyCallable_Check(apply_dynamic_handler))
       printf("thread %d didn't get a callable apply_dynamic_handler\n", gasnet_mynode());

    PyObject *result = PyObject_CallFunction(apply_dynamic_handler, "(s#)", msg_info->data, msg_info->nbytes);
    if (!PyString_Check(result))
        printf("Didn't get a string from CallFunction in async_handler\n");

    int nbytes;
    char *data;
    PyString_AsStringAndSize(result, &data, &nbytes);
    gasnet_AMRequestMedium2(msg_info->sender, APPLY_DYNAMIC_REPLY_HIDX, data, nbytes, msg_info->addr0, msg_info->addr1);
    
    return 0;
}

void
pygas_apply_dynamic_request_handler(gasnet_token_t token, char* data, size_t nbytes, void *addr0, void *addr1)
{
    char* msg = (char*) malloc(nbytes + sizeof(msg_info_t));
    msg_info_t* msg_info = (msg_info_t*) &msg[0];

    gasnet_node_t sender;
    gasnet_AMGetMsgSource(token, &sender);

    msg_info->sender = sender;
    msg_info->nbytes = nbytes;
    msg_info->data = &msg[sizeof(msg_info_t)];
    msg_info->addr0 = addr0;
    msg_info->addr1 = addr1;
    memcpy(msg_info->data, data, nbytes);

    Py_AddPendingCall(pygas_async_request_handler, msg);
}

void
pygas_apply_dynamic_reply_handler(gasnet_token_t token, void* data, size_t nbytes, uint32_t addr0, uint32_t addr1)
{
    uint64_t addr = ((uint64_t) addr0) | (((uint64_t) addr1) << 32);

    char* msg = (char*) malloc(nbytes + sizeof(msg_info_t));
    msg_info_t* msg_info = (msg_info_t*) &msg[0];

    msg_info->nbytes = nbytes;
    msg_info->data = &msg[sizeof(msg_info_t)];
    memcpy(msg_info->data, data, nbytes);

    // write val to end PYGASNET_BLOCKUNTIL
    *((char**) addr) = msg;
}

gasnet_handlerentry_t handler_table[] = {
    {APPLY_DYNAMIC_REQUEST_HIDX,   pygas_apply_dynamic_request_handler},
    {APPLY_DYNAMIC_REPLY_HIDX,     pygas_apply_dynamic_reply_handler}
};

static PyObject *
py_gasnet_attach(PyObject *self, PyObject *args)
{
    int status = gasnet_attach(&handler_table, 2, gasnet_getMaxLocalSegmentSize(), GASNET_PAGESIZE);

    gasnet_set_waitmode(GASNET_WAIT_BLOCK);

    return Py_BuildValue("i", status);
}

static PyObject *
py_gasnet_exit(PyObject *self, PyObject *args)
{
    int ok;
    int exitcode = 0;
    ok = PyArg_ParseTuple(args, "|i", &exitcode);

    /* execute barrier to be compliant with spec */
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    PYGASNET_BARRIER_WAIT(0, GASNET_BARRIERFLAG_ANONYMOUS);

    gasnet_exit(exitcode);

    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_mynode(PyObject *self, PyObject *args)
{
    gasnet_node_t rank = gasnet_mynode();
    return Py_BuildValue("i", rank);
}

static PyObject *
py_gasnet_nodes(PyObject *self, PyObject *args)
{
    gasnet_node_t ranks = gasnet_nodes();
    return Py_BuildValue("i", ranks);
}

static PyObject *
py_gasnet_getenv(PyObject *self, PyObject *args)
{
    int ok;
    const char *key, *value;
    ok = PyArg_ParseTuple(args, "s", &key);

    value = gasnet_getenv(key);

    return Py_BuildValue("s", value);
}

static PyObject *
py_gasnet_barrier_wait(PyObject *self, PyObject *args)
{
    int ok;
    int id = 0;
    int flags = GASNET_BARRIERFLAG_ANONYMOUS;
    ok = PyArg_ParseTuple(args, "|ii", &id, &flags);

    PYGASNET_BARRIER_WAIT(id, flags);

    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_barrier_notify(PyObject *self, PyObject *args)
{
    int ok;
    int id = 0;
    int flags = GASNET_BARRIERFLAG_ANONYMOUS;
    ok = PyArg_ParseTuple(args, "|ii", &id, &flags);

    gasnet_barrier_notify(id, flags);

    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_barrier_try(PyObject *self, PyObject *args)
{
    int ok;
    int id = 0;
    int flags = GASNET_BARRIERFLAG_ANONYMOUS;
    ok = PyArg_ParseTuple(args, "|ii", &id, &flags);

    gasnet_barrier_try(id, flags);

    Py_RETURN_NONE;
}

void py_gasnet_coll_apply(void *results, size_t result_count,
        const void *left_operands, size_t left_count,
        const void *right_operands,
        size_t elem_size, int flags, int arg)
{
    int i;
    int *res = (int*) results;
    int *src1 = (int*) left_operands;
    int *src2 = (int*) right_operands;
    for(i=0; i<result_count; i++) {
        res[i] = src1[i] + src2[i]; //TODO more than add
    }

    //PyObject *reduce_fxn = (PyObject*) arg;
    //PyObject_Print(reduce_fxn, stdout, Py_PRINT_RAW);
}

static PyObject *
py_gasnet_coll_init(PyObject *self, PyObject *args)
{
    int ok;
    ok = PyArg_ParseTuple(args, "");

    gasnet_coll_fn_entry_t fntable[1];
    fntable[0].fnptr=py_gasnet_coll_apply;
    fntable[0].flags=0;

    gasnet_coll_init(0, 0, fntable, 1, 0);

    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_coll_broadcast(PyObject *self, PyObject *args)
{
    int from_thread = 0;
    Py_buffer pb;
    PyObject *obj;
    assert( PyArg_ParseTuple(args, "O|i:from_thread", &obj, &from_thread) );
    assert( !PyObject_GetBuffer(obj, &pb, PyBUF_SIMPLE) );

    const int flags = GASNET_COLL_IN_MYSYNC|GASNET_COLL_OUT_MYSYNC|GASNET_COLL_LOCAL;
    gasnet_coll_broadcast(GASNET_TEAM_ALL, pb.buf, from_thread, pb.buf, pb.len, flags);

    PyBuffer_Release(&pb);
    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_coll_broadcast_nb(PyObject *self, PyObject *args)
{
    int ok, from_thread = 0;
    Py_buffer pb;
    PyObject *obj;
    ok = PyArg_ParseTuple(args, "O|i", &obj, &from_thread);
    ok = PyObject_GetBuffer(obj, &pb, PyBUF_SIMPLE);

    const int flags = GASNET_COLL_IN_MYSYNC|GASNET_COLL_OUT_MYSYNC|GASNET_COLL_LOCAL;
    gasnet_coll_handle_t *handle = (gasnet_coll_handle_t *) malloc( sizeof(gasnet_coll_handle_t) );;
    *handle = gasnet_coll_broadcast_nb(GASNET_TEAM_ALL, pb.buf, from_thread, pb.buf, pb.len, flags);

    PyBuffer_Release(&pb);
    return PyCapsule_New(handle, NULL, NULL);
}

static PyObject *
py_gasnet_coll_scatter(PyObject *self, PyObject *args)
{
    int ok, from_thread = 0;
    Py_buffer pb_send, pb_recv;
    PyObject *obj, *val;
    ok = PyArg_ParseTuple(args, "OO|i", &obj, &val, &from_thread);
    ok = PyObject_GetBuffer(obj, &pb_send, PyBUF_SIMPLE);
    ok = PyObject_GetBuffer(val, &pb_recv, PyBUF_SIMPLE);

    const int flags = GASNET_COLL_IN_MYSYNC|GASNET_COLL_OUT_MYSYNC|GASNET_COLL_LOCAL;
    gasnet_coll_scatter(GASNET_TEAM_ALL, pb_recv.buf, from_thread, pb_send.buf, pb_recv.len, flags);

    PyBuffer_Release(&pb_send);
    PyBuffer_Release(&pb_recv);
    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_coll_gather(PyObject *self, PyObject *args)
{
    int ok, to_thread = 0;
    Py_buffer pb_send, pb_recv;
    PyObject *obj, *arr;
    ok = PyArg_ParseTuple(args, "OO|i", &obj, &arr, &to_thread);
    ok = PyObject_GetBuffer(obj, &pb_send, PyBUF_SIMPLE);
    ok = PyObject_GetBuffer(arr, &pb_recv, PyBUF_SIMPLE);

    const int flags = GASNET_COLL_IN_MYSYNC|GASNET_COLL_OUT_MYSYNC|GASNET_COLL_LOCAL;
    gasnet_coll_gather(GASNET_TEAM_ALL, to_thread, pb_recv.buf, pb_send.buf, pb_send.len, flags);

    PyBuffer_Release(&pb_send);
    PyBuffer_Release(&pb_recv);
    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_coll_gather_all(PyObject *self, PyObject *args)
{
    int ok;
    Py_buffer pb_send, pb_recv;
    PyObject *obj, *arr;
    ok = PyArg_ParseTuple(args, "OO|i", &obj, &arr);
    ok = PyObject_GetBuffer(obj, &pb_send, PyBUF_SIMPLE);
    ok = PyObject_GetBuffer(arr, &pb_recv, PyBUF_SIMPLE);

    const int flags = GASNET_COLL_IN_MYSYNC|GASNET_COLL_OUT_MYSYNC|GASNET_COLL_LOCAL;
    gasnet_coll_gather_all(GASNET_TEAM_ALL, pb_recv.buf, pb_send.buf, pb_send.len, flags);

    PyBuffer_Release(&pb_send);
    PyBuffer_Release(&pb_recv);
    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_coll_exchange(PyObject *self, PyObject *args)
{
    int ok;
    Py_buffer pb_send, pb_recv;
    PyObject *obj, *arr;
    ok = PyArg_ParseTuple(args, "OO|i", &obj, &arr);
    ok = PyObject_GetBuffer(obj, &pb_send, PyBUF_SIMPLE);
    ok = PyObject_GetBuffer(arr, &pb_recv, PyBUF_SIMPLE);

    const int flags = GASNET_COLL_IN_MYSYNC|GASNET_COLL_OUT_MYSYNC|GASNET_COLL_LOCAL;
    gasnet_coll_exchange(GASNET_TEAM_ALL, pb_recv.buf, pb_send.buf, pb_send.len/THREADS, flags);

    PyBuffer_Release(&pb_send);
    PyBuffer_Release(&pb_recv);
    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_coll_wait_sync(PyObject *self, PyObject *args)
{
    int ok;
    PyObject* capsule;
    ok = PyArg_ParseTuple(args, "O!", &PyCapsule_Type, &capsule);

    gasnet_coll_handle_t *handle = (gasnet_coll_handle_t *) PyCapsule_GetPointer(capsule, NULL);
    gasnet_coll_wait_sync(*handle);

    free(handle); // TODO good idea to free handles after wait_sync?
    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_coll_try_sync(PyObject *self, PyObject *args)
{
    int ok;
    PyObject* capsule;
    ok = PyArg_ParseTuple(args, "O!", &PyCapsule_Type, &capsule);

    gasnet_coll_handle_t *handle = (gasnet_coll_handle_t *) PyCapsule_GetPointer(capsule, NULL);
    gasnet_coll_try_sync(*handle);

    Py_RETURN_NONE;
}

static PyObject *
py_gasnet_coll_reduce(PyObject *self, PyObject *args)
{
    int ok;
    Py_buffer pb_send, pb_recv;
    PyObject *obj, *arr, *fxn;
    int to_thread = 0;
    ok = PyArg_ParseTuple(args, "OOiO!", &obj, &arr, &to_thread, &PyFunction_Type, &fxn);
    ok = PyObject_GetBuffer(obj, &pb_send, PyBUF_SIMPLE);
    ok = PyObject_GetBuffer(arr, &pb_recv, PyBUF_SIMPLE);

    const int flags = GASNET_COLL_IN_MYSYNC|GASNET_COLL_OUT_MYSYNC|GASNET_COLL_LOCAL;
    // passing ptr "fxn" as func_args is bad accd to collective_notes.txt
    gasnet_coll_reduce(GASNET_TEAM_ALL, to_thread, pb_recv.buf, pb_send.buf, 0, 0, 1, THREADS, 0, fxn, flags);
    //gasnet_coll_reduce(gasnet_team_handle_t team, gasnet_image_t dstimage, void *dst, void *src, size_t src_blksz, size_t src_offset, size_t elem_size, size_t elem_count, gasnet_coll_fn_handle_t func, int func_arg, int flags GASNETE_THREAD_FARG) ;

    Py_RETURN_NONE;
}

static PyObject *
py_obj_to_capsule(PyObject *self, PyObject *args)
{
    int ok;
    PyObject *obj;
    assert(PyArg_ParseTuple(args, "O", &obj));

    /* increment reference count. TODO distributed reference counting */
    Py_XINCREF(obj);

    return PyLong_FromVoidPtr(obj);
}

static PyObject *
py_capsule_to_obj(PyObject *self, PyObject *args)
{
    long ptr;
    PyObject* obj;
    assert(PyArg_ParseTuple(args, "l", &ptr));

    obj = (PyObject*) ptr;
    Py_XINCREF(obj);
    return obj;
}
    
static PyMethodDef py_gasnet_methods[] = {
    {"init",           py_gasnet_init,           METH_VARARGS, "Bootstrap GASNet job."},
    {"exit",           py_gasnet_exit,           METH_VARARGS, "Terminate GASNet runtime."},
    {"attach",         py_gasnet_attach,         METH_NOARGS,  "Initialize and setup node."},
    {"nodes",          py_gasnet_nodes,          METH_VARARGS, "Number of nodes in job."},
    {"mynode",         py_gasnet_mynode,         METH_VARARGS, "Index of current node in job."},
    {"getenv",         py_gasnet_getenv,         METH_VARARGS, "Query environment when job was spawned."},
    {"barrier_notify", py_gasnet_barrier_notify, METH_VARARGS, "Execute notify for split-phase barrier."},
    {"barrier_wait",   py_gasnet_barrier_wait,   METH_VARARGS, "Execute wait for split-phase barrier."},
    {"barrier_try",    py_gasnet_barrier_try,    METH_VARARGS, "Execute try for split-phase barrier."},

    // Collectives. TODO refactor into separate file
    {"coll_init",      py_gasnet_coll_init,      METH_VARARGS, "Initialize collectives."},
    {"broadcast",      py_gasnet_coll_broadcast, METH_VARARGS, "Broadcast."},
    {"broadcast_nb",   py_gasnet_coll_broadcast_nb, METH_VARARGS, "Broadcast."},
    {"scatter",        py_gasnet_coll_scatter,   METH_VARARGS, "Scatter."},
    {"gather",         py_gasnet_coll_gather,    METH_VARARGS, "Gather."},
    {"all_gather",     py_gasnet_coll_gather_all,METH_VARARGS, "Gather all."},
    {"exchange",       py_gasnet_coll_exchange,  METH_VARARGS, "Exchange."},
    {"wait_sync",      py_gasnet_coll_wait_sync, METH_VARARGS, "Wait sync on nonblocking collectives handle."},
    {"try_sync",       py_gasnet_coll_try_sync,  METH_VARARGS, "Try sync on nonblocking collectives handle."},
    {"reduce",         py_gasnet_coll_reduce,    METH_VARARGS, "Reduce."},

    // My functions.
    {"apply_dynamic",  py_gasnet_apply_dynamic,  METH_VARARGS, "Apply a dynamic function"},
    {"set_apply_dynamic_handler",  set_apply_dynamic_handler,  METH_VARARGS, "Set the request hanlder that applies a dynamic function"},
    {"obj_to_capsule",    py_obj_to_capsule,  METH_VARARGS, "Encapsulate the given object."},
    {"capsule_to_obj",    py_capsule_to_obj,  METH_VARARGS, "Decapsulate the given capsule."},

    // Sentinel
    {NULL,             NULL}
};

PyMODINIT_FUNC
initgasnet(void)
{
    PyObject *module = Py_InitModule3("gasnet", py_gasnet_methods, "Interface to GASNet.");

    PyModule_AddIntConstant(module, "BARRIERFLAG_ANONYMOUS", GASNET_BARRIERFLAG_ANONYMOUS);
    PyModule_AddIntConstant(module, "BARRIERFLAG_MISMATCH",  GASNET_BARRIERFLAG_MISMATCH);

}
