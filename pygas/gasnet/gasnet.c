#include "Python.h"

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
    gasnet_node_t sender;
    size_t nbytes;
    void* addr;
} msg_info_t;
  
static PyObject *
py_gasnet_init(PyObject *self, PyObject *args)
{
    PyEval_InitThreads();

    int argc = 0;
    char **argv = (char**) malloc(argc * sizeof(char*));

    int status = gasnet_init(&argc, &argv);

    free(argv);
    return Py_BuildValue("i", status);
}

static PyObject *
py_gasnet_apply_dynamic(PyObject *self, PyObject *args)
{
    char *data;
    int dest = 0, nbytes = 0;
    PyArg_ParseTuple(args, "is#", &dest, &data, &nbytes);

    char *reply = NULL;
    char request[sizeof(msg_info_t)+nbytes];
    msg_info_t* request_info = (msg_info_t*) &request[0];
    request_info->sender = MYTHREAD;
    request_info->nbytes = nbytes;
    request_info->addr = (void*) &reply;
    memcpy(&request[sizeof(msg_info_t)], data, nbytes);

    gasnet_AMRequestMedium0(dest, APPLY_DYNAMIC_REQUEST_HIDX, &request, sizeof(request));
    PYGASNET_BLOCKUNTIL(reply != NULL);

    msg_info_t* reply_info = (msg_info_t*) &reply[0];
    PyObject * result = Py_BuildValue("s#", &reply[sizeof(msg_info_t)], reply_info->nbytes);

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
pygas_async_request_handler(char* request) {
    msg_info_t* request_info = (msg_info_t*) &request[0];

    PyObject *result = PyObject_CallFunction(apply_dynamic_handler, "(s#)", &request[sizeof(msg_info_t)], request_info->nbytes);
    assert(PyString_Check(result));

    int nbytes;
    char *data;
    PyString_AsStringAndSize(result, &data, &nbytes);
    char reply[sizeof(msg_info_t)+nbytes];
    msg_info_t *reply_info = (msg_info_t*) &reply[0];
    reply_info->sender = MYTHREAD;
    reply_info->nbytes = nbytes;
    reply_info->addr = request_info->addr;
    memcpy(&reply[sizeof(msg_info_t)], data, nbytes);
    
    gasnet_AMRequestMedium0(request_info->sender, APPLY_DYNAMIC_REPLY_HIDX, &reply, sizeof(reply));
    
    free(request);
    return 0;
}

void
pygas_apply_dynamic_request_handler(gasnet_token_t token, char* data, size_t nbytes)
{
    char* msg = (char*) malloc(nbytes);
    memcpy(msg, data, nbytes);
    
    Py_AddPendingCall(pygas_async_request_handler, msg);
}

void
pygas_apply_dynamic_reply_handler(gasnet_token_t token, void* data, size_t nbytes)
{
    char* reply = (char*) malloc(nbytes);
    memcpy(reply, data, nbytes);

    // write val to end PYGASNET_BLOCKUNTIL
    msg_info_t* reply_info = (msg_info_t*) &reply[0];
    *((char**) reply_info->addr) = reply;
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

static PyObject *
py_gasnet_coll_init(PyObject *self, PyObject *args)
{
    int ok;
    ok = PyArg_ParseTuple(args, "");

    gasnet_coll_init(0, 0, 0, 0, 0);

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
