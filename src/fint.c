#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct field_intersept_s
{

} FINTState;

static int FINT_run();

static int FINT_stop()
{
    printf("FINT stop\n");
    return 0;
}

static PyObject *field_intersept_run(PyObject *fintmodule, PyObject *Py_UNUSED(ignored))
{
    if (FINT_run() != 0)
    {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *field_intersept_stop(PyObject *fintmodule, PyObject *Py_UNUSED(ignored))
{
    if (FINT_stop() != 0)
    {
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyMethodDef filed_intersept_methods[] = {
    {"stop", (PyCFunction)field_intersept_stop, METH_NOARGS, "stops interseption"},
    {NULL} /* Sentinel */
};

static int field_intersept_exec(PyObject *module)
{
    printf("FINT exec\n");
    PyModule_AddStringConstant(module, "__version__", "0.0.1");

    return FINT_run();
}

void field_intersept_free(PyObject *module)
{
    printf("FINT freed\n");
}

#ifdef Py_mod_exec
static PyModuleDef_Slot field_intersept_slots[] = {
    {Py_mod_exec, (void *)field_intersept_exec},
    {0, NULL}};
#endif

static PyModuleDef fieldinterseptmoduledef = {
    PyModuleDef_HEAD_INIT,
    .m_name = "field_intersept",
    .m_doc = "Intersepting field updates and execute arbitrary code there",
    .m_methods = filed_intersept_methods,
    .m_free = (freefunc)field_intersept_free,
#ifdef Py_mod_exec
    .m_slots = field_intersept_slots,
#endif
    .m_size = sizeof(FINTState)};

PyMODINIT_FUNC PyInit_field_intersept(void)
{
#ifdef Py_mod_exec
    return PyModuleDef_Init(&fieldinterseptmoduledef);
#else
    PyObject *module;
    module = PyModule_Create(&fieldinterseptmoduledef);
    if (module == NULL)
        return NULL;

    if (field_intersept_exec(module) != 0)
    {
        Py_DECREF(module);
        return NULL;
    }

    return module;
#endif
}

//***************************************************************
//--------------- interesting things start here -----------------
//***************************************************************

// get dictionary pointer with methods which is store in interpreter state
PyObject *perform_black_magic(PyTypeObject *type);

// indirect function to call on field update
int PyObject_GenericSetAttr_int(PyObject *obj, PyObject *name, PyObject *value)
{

    printf("%p.", obj);
    PyObject_Print(name, stdout, Py_PRINT_RAW);
    printf("=");
    PyObject_Print(value, stdout, Py_PRINT_RAW);
    printf("\n");
    return PyObject_GenericSetAttr(obj, name, value);
}

// indirect function to call on field read
PyObject *PyObject_GenericGetAttr_int(PyObject *obj, PyObject *name)
{

    printf("> %p.", obj);
    PyObject_Print(name, stdout, Py_PRINT_RAW);
    printf("\n");
    return PyObject_GenericGetAttr(obj, name);
}

// run on import
static int FINT_run()
{
    printf("---------- FINT run ---------\n");

    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    {

        {
            // setting indirect funtion for respecting field of object class
            PyBaseObject_Type.tp_setattro = PyObject_GenericSetAttr_int;
            // buidling __setattr__ string to lookup in dict
            PyObject *name = PyUnicode_FromString("__setattr__");

            replace_intr_objects_method(name, &PyObject_GenericSetAttr_int);

            if (replace_intr_objects_method(name, &PyObject_GenericSetAttr_int) != 0)
            {
                printf("failed to replace setattr\n");
            }
        }
        {
            // PyBaseObject_Type.tp_getattro = PyObject_GenericGetAttr_int;
            PyObject *name = PyUnicode_FromString("__getattribute__");

            if (replace_intr_objects_method(name, &PyObject_GenericGetAttr_int) != 0)
            {
                printf("failed to replace __getattribute__\n");
            }
        }
    }
    /* Release the thread. No Python API allowed beyond this point. */
    PyGILState_Release(gstate);

    return 0;
}

int replace_intr_objects_method(PyObject *name, void *underlying_method)
{
    // retrieving dictionary with methods of built-in types
    // magic is required as the internals of interpreter state are *usually* inaccessable
    PyObject *dict = perform_black_magic(&PyBaseObject_Type);

    if (dict == NULL)
    {
        printf("Failed to get dict for PyBaseObject_Type\n");
        return -1;
    }

    // getting a method wrapper
    PyObject *descr = PyDict_GetItem(dict, name);

    if (descr == NULL)
    {
        printf("No such such method: `");
        PyObject_Print(name, stdout, Py_PRINT_RAW);
        printf("`\n");
        return -1;
    }

    // should be null checks and things, but they will be added later
    PyWrapperDescrObject *d = (PyWrapperDescrObject *)descr;

    // replacing the underlying function of a wrapper
    d->d_wrapped = underlying_method;

    return 0;
}

//------- Exposing the internals of python to access interpreter state fields

#define Py_BUILD_CORE

#undef _PyGC_FINALIZED
#define _PyGC_FINALIZED dontlookatmeplease

#include <internal/pycore_interp.h>

#undef Py_BUILD_CORE
#undef _PyGC_FINALIZED

PyObject *perform_black_magic(PyTypeObject *type)
{
    size_t idx = (size_t)type->tp_subclasses - 1;

    PyInterpreterState *interp = PyInterpreterState_Get();
    PyObject *dict = interp->types.builtins[idx].tp_dict;
    return dict;
}