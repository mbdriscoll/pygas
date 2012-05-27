#include "Python.h"

#define GASNET_PAR
#include "gasnet.h"

static PyObject *
py_gasnet_init(PyObject *self, PyObject *args)
{
    int argc = 5;
    char **argv = (char**) malloc(argc * sizeof(char*));
    argv[0] = "2";
    argv[1] = "3";
    argv[2] = "4";
    argv[3] = "5";
    argv[4] = "6";

    int status = gasnet_init(&argc, &argv);

    return Py_BuildValue("i", status);
}

static PyObject *
py_gasnet_attach(PyObject *self, PyObject *args)
{
    int status = gasnet_attach(NULL, 0, gasnet_getMaxLocalSegmentSize(), GASNET_PAGESIZE);

    return Py_BuildValue("i", status);
}

static PyObject *
py_gasnet_exit(PyObject *self, PyObject *args)
{
    int ok;
    int exitcode = 0;
    ok = PyArg_ParseTuple(args, "|i", &exitcode);

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

    gasnet_barrier_wait(id, flags);

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
    int ok, from_thread = 0;
    Py_buffer pb;
    PyObject *obj;
    ok = PyArg_ParseTuple(args, "O|i", &obj, &from_thread);
    ok = PyObject_GetBuffer(obj, &pb, PyBUF_SIMPLE);

    const int flags = GASNET_COLL_IN_MYSYNC|GASNET_COLL_OUT_MYSYNC|GASNET_COLL_LOCAL;
    gasnet_coll_broadcast(GASNET_TEAM_ALL, pb.buf, from_thread, pb.buf, pb.len, flags);

    PyBuffer_Release(&pb);
    Py_RETURN_NONE;
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
    int ok, to_thread = 0;
    Py_buffer pb_send, pb_recv;
    PyObject *obj, *arr;
    ok = PyArg_ParseTuple(args, "OO|i", &obj, &arr, &to_thread);
    ok = PyObject_GetBuffer(obj, &pb_send, PyBUF_SIMPLE);
    ok = PyObject_GetBuffer(arr, &pb_recv, PyBUF_SIMPLE);

    const int flags = GASNET_COLL_IN_MYSYNC|GASNET_COLL_OUT_MYSYNC|GASNET_COLL_LOCAL;
    gasnet_coll_gather_all(GASNET_TEAM_ALL, pb_recv.buf, pb_send.buf, pb_send.len, flags);

    PyBuffer_Release(&pb_send);
    PyBuffer_Release(&pb_recv);
    Py_RETURN_NONE;
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
    {"scatter",        py_gasnet_coll_scatter,   METH_VARARGS, "Scatter."},
    {"gather",         py_gasnet_coll_gather,    METH_VARARGS, "Gather."},
    {"all_gather",     py_gasnet_coll_gather_all,METH_VARARGS, "Gather all."},

    {NULL,             NULL}           /* sentinel */
};

PyMODINIT_FUNC
init_gasnet(void)
{
    PyObject *module = Py_InitModule3("_gasnet", py_gasnet_methods, "Interface to GASNet.");

    PyModule_AddIntConstant(module, "BARRIERFLAG_ANONYMOUS", GASNET_BARRIERFLAG_ANONYMOUS);
    PyModule_AddIntConstant(module, "BARRIERFLAG_MISMATCH",  GASNET_BARRIERFLAG_MISMATCH);
}
