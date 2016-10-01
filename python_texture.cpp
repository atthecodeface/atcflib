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
#include "texture.h"

/*a Defines
 */

/*a Types
 */
/*t t_PyObject_texture
 */
typedef struct {
     PyObject_HEAD
     t_texture_ptr texture;
} t_PyObject_texture;

static PyObject *
python_texture_method_save(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_texture *py_obj = (t_PyObject_texture *)self;
    int components, conversion;
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

PyMethodDef python_texture_methods[] = {
    {"save", (PyCFunction)python_texture_method_save, METH_VARARGS},
    {NULL, NULL},
};

static void
python_texture_dealloc(PyObject *self)
{
    t_PyObject_texture *py_obj = (t_PyObject_texture *)self;
    if (py_obj->texture) {
        texture_destroy(py_obj->texture);
        py_obj->texture = NULL;
    }
}

static PyObject *
python_texture_getattr(PyObject *self, char *attr)
{
    t_PyObject_texture *py_obj = (t_PyObject_texture *)self;
    
    if (py_obj->texture) {
        if (!strcmp(attr, "width")) {
            return PyInt_FromLong(texture_header(py_obj->texture)->width);
        }
        if (!strcmp(attr, "height")) {
            return PyInt_FromLong(texture_header(py_obj->texture)->height);
        }
    }
    return Py_FindMethod(python_texture_methods, self, attr);
}

/*v PyTypeObject_texture_frame
 */
static PyTypeObject PyTypeObject_texture_frame = {
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
};

/*f python_texture
 */
PyObject *
python_texture(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_texture *py_obj;

    PyTypeObject_texture_frame.ob_type = &PyType_Type;

    int width, height, components, precision;
    const char *filename = NULL;

    static const char *kwlist[] = {"width", "height", "filename", "components", "precision", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iisii", (char **)kwlist, 
                                     &width, &height, &filename, &components, &precision))
        return NULL;

    py_obj = PyObject_New(t_PyObject_texture, &PyTypeObject_texture_frame);
    py_obj->texture = NULL;
    if (filename) {
        py_obj->texture = texture_load(filename,0);
    } else {
        py_obj->texture = texture_create(32,32);
    }
    return (PyObject *)py_obj;
}

