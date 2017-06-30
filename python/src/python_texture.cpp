/*a Copyright
  
  This file 'python_texture.cpp' copyright Gavin J Stark 2016
  
  This is free software; you can redistribute it and/or modify it however you wish,
  with no obligations
  
  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*a Includes
 */
#include <Python.h>
#include "python_texture.h"
#include "shader.h"
#include "texture.h"
#include <list>

/*a Defines
 */

/*a Types
 */
/*t t_PyObject_texture
 */
typedef struct t_PyObject_texture *t_PyObject_texture_ptr;
typedef struct t_PyObject_texture {
    PyObject_HEAD
    t_texture_ptr texture;
    int handle;
} t_PyObject_texture;

/*a Forward function declarations
 */
static int       python_texture_init(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *python_texture_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static void      python_texture_dealloc(PyObject *self);
static PyObject *python_texture_getattr(PyObject *self, char *attr);
static PyObject *python_texture_method_save(PyObject* self, PyObject* args, PyObject *kwds);
static void      python_texture_dealloc(PyObject *self);

/*a Static variables
 */
/*v texture_list
 */
static int texture_uid;
static std::list<t_PyObject_texture_ptr> texture_list;

/*v python_texture_methods
 */
static PyMethodDef python_texture_methods[] = {
    {"save", (PyCFunction)python_texture_method_save, METH_VARARGS|METH_KEYWORDS},
    {NULL, NULL},
};

/*v PyTypeObject_texture
 */
static PyTypeObject PyTypeObject_texture = {
    PyObject_HEAD_INIT(NULL)
    0, // variable size
    "texture", // type name
    sizeof(t_PyObject_texture), // basic size
    0, // item size - zero for static sized object types
    python_texture_dealloc, //py_engine_dealloc, /*tp_dealloc*/
    0, /*tp_print - basically deprecated */
    python_texture_getattr, /*tp_getattr*/
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
    "Texture object",       /* tp_doc */
    0,		                   /* tp_traverse */
    0,		                   /* tp_clear */
    0,		                   /* tp_richcompare */
    0,		                   /* tp_weaklistoffset */
    0,		                   /* tp_iter */
    0,		                   /* tp_iternext */
    python_texture_methods, /* tp_methods */
    0, //python_quaternion_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    python_texture_init,    /* tp_init */
    0,                         /* tp_alloc */
    python_texture_new,     /* tp_new */
};

/*a Python texture methods
 */
/*f python_texture_method_save
 */
static PyObject *
python_texture_method_save(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_texture *py_obj = (t_PyObject_texture *)self;
    //int components, conversion;
    const char *filename = NULL;

    static const char *kwlist[] = {"filename", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", (char **)kwlist, 
                                     &filename))
        return NULL;

    if (py_obj->texture) {
        int err;
        err = texture_save(py_obj->texture, filename, 0, 0);
        if (err) {
        }
    }
    Py_RETURN_NONE;
}

/*f python_texture_dealloc
 */
static void
python_texture_dealloc(PyObject *self)
{
    t_PyObject_texture *py_obj = (t_PyObject_texture *)self;
    if (py_obj->texture) {
        texture_destroy(py_obj->texture);
        py_obj->texture = NULL;
        texture_list.remove(py_obj);
    }
}

/*f python_texture_getattr
 */
static PyObject *
python_texture_getattr(PyObject *self, char *attr)
{
    t_PyObject_texture *py_obj = (t_PyObject_texture *)self;
    
    if (py_obj->texture) {
        if (!strcmp(attr, "gl_id")) {
            return PyInt_FromLong(texture_header(py_obj->texture)->gl_id);
        }
        if (!strcmp(attr, "width")) {
            return PyInt_FromLong(texture_header(py_obj->texture)->width);
        }
        if (!strcmp(attr, "height")) {
            return PyInt_FromLong(texture_header(py_obj->texture)->height);
        }
        if (!strcmp(attr, "handle")) {
            return PyInt_FromLong(py_obj->handle);
        }
    }
    return Py_FindMethod(python_texture_methods, self, attr);
}

/*a Python object
 */
/*f python_texture_init_premodule
 */
int python_texture_init_premodule(void)
{
    if (PyType_Ready(&PyTypeObject_texture) < 0)
        return -1;
    return 0;
}

/*f python_texture_init_postmodule
 */
void python_texture_init_postmodule(PyObject *module)
{
    Py_INCREF(&PyTypeObject_texture);
    PyModule_AddObject(module, "texture", (PyObject *)&PyTypeObject_texture);
}

/*f python_texture_new
 */
static PyObject *
python_texture_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    t_PyObject_texture *py_obj;
    shader_init();
    texture_draw_init();
    py_obj = (t_PyObject_texture *)type->tp_alloc(type, 0);
    if (py_obj) {
        py_obj->texture = NULL;
        py_obj->handle = texture_uid++;
        texture_list.push_front(py_obj);
    }
    return (PyObject *)py_obj;
}

/*f python_texture_init
 */
static int
python_texture_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    t_PyObject_texture *py_obj = (t_PyObject_texture *)self;

    int width=0, height=0, components, precision;
    const char *filename = NULL;

    static const char *kwlist[] = {"width", "height", "filename", "components", "precision", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iisii", (char **)kwlist, 
                                     &width, &height, &filename, &components, &precision))
        return -1;

    if (filename) {
        py_obj->texture = texture_load(filename,0);
        if (!py_obj->texture) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to load texture");
            return -1;
        }
    } else {
        if ((width<1) || (width>64*1024) ||
            (height<1) || (height>64*1024)) {
            PyErr_SetString(PyExc_RuntimeError, "Bad height or width for texture");
            return -1;
        }
        py_obj->texture = texture_create(width, height);
        if (!py_obj->texture) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to create texture");
            return -1;
        }
    }
    return 0;
}

/*a Data sharing with other objects
 */
/*f python_texture_from_handle
 */
PyObject *
python_texture_from_handle(int handle)
{
    for (auto p : texture_list) {
        if (p->handle==handle)
        {
            return (PyObject *)p;
        }
    }
    return NULL;
}

/*f python_texture_data
 */
int
python_texture_data(PyObject* self, int id, void *data_ptr)
{
    t_PyObject_texture *py_obj = (t_PyObject_texture *)self;
    if (!PyObject_TypeCheck(self, &PyTypeObject_texture))
        return 0;

    ((t_texture_ptr *)data_ptr)[0] = py_obj->texture;
    return 1;
}

