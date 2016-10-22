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
#include "python_vector.h"
#include "vector.h"
#include "python_quaternion.h"
#include "quaternion.h"

/*a Defines
 */

/*a Types
 */
/*t t_PyObject_vector
 */
typedef struct t_PyObject_vector *t_PyObject_vector_ptr;
typedef struct t_PyObject_vector {
    PyObject_HEAD
    c_vector *vector;
} t_PyObject_vector;

/*a Forward declarations
 */
static PyObject *python_vector_method_copy(PyObject* self);
static PyObject *python_vector_method_modulus(PyObject* self);
static PyObject *python_vector_method_modulus_squared(PyObject* self);
static PyObject *python_vector_method_normalize(PyObject* self);

static PyObject *python_vector_method_scale(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_vector_method_add(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_vector_method_dot_product(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_vector_method_cross_product(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_vector_method_angle_axis_to_v(PyObject* self, PyObject* args, PyObject *kwds);

static PyObject *python_vector_method_binary_add(PyObject* self, PyObject* b);
static PyObject *python_vector_method_binary_subtract(PyObject* self, PyObject* b);

static PyObject *python_vector_method_unary_abs(PyObject* self);
static int       python_vector_method_unary_nonzero(PyObject* self);
static PyObject *python_vector_method_unary_negate(PyObject* self);

static PyObject *python_vector_getattr(PyObject *self, char *attr);
static PyObject *python_vector_str(PyObject *self);
static PyObject *python_vector_repr(PyObject *self);
static void      python_vector_dealloc(PyObject *self);

/*a Static variables
 */

/*v python_vector_methods
 */
static PyMethodDef python_vector_methods[] = {
    {"copy",          (PyCFunction)python_vector_method_copy,              METH_NOARGS},
    {"modulus",       (PyCFunction)python_vector_method_modulus,           METH_NOARGS},
    {"modulus_squared", (PyCFunction)python_vector_method_modulus_squared, METH_NOARGS},
    {"normalize",     (PyCFunction)python_vector_method_normalize,         METH_NOARGS},

    {"scale",         (PyCFunction)python_vector_method_scale,          METH_VARARGS|METH_KEYWORDS},
    {"add",           (PyCFunction)python_vector_method_add,            METH_VARARGS|METH_KEYWORDS},
    {"dot_product",   (PyCFunction)python_vector_method_dot_product,    METH_VARARGS|METH_KEYWORDS},
    {"cross_product", (PyCFunction)python_vector_method_cross_product,  METH_VARARGS|METH_KEYWORDS},//
    {"angle_axis_to_v", (PyCFunction)python_vector_method_angle_axis_to_v,  METH_VARARGS|METH_KEYWORDS},//
    {NULL, NULL},
};

/*f python_vector_new
 */
static PyObject *
python_vector_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    t_PyObject_vector *py_obj;
    py_obj = (t_PyObject_vector *)type->tp_alloc(type, 0);
    if (py_obj) {
        py_obj->vector = NULL;
    }
    return (PyObject *)py_obj;
}

/*f python_vector_init
 */
static int
python_vector_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;

    static const char *kwlist[] = {"vector", "length", NULL};
    PyObject *vector=NULL;
    int length=0;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!i", (char **)kwlist, 
                                     &PyTuple_Type, &vector, &length))
        return -1;
    if (length>0) {
        py_obj->vector = new c_vector(length);
    } else if (vector) {
        PyObject *tuple = PySequence_Fast(vector, "vector must be a tuple or list");
        int len = PySequence_Size(vector);
        py_obj->vector = new c_vector(len);
        for (int i=0; (i<len) && i<VECTOR_MAX_LENGTH; i++) {
            PyObject *item = PySequence_Fast_GET_ITEM(tuple, i);
            ((double *)py_obj->vector->coords())[i] = PyFloat_AsDouble(item);
        }
        Py_DECREF(tuple);
    } else {
        py_obj->vector = NULL;
    }
    return 0;
}

/*v python_vector_number_methods
 */
static PyNumberMethods python_vector_number_methods = {
    python_vector_method_binary_add,
    python_vector_method_binary_subtract,
    0, //python_vector_method_binary_multiply,
    0, //python_vector_method_binary_divide,
    0,               /* binaryfunc nb_remainder;    __mod__ */
    0,               /* binaryfunc nb_divmod;       __divmod__ */
    0,               /* ternaryfunc nb_power;       __pow__ */
    python_vector_method_unary_negate,
    0,               /* unaryfunc nb_positive;      __pos__ */
    python_vector_method_unary_abs,
    python_vector_method_unary_nonzero,
    0, // python_vector_method_unary_invert,
    0,               /* binaryfunc nb_lshift;       __lshift__ */
    0,               /* binaryfunc nb_rshift;       __rshift__ */
    0,               /* binaryfunc nb_and;          __and__ */
    0,               /* binaryfunc nb_xor;          __xor__ */
    0,               /* binaryfunc nb_or;           __or__ */
    0,               /* coercion nb_coerce;         __coerce__ */
    0,               /* unaryfunc nb_int;           __int__ */
    0,               /* unaryfunc nb_long;          __long__ */
    0,               /* unaryfunc nb_float;         __float__ */
    0,               /* unaryfunc nb_oct;           __oct__ */
    0,               /* unaryfunc nb_hex;           __hex__ */
};

/*v PyTypeObject_vector_frame
 */
PyTypeObject PyTypeObject_vector_frame = {
    PyObject_HEAD_INIT(NULL)
    0, // variable size
    "vector", // type name
    sizeof(t_PyObject_vector), // basic size
    0, // item size - zero for static sized object types
    python_vector_dealloc, //py_engine_dealloc, /*tp_dealloc*/
    0, /*tp_print - basically deprecated */
    python_vector_getattr, /*tp_getattr*/
    0, /*tp_setattr*/
    0, /*tp_compare*/
	python_vector_repr,     /* tp_repr */
    &python_vector_number_methods, /*tp_as_number*/
    0, /*tp_as_sequence*/
    0, /*tp_as_mapping*/
    0, /*tp_hash */
	0, /* tp_call - called if the object itself is invoked as a method */
	python_vector_str,     /* tp_str */
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Vector objects",       /* tp_doc */
    0,		                   /* tp_traverse */
    0,		                   /* tp_clear */
    0,		                   /* tp_richcompare */
    0,		                   /* tp_weaklistoffset */
    0,		                   /* tp_iter */
    0,		                   /* tp_iternext */
    python_vector_methods, /* tp_methods */
    0, //python_vector_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    python_vector_init,    /* tp_init */
    0,                         /* tp_alloc */
    python_vector_new,     /* tp_new */
};

/*a Python vector object methods
 */
/*f python_vector_method_copy
 */
static PyObject *
python_vector_method_copy(PyObject* self)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    if (py_obj->vector) {
        return python_vector_from_c(new c_vector(*py_obj->vector));
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_modulus
 */
static PyObject *
python_vector_method_modulus(PyObject* self)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    if (py_obj->vector) {
        return PyFloat_FromDouble(py_obj->vector->modulus());
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_modulus_squared
 */
static PyObject *
python_vector_method_modulus_squared(PyObject* self)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    if (py_obj->vector) {
        return PyFloat_FromDouble(py_obj->vector->modulus_squared());
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_normalize
 */
static PyObject *
python_vector_method_normalize(PyObject* self)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    if (py_obj->vector) {
        py_obj->vector->normalize();
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_scale
 */
static PyObject *
python_vector_method_scale(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    double scale;

    static const char *kwlist[] = {"scale", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "d", (char **)kwlist, 
                                     &scale))
        return NULL;

    if (py_obj->vector) {
        py_obj->vector->scale(scale);
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_binary_add
 */
static PyObject *
python_vector_method_binary_add(PyObject* self, PyObject* b)
{
    t_PyObject_vector *vec_a = (t_PyObject_vector *)self;
    t_PyObject_vector *vec_b = (t_PyObject_vector *)b;

    if (PyObject_TypeCheck(vec_b, &PyTypeObject_vector_frame) &&
        (vec_a->vector && vec_b->vector) ) {
            c_vector *q = new c_vector(vec_a->vector->length());
            *q = (*vec_a->vector) + (*vec_b->vector);
            return python_vector_from_c(q);
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_binary_subtract
 */
static PyObject *
python_vector_method_binary_subtract(PyObject* self, PyObject* b)
{
    t_PyObject_vector *vec_a = (t_PyObject_vector *)self;
    t_PyObject_vector *vec_b = (t_PyObject_vector *)b;

    if (PyObject_TypeCheck(vec_b, &PyTypeObject_vector_frame) &&
        (vec_a->vector && vec_b->vector) ) {
            c_vector *q = new c_vector(vec_a->vector->length());
            *q = (*vec_a->vector) - (*vec_b->vector);
            return python_vector_from_c(q);
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_unary_negate
 */
static PyObject *
python_vector_method_unary_negate(PyObject* self)
{
    t_PyObject_vector *vec_a = (t_PyObject_vector *)self;

    if (vec_a->vector) {
        c_vector *q = new c_vector(vec_a->vector->length());
        *q = (*vec_a->vector);
        *q = -*q;
        return python_vector_from_c(q);
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_unary_nonzero
 */
static int
python_vector_method_unary_nonzero(PyObject* self)
{
    t_PyObject_vector *vec_a = (t_PyObject_vector *)self;

    if (vec_a->vector) {
        const double *coords = vec_a->vector->coords();
        for (int i=0; i<vec_a->vector->length(); i++) {
            if (coords[i]!=0) return 1;
        }
        return 0;
    }
    return 0;
}

/*f python_vector_method_unary_abs
 */
static PyObject *
python_vector_method_unary_abs(PyObject* self)
{
    t_PyObject_vector *vec_a = (t_PyObject_vector *)self;

    if (vec_a->vector) {
        return PyFloat_FromDouble(vec_a->vector->modulus());
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_add
 */
static PyObject *
python_vector_method_add(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;

    t_PyObject_vector *vec_b;
    double scale=1.0;
    static const char *kwlist[] = {"other", "scale", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|d", (char **)kwlist, &vec_b, &scale))
        return NULL;

    if (!PyObject_TypeCheck((PyObject *)vec_b, &PyTypeObject_vector_frame))
        return NULL;
    if (py_obj->vector && vec_b->vector) {
        py_obj->vector->add_scaled(vec_b->vector, scale);
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_dot_product
 */
static PyObject *
python_vector_method_dot_product(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;

    t_PyObject_vector *vec_b;
    static const char *kwlist[] = {"other", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, &vec_b))
        return NULL;

    if (!PyObject_TypeCheck((PyObject *)vec_b, &PyTypeObject_vector_frame))
        return NULL;
    if (py_obj->vector && vec_b->vector) {
        return Py_BuildValue("d",py_obj->vector->dot_product(*vec_b->vector));
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_cross_product
 */
static PyObject *
python_vector_method_cross_product(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    t_PyObject_vector *vec_b;
    static const char *kwlist[] = {"other", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, &vec_b))
        return NULL;

    if (!PyObject_TypeCheck((PyObject *)vec_b, &PyTypeObject_vector_frame))
        return NULL;
    if (py_obj->vector && vec_b->vector) {
        c_vector *v;
        v = new c_vector(py_obj->vector->cross_product(*vec_b->vector));
        return python_vector_from_c(v);
    }
    Py_RETURN_NONE;
}

/*f python_vector_method_angle_axis_to_v
 */
static PyObject *
python_vector_method_angle_axis_to_v(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    t_PyObject_vector *vec_b;
    static const char *kwlist[] = {"other", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", (char **)kwlist, &vec_b))
        return NULL;

    if (!PyObject_TypeCheck((PyObject *)vec_b, &PyTypeObject_vector_frame))
        return NULL;
    if (py_obj->vector && vec_b->vector) {
        double c, s;
        c_quaternion *q;
        c_vector v=py_obj->vector->angle_axis_to_v(*(vec_b->vector), &c, &s);
        q = new c_quaternion();
        q->from_rotation(c,s,v.coords());
        return python_quaternion_from_c(q);
    }
    Py_RETURN_NONE;
}

/*a Python vector infrastructure mthods
 */
/*f python_vector_repr
 */
static PyObject *
python_vector_repr(PyObject* self)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    if (py_obj->vector) {
        char buffer[1024];
        sprintf(buffer,"vector((");
        for (int i=0; i<py_obj->vector->length(); i++) {
            sprintf(buffer+strlen(buffer),"%lf,",py_obj->vector->coords()[i]);
        }
        sprintf(buffer+strlen(buffer),"))");
        return PyString_FromFormat("%s", buffer);
    }
    Py_RETURN_NONE;
}

/*f python_vector_str
 */
static PyObject *
python_vector_str(PyObject* self)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    if (py_obj->vector) {
        char buffer[1024];
        py_obj->vector->__str__(buffer, sizeof(buffer));
        return PyString_FromFormat("%s", buffer);
    }
    Py_RETURN_NONE;
}

/*f python_vector_dealloc
 */
static void
python_vector_dealloc(PyObject *self)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    if (py_obj->vector) {
        delete(py_obj->vector);
        py_obj->vector = NULL;
    }
}

/*f python_vector_getattr
 */
static PyObject *
python_vector_getattr(PyObject *self, char *attr)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    
    if (py_obj->vector) {
        if (!strcmp(attr, "coords")) {
            const double *coords = py_obj->vector->coords();
            return Py_BuildValue(((const char *)"dddddddd")+(8-py_obj->vector->length()),
                                 coords[0], coords[1], coords[2], coords[3],
                                 coords[4], coords[5], coords[6], coords[7]);
        }
    }
    return Py_FindMethod(python_vector_methods, self, attr);
}

/*a Python object
 */
/*f python_vector_from_c
 */
PyObject *
python_vector_from_c(c_vector *vector)
{
    t_PyObject_vector *py_obj;
    PyObject *obj = PyObject_CallObject((PyObject *) &PyTypeObject_vector_frame, NULL);
    if (!obj) {
        return Py_None;
    }
    py_obj = (t_PyObject_vector *)obj;
    py_obj->vector = vector;

    return obj;
}

/*f python_vector_init_premodule
 */
int python_vector_init_premodule(void)
{
    if (PyType_Ready(&PyTypeObject_vector_frame) < 0)
        return -1;
    return 0;
}

/*f python_vector_init_postmodule
 */
void python_vector_init_postmodule(PyObject *module)
{
    Py_INCREF(&PyTypeObject_vector_frame);
    PyModule_AddObject(module, "vector", (PyObject *)&PyTypeObject_vector_frame);
}

/*a Data sharing with other objects
 */
/*f python_vector_data
 */
extern int python_vector_data(PyObject* self, int id, void *data_ptr)
{
    t_PyObject_vector *py_obj = (t_PyObject_vector *)self;
    if (!PyObject_TypeCheck(self, &PyTypeObject_vector_frame))
        return 0;

    ((c_vector **)data_ptr)[0] = py_obj->vector;
    return 1;
}

