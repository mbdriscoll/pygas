#include "Python.h"

#define GASNET_PAR
#include "gasnet.h"

static PyObject *
_gasnet_init(PyObject *self, PyObject *args)
{
    return Py_BuildValue("i", 42);
}

static PyObject *
_gasnet_exit(PyObject *self, PyObject *args)
{
    return Py_BuildValue("i", 42);
}

static PyMethodDef gasnet_methods[] = {
    {"init",    _gasnet_init, METH_VARARGS, "Initialize GASNet runtime."},
    {"exit",    _gasnet_exit, METH_VARARGS, "Terminate GASNet runtime."},
    {NULL,      NULL}           /* sentinel */
};

PyMODINIT_FUNC
initgasnet(void)
{
    Py_InitModule3("gasnet", gasnet_methods, "Interface to GASNet.");
}
