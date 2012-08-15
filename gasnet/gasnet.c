#include "pygas.h"

#include "rpc.h"
#include "pipeline.h"
#include "rmalloc.h"

/* The main GASNet handler table. */
gasnet_handlerentry_t handler_table[] = {
    {RPC_REQUEST_HIDX,   pygas_rpc_request_handler},
    {RPC_REPLY_HIDX,     pygas_rpc_reply_handler},
    {RMALLOC_REQUEST_HIDX,         pygas_rmalloc_request_handler},
    {RMALLOC_REPLY_HIDX,           pygas_rmalloc_reply_handler}
};

static PyObject *
pygas_gasnet_init(PyObject *self, PyObject *args)
{
    int argc = 1;
    char **argv = (char**) malloc(argc * sizeof(char*));
    argv[0] = "";

    int status = gasnet_init(&argc, &argv);

    //free(argv);
    return Py_BuildValue("i", status);
}

static PyObject *
pygas_gasnet_attach(PyObject *self, PyObject *args)
{
    int num_entries = sizeof(handler_table)/sizeof(gasnet_handlerentry_t);
    int status = gasnet_attach(handler_table, num_entries, gasnet_getMaxLocalSegmentSize(), GASNET_PAGESIZE);

    return Py_BuildValue("i", status);
}

static PyObject *
pygas_gasnet_exit(PyObject *self, PyObject *args)
{
    int exitcode = 0;
    if (!PyArg_ParseTuple(args, "|i", &exitcode))
        return NULL;

    /* execute barrier to be compliant with spec */
    gasnet_barrier_notify(0, GASNET_BARRIERFLAG_ANONYMOUS);
    PYGAS_GASNET_BARRIER_WAIT(0, GASNET_BARRIERFLAG_ANONYMOUS);

    gasnet_exit(exitcode);

    Py_RETURN_NONE;
}

static PyObject *
pygas_gasnet_mynode(PyObject *self, PyObject *args)
{
    gasnet_node_t rank = gasnet_mynode();
    return Py_BuildValue("i", rank);
}

static PyObject *
pygas_gasnet_nodes(PyObject *self, PyObject *args)
{
    gasnet_node_t ranks = gasnet_nodes();
    return Py_BuildValue("i", ranks);
}

static PyObject *
pygas_gasnet_getenv(PyObject *self, PyObject *args)
{
    const char *key, *value;
    if (!PyArg_ParseTuple(args, "s", &key))
        return NULL;

    value = gasnet_getenv(key);

    return Py_BuildValue("s", value);
}

static PyObject *
pygas_AMMaxMedium(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    size_t value = gasnet_AMMaxMedium();

    return Py_BuildValue("i", value);
}

static PyObject *
pygas_gasnet_barrier_wait(PyObject *self, PyObject *args)
{
    int id = 0;
    int flags = GASNET_BARRIERFLAG_ANONYMOUS;
    if (!PyArg_ParseTuple(args, "|ii", &id, &flags))
        return NULL;

    PYGAS_GASNET_BARRIER_WAIT(id, flags);

    Py_RETURN_NONE;
}

static PyObject *
pygas_gasnet_barrier_notify(PyObject *self, PyObject *args)
{
    int id = 0;
    int flags = GASNET_BARRIERFLAG_ANONYMOUS;
    if (!PyArg_ParseTuple(args, "|ii", &id, &flags))
        return NULL;

    gasnet_barrier_notify(id, flags);

    Py_RETURN_NONE;
}

static PyObject *
pygas_gasnet_barrier_try(PyObject *self, PyObject *args)
{
    int id = 0;
    int flags = GASNET_BARRIERFLAG_ANONYMOUS;
    if (!PyArg_ParseTuple(args, "|ii", &id, &flags))
        return NULL;

    gasnet_barrier_try(id, flags);

    Py_RETURN_NONE;
}

static PyObject *
pygas_gasnet_coll_init(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    gasnet_coll_init(0, 0, 0, 0, 0);

    Py_RETURN_NONE;
}

static PyObject *
pygas_gasnet_coll_broadcast(PyObject *self, PyObject *args)
{
    char* data;
    Py_ssize_t len;
    gasnet_team_handle_t team_id;
    gasnet_node_t from_rank = 0;
    if (!PyArg_ParseTuple(args, "iz#i", &team_id, &data, &len, &from_rank))
        return NULL;

    // broadcast size of serialized object to reserve space
    const int flags = GASNET_COLL_IN_ALLSYNC|GASNET_COLL_OUT_ALLSYNC|GASNET_COLL_LOCAL;
    gasnet_coll_broadcast(team_id, &len, from_rank,
                          &len, sizeof(Py_ssize_t), flags);

    // allocate space for object on recving threads
    gasnet_node_t myrank = gasnete_coll_team_node2rank(team_id, MYTHREAD);
    if (myrank != from_rank)
        data = (char*) malloc(len);

    // broadcast serialized object
    gasnet_coll_broadcast(team_id, data, from_rank, data, len, flags);
    PyObject* answer = PyString_FromStringAndSize(data, len);

    if (myrank != from_rank)
        free(data);

    return answer;
}

static PyObject *
pygas_obj_to_capsule(PyObject *self, PyObject *args)
{
    PyObject *obj;
    PyArg_ParseTuple(args, "O", &obj);

    /* increment reference count. TODO distributed reference counting */
    Py_XINCREF(obj);

    return PyLong_FromVoidPtr(obj);
}

static PyObject *
pygas_capsule_to_obj(PyObject *self, PyObject *args)
{
    long ptr;
    PyObject *obj;
    PyArg_ParseTuple(args, "l", &ptr);

    obj = (PyObject*) ptr;
    Py_XINCREF(obj);
    return obj;
}

static PyObject *
pygas_gasnet_team_all(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ""))
        return NULL;
    return PyLong_FromLong(GASNET_TEAM_ALL);
}

static PyObject *
pygas_gasnet_team_size(PyObject *self, PyObject *args)
{
    gasnete_coll_team_t team_id;
    if (!PyArg_ParseTuple(args, "l", &team_id))
        return NULL;
    gasnet_node_t images = gasnete_coll_team_size(team_id);
    return PyLong_FromLong(images);
}

static PyObject *
pygas_gasnet_team_split(PyObject *self, PyObject *args)
{
    gasnet_node_t parent, color, relrank;
    if (!PyArg_ParseTuple(args, "lll", &parent, &color, &relrank))
        return NULL;

    gasnet_seginfo_t scratch;
    scratch.size = 4*1024*1024; // TODO how to pick this
    scratch.addr = malloc(scratch.size);

    gasnet_team_handle_t team =
        gasnet_coll_team_split(parent, color, relrank, &scratch);

    return PyLong_FromLong(team);
}

static PyObject *
pygas_gasnet_team_rank2node(PyObject *self, PyObject *args)
{
    gasnete_coll_team_t team_id;
    gasnet_image_t rank;
    if (!PyArg_ParseTuple(args, "ll", &team_id, &rank))
        return NULL;

    gasnet_node_t node = gasnete_coll_team_rank2node(team_id, rank);

    return PyLong_FromLong(node);
}

static PyObject *
pygas_gasnet_team_node2rank(PyObject *self, PyObject *args)
{
    gasnete_coll_team_t team_id;
    gasnet_image_t node;
    if (!PyArg_ParseTuple(args, "ll", &team_id, &node))
        return NULL;

    gasnet_node_t rank = gasnete_coll_team_node2rank(team_id, node);

    return PyLong_FromLong(rank);
}

static PyMethodDef pygas_gasnet_methods[] = {
    {"init",           pygas_gasnet_init,           METH_VARARGS, "Bootstrap GASNet job."},
    {"exit",           pygas_gasnet_exit,           METH_VARARGS, "Terminate GASNet runtime."},
    {"attach",         pygas_gasnet_attach,         METH_NOARGS,  "Initialize and setup node."},
    {"nodes",          pygas_gasnet_nodes,          METH_VARARGS, "Number of nodes in job."},
    {"mynode",         pygas_gasnet_mynode,         METH_VARARGS, "Index of current node in job."},
    {"getenv",         pygas_gasnet_getenv,         METH_VARARGS, "Query environment when job was spawned."},
    {"barrier_notify", pygas_gasnet_barrier_notify, METH_VARARGS, "Execute notify for split-phase barrier."},
    {"barrier_wait",   pygas_gasnet_barrier_wait,   METH_VARARGS, "Execute wait for split-phase barrier."},
    {"barrier_try",    pygas_gasnet_barrier_try,    METH_VARARGS, "Execute try for split-phase barrier."},

    // AM function. Probably will hide later.
    {"AMMaxMedium",    pygas_AMMaxMedium,           METH_VARARGS, "Max number of bytes to send in single AM request."},

    // Collectives. TODO refactor into separate file
    {"coll_init",      pygas_gasnet_coll_init,      METH_VARARGS, "Initialize collectives."},
    {"broadcast",      pygas_gasnet_coll_broadcast, METH_VARARGS, "Broadcast."},

    // Teams
    {"team_all",       pygas_gasnet_team_all,       METH_VARARGS, "Get the value of GASNET_TEAM_ALL."},
    {"team_size",      pygas_gasnet_team_size,      METH_VARARGS, "The number of threads in the given team."},
    {"team_split",     pygas_gasnet_team_split,     METH_VARARGS, "Split a current team into subteams based on color."},
    {"rank2node",      pygas_gasnet_team_rank2node, METH_VARARGS, "The rank of a thread within a given team."},
    {"node2rank",      pygas_gasnet_team_node2rank, METH_VARARGS, "The global thread number of a rank within a team."},

    // Internal functions.
    {"_rpc",               pygas_gasnet_rpc,         METH_VARARGS, "Execute a remote procedure call"},
    {"_set_rpc_handler",   set_rpc_handler,          METH_VARARGS, "Set the request handler that executes a remote procedure. For internal use only."},
    {"_obj_to_capsule",    pygas_obj_to_capsule,     METH_VARARGS, "Encapsulate the given object."},
    {"_capsule_to_obj",    pygas_capsule_to_obj,     METH_VARARGS, "Decapsulate the given capsule."},

    // Sentinel
    {NULL,             NULL}
};

PyMODINIT_FUNC
initgasnet(void)
{
    PyObject *module = Py_InitModule3("gasnet", pygas_gasnet_methods, "Interface to GASNet.");

    PyModule_AddIntConstant(module, "BARRIERFLAG_ANONYMOUS", GASNET_BARRIERFLAG_ANONYMOUS);
    PyModule_AddIntConstant(module, "BARRIERFLAG_MISMATCH",  GASNET_BARRIERFLAG_MISMATCH);
}
