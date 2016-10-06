/*a Copyright
  
  This file 'python_filter.cpp' copyright Gavin J Stark 2016
  
  This is free software; you can redistribute it and/or modify it however you wish,
  with no obligations
  
  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*a Includes
 */
#include <Python.h>
#include "python_filter.h"
#include "python_lens_projection.h"
#include "python_texture.h"
#include "filter.h"

/*a Defines
 */

/*a Types
 */
/*t t_PyObject_filter
 */
typedef struct {
    PyObject_HEAD
    c_filter *filter;
    t_exec_context ec;
} t_PyObject_filter;

/*a Forward function declarations
 */
static int python_filter_init(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *python_filter_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static void python_filter_dealloc(PyObject *self);
static PyObject *python_filter_getattr(PyObject *self, char *attr);

static PyObject *python_filter_method_compile(PyObject* self);
static PyObject *python_filter_method_exec(PyObject* self);
static PyObject *python_filter_method_define(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_filter_method_parameter(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_filter_method_textures(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_filter_method_projections(PyObject* self, PyObject* args, PyObject *kwds);

/*a Static variables
 */
/*v python_filter_methods
 */
PyMethodDef python_filter_methods[] = {
    {"projections", (PyCFunction)python_filter_method_projections, METH_VARARGS|METH_KEYWORDS},
    {"textures", (PyCFunction)python_filter_method_textures, METH_VARARGS|METH_KEYWORDS},
    {"define",   (PyCFunction)python_filter_method_define, METH_VARARGS|METH_KEYWORDS},
    {"parameter",  (PyCFunction)python_filter_method_parameter, METH_VARARGS|METH_KEYWORDS},
    {"compile",  (PyCFunction)python_filter_method_compile, METH_NOARGS},
    {"execute",  (PyCFunction)python_filter_method_exec,    METH_NOARGS},
    {NULL, NULL},
};

/*v PyTypeObject_filter
 */
static PyTypeObject PyTypeObject_filter = {
    PyObject_HEAD_INIT(NULL)
    0, // variable size
    "filter", // type name
    sizeof(t_PyObject_filter), // basic size
    0, // item size - zero for static sized object types
    python_filter_dealloc, //py_engine_dealloc, /*tp_dealloc*/
    0, /*tp_print - basically deprecated */
    python_filter_getattr, /*tp_getattr*/
    0, /*tp_setattr*/
    0, /*tp_compare*/
    0, /*tp_repr - ideally a represenation that is python that recreates this object */
    0, /*tp_as_number*/
    0, /*tp_as_sequence*/
    0, /*tp_as_mapping*/
    0, /*tp_hash */
	0, /* tp_call - called if the object itself is invoked as a method */
	0, /* tp_str */
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Filter object",       /* tp_doc */
    0,		                   /* tp_traverse */
    0,		                   /* tp_clear */
    0,		                   /* tp_richcompare */
    0,		                   /* tp_weaklistoffset */
    0,		                   /* tp_iter */
    0,		                   /* tp_iternext */
    python_filter_methods, /* tp_methods */
    0, //python_quaternion_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    python_filter_init,    /* tp_init */
    0,                         /* tp_alloc */
    python_filter_new,     /* tp_new */
};

/*a Python filter methods
*/
/*f python_filter_method_compile
 */
static PyObject *
python_filter_method_compile(PyObject* self)
{
    t_PyObject_filter *py_obj = (t_PyObject_filter *)self;
    if (py_obj->filter) {
        int err;
        err = py_obj->filter->compile();
        if (err!=0) {
            PyErr_SetString(PyExc_RuntimeError, py_obj->filter->parse_error);
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

/*f python_filter_method_exec
 */
static PyObject *
python_filter_method_exec(PyObject* self)
{
    t_PyObject_filter *py_obj = (t_PyObject_filter *)self;
    if (py_obj->filter) {
        int err;
        py_obj->ec.use_ids = 0;
        err = py_obj->filter->execute(&py_obj->ec);
        gl_get_errors("Filter executed");
    }
    Py_RETURN_NONE;
}

/*f python_filter_method_define
 */
static PyObject *
python_filter_method_define(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_filter *py_obj = (t_PyObject_filter *)self;
    const char *name = NULL;
    const char *value = NULL;
    int remove=0;

    static const char *kwlist[] = {"name", "value", "remove", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|si", (char **)kwlist, 
                                     &name, &value, &remove))
        return NULL;

    if (py_obj->filter) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "-D%s", name);
        buffer[sizeof(buffer)-1] = 0;
        if (remove) {
            py_obj->filter->unset_parameter(buffer);
        } else {
            py_obj->filter->set_parameter(buffer, value?value:"");
        }
    }
    Py_RETURN_NONE;
}

/*f python_filter_method_parameter
 */
static PyObject *
python_filter_method_parameter(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_filter *py_obj = (t_PyObject_filter *)self;
    const char *name = NULL;
    PyObject *value = NULL;

    static const char *kwlist[] = {"name", "value", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO", (char **)kwlist, 
                                     &name, &value))
        return NULL;

    if (py_obj->filter) {
        if (PyFloat_Check(value)) {
            py_obj->filter->set_parameter(name, PyFloat_AsDouble(value));
        } else if (PyInt_Check(value)) {
            py_obj->filter->set_parameter(name, (int)PyInt_AsLong(value));
        } else if (PyLong_Check(value)) {
            py_obj->filter->set_parameter(name, (int)PyLong_AsLong(value));
        } else {
            PyErr_SetString(PyExc_RuntimeError, "Need float or int to set parameter");
        }
    }
    Py_RETURN_NONE;
}

/*f python_filter_method_textures
 */
static PyObject *
python_filter_method_textures(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_filter *py_obj = (t_PyObject_filter *)self;
    PyObject *textures;

    static const char *kwlist[] = {"textures", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, 
                                     &textures))
        return NULL;

    if (py_obj->filter) {
        PyObject *tuple = PySequence_Fast(textures, "textures must be a tuple or list");
        int len = PySequence_Size(textures);
        for (int i=0; i<len; i++) {
            PyObject *item = PySequence_Fast_GET_ITEM(tuple, i);
            t_texture_ptr texture;
            if (python_texture_data(item, 0, (void *)&texture)) {
                py_obj->filter->bind_texture(i, texture);
            } else {
                PyErr_SetString(PyExc_RuntimeError, "Non-texture object in tuple");
                Py_DECREF(tuple);
                return NULL;
            }
        }
        Py_DECREF(tuple);
    }
    Py_RETURN_NONE;
}

/*f python_filter_method_projections
 */
static PyObject *
python_filter_method_projections(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_filter *py_obj = (t_PyObject_filter *)self;
    PyObject *from, *to;

    static const char *kwlist[] = {"from", "to", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", (char **)kwlist, 
                                     &from, &to))
        return NULL;

    if (py_obj->filter) {
        class c_lens_projection *to_proj, *from_proj;
        if ( (python_lens_projection_data(from, 0, (void *)&from_proj)) &&
             (python_lens_projection_data(to,   0, (void *)&to_proj)) ) {
            py_obj->filter->bind_projection(0, from_proj);
            py_obj->filter->bind_projection(1, to_proj);
        } else {
            PyErr_SetString(PyExc_RuntimeError, "Non-projection object in tuple");
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

/*f python_filter_dealloc
 */
static void
python_filter_dealloc(PyObject *self)
{
    t_PyObject_filter *py_obj = (t_PyObject_filter *)self;
    if (py_obj->filter) {
        delete(py_obj->filter);
        py_obj->filter = NULL;
    }
}

/*f python_filter_getattr
 */
static PyObject *
python_filter_getattr(PyObject *self, char *attr)
{
    t_PyObject_filter *py_obj = (t_PyObject_filter *)self;
    
    if (py_obj->filter) {
    }
    if (!strcmp(attr, "num_points")) {
        return PyInt_FromLong(py_obj->ec.num_points);
    }
    if (!strcmp(attr, "points")) {
        if (py_obj->ec.points) {
            PyObject *list;
            list = PyList_New(0);
            for (int i=0; i<py_obj->ec.num_points; i++) {
                t_point_value *pv;
                pv = &(py_obj->ec.points[i]);
                PyList_Append(list, 
                              Py_BuildValue("iifff", pv->x, pv->y, (double)pv->value, (double)pv->vec_x, (double)pv->vec_y)
                    ); // CAN FAIL
            }
            return list;
        }
        Py_RETURN_NONE;
    }
    return Py_FindMethod(python_filter_methods, self, attr);
}

/*a Python object
 */
/*f python_filter_new
 */
static PyObject *
python_filter_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    t_PyObject_filter *py_obj;
    py_obj = (t_PyObject_filter *)type->tp_alloc(type, 0);
    if (py_obj) {
        py_obj->filter = NULL;
    }
    return (PyObject *)py_obj;
}

/*f python_filter_init
 */
static int
python_filter_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    t_PyObject_filter *py_obj = (t_PyObject_filter *)self;

    const char *filter = NULL;
    static const char *kwlist[] = {"filter", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", (char **)kwlist, 
                                     &filter))
        return -1;

    py_obj->filter = NULL;
    if (filter) {
        py_obj->filter = filter_from_string(filter);
        if (!py_obj->filter) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create filter");
            return -1;
        }
    }
    if (py_obj->filter) {
        if (py_obj->filter->parse_error) {
            PyErr_SetString(PyExc_RuntimeError, py_obj->filter->parse_error);
            return -1;
        }
    }
    return 0;
}

/*f python_filter_init_premodule
 */
int python_filter_init_premodule(void)
{
    if (PyType_Ready(&PyTypeObject_filter) < 0)
        return -1;
    return 0;
}

/*f python_filter_init_postmodule
 */
void python_filter_init_postmodule(PyObject *module)
{
    Py_INCREF(&PyTypeObject_filter);
    PyModule_AddObject(module, "filter", (PyObject *)&PyTypeObject_filter);
}


