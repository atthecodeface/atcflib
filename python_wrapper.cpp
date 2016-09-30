/*a Copyright
  
  This file 'eh_py_engine.cpp' copyright Gavin J Stark 2016
  
  This is free software; you can redistribute it and/or modify it however you wish,
  with no obligations
  
  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*a Includes
 */
#include <Python.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "shader.h"
#include "key_value.h"
#include "timer.h"

/*a Defines
 */
#define PY_DEBUG
#undef PY_DEBUG
#if 0
#include <sys/time.h>
#include <pthread.h>
#define WHERE_I_AM {struct timeval tp; gettimeofday(&tp,NULL);fprintf(stderr,"%8ld.%06d:%p:%s:%d\n",tp.tv_sec,tp.tv_usec,pthread_self(),__func__,__LINE__ );}
#else
#define WHERE_I_AM {}
#endif

/*a Types
 */
/*t t_wrapped_PyObject
 */
typedef struct {
     PyObject_HEAD
} t_wrapped_PyObject;


static PyObject *python_wrapper_new( PyObject* self, PyObject* args );

/*v py_engine_methods
 */
static PyMethodDef python_wrapper_methods[] =
{
    {"py_engine", python_wrapper_new, METH_VARARGS, "Create a new wrapper object."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

/*v py_engine_PyTypeObject_frame
 */
static PyTypeObject wrapped_PyTypeObject_frame = {
    PyObject_HEAD_INIT(NULL)
    0,
    "Wrapped thing", // for printing
    sizeof( t_wrapped_PyObject ), // basic size
    0, // item size
    0, //py_engine_dealloc, /*tp_dealloc*/
    0, //py_engine_print,   /*tp_print*/
    0, //py_engine_getattr,          /*tp_getattr*/
    0,          /*tp_setattr*/
    0,          /*tp_compare*/
    0, //py_engine_repr,          /*tp_repr*/
    0,          /*tp_as_number*/
    0,          /*tp_as_sequence*/
    0,          /*tp_as_mapping*/
    0,          /*tp_hash */
	0,			/* tp_call - called if the object itself is invoked as a method */
	0, //py_engine_str, /*tp_str */
};


/*f python_wrapper_new
 */
static PyObject *python_wrapper_new(PyObject* self, PyObject* args)
{
	 t_wrapped_PyObject *py_obj;

    if (!PyArg_ParseTuple(args,":new"))
	{
        return NULL;
	}

    py_obj = PyObject_New(t_wrapped_PyObject, &wrapped_PyTypeObject_frame);

    return (PyObject*)py_obj;
}

/*a C code for python_wrapper
 */
extern "C" void initpython_wrapper( void )
{
    PyObject *m;
    int i;

    m = Py_InitModule3("python_wrapper", python_wrapper_methods, "Python interface to wrapped thing" );

}

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/

