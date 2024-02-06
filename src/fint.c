#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define Py_BUILD_CORE

#undef _PyGC_FINALIZED
#define _PyGC_FINALIZED dontlookatmeplease

#include <internal/pycore_interp.h>

#undef Py_BUILD_CORE

typedef struct field_intersept_s
{

} FINTState;

int PyObject_GenericSetAttr_int(PyObject *obj, PyObject *name, PyObject *value)
{
    printf("Indirect function called\n");
    return PyObject_GenericSetAttr(obj, name, value);
}

static int FINT_run()
{
    printf("---------- FINT run ---------\n");

    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    {
        PyBaseObject_Type.tp_setattro = PyObject_GenericSetAttr_int;

        printf("\033[33;1;4m> generic set attr function: %p\033[0m\n", PyObject_GenericSetAttr);
        printf("\033[32;1;4m> generic indirect set attr function: %p\033[0m\n", PyObject_GenericSetAttr_int);
        printf("\033[34;1;4m> base object type address: %p\033[0m\n", &PyBaseObject_Type);
        printf("\033[34;1;4m> base object type.tp_dict address: %p\033[0m\n", PyBaseObject_Type.tp_dict);

        printf("\033[34;1;4m> getting interpreter state\033[0m\n");

        /* Perform Python actions here. */
        PyInterpreterState *interp = PyInterpreterState_Get();
        printf("\033[34;1;4m> getting PyBaseObject_Type state\033[0m\n");

        size_t idx = (size_t)PyBaseObject_Type.tp_subclasses - 1;
        PyObject *dict = interp->types.builtins[idx].tp_dict;
        PyObject *name = PyUnicode_FromString("__setattr__");


        printf("\033[32m> name: ");
        PyObject_Print(name, stdout, Py_PRINT_RAW);
        printf("\033[0m\n");
        
        PyObject *descr = PyDict_GetItem(dict, name);
        PyWrapperDescrObject *d = (PyWrapperDescrObject *)descr;
        d->d_wrapped = &PyObject_GenericSetAttr_int;

        // static_builtin_state *pyobj_t_state = _PyStaticType_GetState(interp, &PyObject_Type);
        // if(pyobj_t_state != NULL){
        //     printf("\033[34;1;4m> No state for base object type\033[0m\n");
        //     return 0;
        // }

        // PyTypeObject *dict = pyobj_t_state->tp_dict;

        printf("\033[34;1;4m> interpreter[obj_type_idx]->tp_dict address: %p\033[0m\n", dict);
        printf("\033[32m> Dict itself looks like this: ");
        PyObject_Print(dict, stdout, Py_PRINT_RAW);
        printf("\033[0m\n");
    }

    /* Release the thread. No Python API allowed beyond this point. */
    PyGILState_Release(gstate);

    return 0;
}

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