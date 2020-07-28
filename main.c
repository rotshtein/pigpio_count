#include <Python.h>
#include <structmember.h>
#include <pigpiod_if2.h>

static int g_pi = -1;

static PyObject *PiGpioError;

typedef struct {
    PyObject_HEAD
    unsigned int pin;
    int callback_id;
    uint32_t tick_count;
    char running;
} PulseCounter;

static void callback_func(int pi, unsigned user_gpio, unsigned level, uint32_t tick, PulseCounter *self) {
    self->tick_count++;
}

static void stop(PulseCounter *self) {
    if (!self->running) return;

    callback_cancel(self->callback_id);
    self->running = 0;
}

static PyObject *initialize(PyObject *self, PyObject *args) {
    g_pi = pigpio_start(NULL, NULL);

    if (g_pi < 0)
        return PyErr_Format(PiGpioError, "Initialization failed! pigpio_start() returned %d", g_pi);

    Py_RETURN_NONE;
}

static PyObject *shutdown(PyObject *self, PyObject *args) {
    if (g_pi > 0) {
        pigpio_stop(g_pi);
        g_pi = -1;
    }

    Py_RETURN_NONE;
}

static int PulseCounter_init(PulseCounter *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {"pin", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "I", kwlist, &self->pin))
        return -1;

    if (self->pin > 31) {
        PyErr_SetString(PyExc_ValueError, "pin must be between 0 and 31");
        return -1;
    }

    return 0;
}

#if PY_MAJOR_VERSION >= 3
static void PulseCounter_finalize(PulseCounter *self) {
    stop(self);
}
#else
static void PulseCounter_dealloc(PulseCounter *self) {
    stop(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}
#endif

static PyObject *PulseCounter_start(PulseCounter *self, PyObject *Py_UNUSED(ignored)) {
    int ret;

    if (g_pi < 0 && !initialize(NULL, NULL)) {
        return NULL;
    }

    ret = callback_ex(g_pi, self->pin, RISING_EDGE, (CBFuncEx_t)callback_func, self);

    if (ret < 0)
        return PyErr_Format(PiGpioError, "callback_ex() returned %d", ret);

    self->callback_id = ret;
    self->running = 1;

    Py_RETURN_NONE;
}

static PyObject *PulseCounter_stop(PulseCounter *self, PyObject *Py_UNUSED(ignored)) {
    stop(self);

    Py_RETURN_NONE;
}

static PyMemberDef PulseCounter_members[] = {
    {"tick_count", T_UINT, offsetof(PulseCounter, tick_count), READONLY, ""},
    {"pin", T_UINT, offsetof(PulseCounter, pin), READONLY, ""},
    {"running", T_BOOL, offsetof(PulseCounter, running), READONLY, ""},
    {NULL}
};

static PyMethodDef PulseCounter_methods[] = {
    {"start", (PyCFunction)PulseCounter_start, METH_NOARGS, ""},
    {"stop", (PyCFunction)PulseCounter_stop, METH_NOARGS, ""},
    {NULL}
};

static PyTypeObject PulseCounterType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pigpio_count.PulseCounter",
    .tp_doc = "Count pulses using interrupt",
    .tp_basicsize = sizeof(PulseCounter),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)PulseCounter_init,
#if PY_MAJOR_VERSION >= 3
    .tp_finalize = (destructor)PulseCounter_finalize,
#else
    .tp_dealloc = (destructor)PulseCounter_dealloc,
#endif
    .tp_members = PulseCounter_members,
    .tp_methods = PulseCounter_methods
};

static PyMethodDef module_funcs[] = {
    {"initialize", initialize, METH_NOARGS, "initialize(): None"},
    {"shutdown", shutdown, METH_NOARGS, "shutdown(): None"},
    {NULL}
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef module_definition = {
    PyModuleDef_HEAD_INIT,
    "pigpio_count",
    "Pulse counter module using pigpiod",
    -1,
    module_funcs
};

PyMODINIT_FUNC PyInit_pigpio_count(void) {
#else
PyMODINIT_FUNC initpigpio_count(void) {
#endif
    PyObject *m;

    if (PyType_Ready(&PulseCounterType) < 0)
        return NULL;

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&module_definition);
#else
    m = Py_InitModule("pigpio_count", module_funcs);
#endif

    PiGpioError = PyErr_NewException("pigpio_count.PiGpioError", NULL, NULL);

    Py_INCREF(PiGpioError);
    if (PyModule_AddObject(m, "PiGpioError", PiGpioError) < 0) {
        Py_DECREF(&PiGpioError);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&PulseCounterType);
    if (PyModule_AddObject(m, "PulseCounter", (PyObject *)&PulseCounterType) < 0) {
        Py_DECREF(&PulseCounterType);
        Py_DECREF(m);
        return NULL;
    }

#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}
