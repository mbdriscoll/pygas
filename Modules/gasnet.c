#include "Python.h"

#define GASNET_PAR
#include "gasnet.h"

static PyObject *
_gasnet_init(PyObject *self, PyObject *args)
{
    int argc = 2;
    char **argv = (char**) malloc(argc * sizeof(char*));
    argv[0] = "./pygas";
    argv[1] = "1";

    int status = gasnet_init(&argc, &argv);
    return Py_BuildValue("i", status);
}

static PyObject *
_gasnet_exit(PyObject *self, PyObject *args)
{
    gasnet_exit(0);
    return Py_None;
}

static PyObject *
_gasnet_mynode(PyObject *self, PyObject *args)
{
    gasnet_node_t rank = gasnet_mynode();
    return Py_BuildValue("i", rank);
}

static PyObject *
_gasnet_nodes(PyObject *self, PyObject *args)
{
    gasnet_node_t ranks = gasnet_nodes();
    return Py_BuildValue("i", ranks);
}

static PyMethodDef gasnet_methods[] = {
    {"init",    _gasnet_init,    METH_VARARGS, "Initialize GASNet runtime."},
    {"exit",    _gasnet_exit,    METH_VARARGS, "Terminate GASNet runtime."},
    {"nodes",   _gasnet_nodes,   METH_VARARGS, "Number of nodes in job."},
    {"mynode",  _gasnet_mynode,  METH_VARARGS, "Index of current node in job."},
    {NULL,      NULL}           /* sentinel */
};

PyMODINIT_FUNC
initgasnet(void)
{
    Py_InitModule3("gasnet", gasnet_methods, "Interface to GASNet.");
}
