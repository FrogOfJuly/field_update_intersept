#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct field_intersept_s{

} FINTState;


static int FINT_run(){
    printf("FINT run\n");
    return 0;
}

static int FINT_stop(){
    printf("FINT stop\n");
    return 0;
}


static PyObject *field_intersept_run(PyObject *fintmodule, PyObject *Py_UNUSED(ignored)){
    if (FINT_run() != 0){
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *field_intersept_stop(PyObject *fintmodule, PyObject *Py_UNUSED(ignored)){
    if (FINT_stop() != 0){
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyMethodDef filed_intersept_methods[] = {
    {"stop", (PyCFunction)field_intersept_stop, METH_NOARGS, "stops interseption"},
    {NULL} /* Sentinel */
};


static int field_intersept_exec(PyObject *module){
    printf("FINT exec\n");
    PyModule_AddStringConstant(module, "__version__", "0.0.1");

    return FINT_run();
}

void field_intersept_free(PyObject* module){
    printf("FINT freed\n");
}

#ifdef Py_mod_exec
static PyModuleDef_Slot field_intersept_slots[] = {
    {Py_mod_exec, (void*)field_intersept_exec},
    {0, NULL}
};
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
    .m_size = sizeof(FINTState)
};


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