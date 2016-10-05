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
#include "python_quaternion.h"
#include "quaternion.h"

/*a Defines
 */

/*a Types
 */
/*t t_PyObject_quaternion
 */
typedef struct t_PyObject_quaternion *t_PyObject_quaternion_ptr;
typedef struct t_PyObject_quaternion {
    PyObject_HEAD
    c_quaternion *quaternion;
} t_PyObject_quaternion;

/*a Forward declarations
 */
static PyObject *python_quaternion_getattr(PyObject *self, char *attr);
static PyObject *python_quaternion_method_scale(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_lookat(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_from_euler(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_from_rotation(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_add(PyObject* self, PyObject* b);
static PyObject *python_quaternion_method_subtract(PyObject* self, PyObject* b);
static PyObject *python_quaternion_method_multiply(PyObject* self, PyObject* b);
static PyObject *python_quaternion_method_divide(PyObject* self, PyObject* b);
static PyObject *python_quaternion_method_abs(PyObject* self);
static PyObject *python_quaternion_method_conjugate(PyObject* self);
static PyObject *python_quaternion_str(PyObject *self);
static void      python_quaternion_dealloc(PyObject *self);

/*a Static variables
 */

/*v python_quaternion_methods
 */
static PyMethodDef python_quaternion_methods[] = {
    {"scale",      (PyCFunction)python_quaternion_method_scale,      METH_VARARGS|METH_KEYWORDS},
    {"lookat",     (PyCFunction)python_quaternion_method_lookat,     METH_VARARGS|METH_KEYWORDS},
    {"from_euler", (PyCFunction)python_quaternion_method_from_euler, METH_VARARGS|METH_KEYWORDS},
    {"from_rotation", (PyCFunction)python_quaternion_method_from_rotation, METH_VARARGS|METH_KEYWORDS},
    {NULL, NULL},
};

/*v PyTypeObject_quaternion_frame
 */
static PyNumberMethods python_quaternion_number_methods = {
    python_quaternion_method_add,
    python_quaternion_method_subtract,
    python_quaternion_method_multiply,
    python_quaternion_method_divide,
    0,               /* binaryfunc nb_remainder;    __mod__ */
    0,               /* binaryfunc nb_divmod;       __divmod__ */
    0,               /* ternaryfunc nb_power;       __pow__ */
    0,               /* unaryfunc nb_negative;      __neg__ */
    0,               /* unaryfunc nb_positive;      __pos__ */
    python_quaternion_method_abs,
    0,               /* inquiry nb_nonzero;         __nonzero__ */
    python_quaternion_method_conjugate,    /* unaryfunc nb_invert;        ~ __invert__ */
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

static PyTypeObject PyTypeObject_quaternion_frame = {
    PyObject_HEAD_INIT(NULL)
    0, // variable size
    "quaternion", // type name
    sizeof(t_PyObject_quaternion), // basic size
    0, // item size - zero for static sized object types
    python_quaternion_dealloc, //py_engine_dealloc, /*tp_dealloc*/
    0, /*tp_print - basically deprecated */
    python_quaternion_getattr, /*tp_getattr*/
    0, /*tp_setattr*/
    0, /*tp_compare*/
    0, /*tp_repr - ideally a represenation that is python that recreates this object */
    &python_quaternion_number_methods, /*tp_as_number*/
    0, /*tp_as_sequence*/
    0, /*tp_as_mapping*/
    0, /*tp_hash */
	0, /* tp_call - called if the object itself is invoked as a method */
	python_quaternion_str, /* tp_str */
};

/*a Python quaternion methods
 */
/*f python_quaternion_method_scale
 */
static PyObject *
python_quaternion_method_scale(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    double scale;

    static const char *kwlist[] = {"scale", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "d", (char **)kwlist, 
                                     &scale))
        return NULL;

    if (py_obj->quaternion) {
        py_obj->quaternion->scale(scale);
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_add
 */
static PyObject *
python_quaternion_method_add(PyObject* self, PyObject* b)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;
    t_PyObject_quaternion *quat_b = (t_PyObject_quaternion *)b;

    if (PyObject_TypeCheck(quat_b, &PyTypeObject_quaternion_frame) &&
        (quat_a->quaternion && quat_b->quaternion) ) {
            c_quaternion *q = new(c_quaternion);
            *q = (*quat_a->quaternion) + (*quat_b->quaternion);
            return python_quaternion_from_c(q);
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_subtract
 */
static PyObject *
python_quaternion_method_subtract(PyObject* self, PyObject* b)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;
    t_PyObject_quaternion *quat_b = (t_PyObject_quaternion *)b;

    if (PyObject_TypeCheck(quat_b, &PyTypeObject_quaternion_frame) &&
        (quat_a->quaternion && quat_b->quaternion) ) {
            c_quaternion *q = new(c_quaternion);
            *q = (*quat_a->quaternion) - (*quat_b->quaternion);
            return python_quaternion_from_c(q);
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_multiply
 */
static PyObject *
python_quaternion_method_multiply(PyObject* self, PyObject* b)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;
    t_PyObject_quaternion *quat_b = (t_PyObject_quaternion *)b;

    if (PyObject_TypeCheck(quat_b, &PyTypeObject_quaternion_frame) &&
        (quat_a->quaternion && quat_b->quaternion) ) {
            c_quaternion *q = new(c_quaternion);
            *q = (*quat_a->quaternion) * (*quat_b->quaternion);
            return python_quaternion_from_c(q);
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_divide
 */
static PyObject *
python_quaternion_method_divide(PyObject* self, PyObject* b)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;
    t_PyObject_quaternion *quat_b = (t_PyObject_quaternion *)b;

    if (PyObject_TypeCheck(quat_b, &PyTypeObject_quaternion_frame) &&
        (quat_a->quaternion && quat_b->quaternion) ) {
            c_quaternion *q = new(c_quaternion);
            *q = (*quat_a->quaternion) / (*quat_b->quaternion);
            return python_quaternion_from_c(q);
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_abs
 */
static PyObject *
python_quaternion_method_abs(PyObject* self)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;

    if (quat_a->quaternion) {
        c_quaternion *q = new(c_quaternion);
        *q = (*quat_a->quaternion);
        return python_quaternion_from_c(q->normalize());
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_conjugate
 */
static PyObject *
python_quaternion_method_conjugate(PyObject* self)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;

    if (quat_a->quaternion) {
        c_quaternion *q = new(c_quaternion);
        *q = (*quat_a->quaternion);
        return python_quaternion_from_c(q->conjugate());
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_lookat
 */
static PyObject *
python_quaternion_method_lookat(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;

    PyObject *xyz, *up;
    double xyz_d[3], up_d[3];

    static const char *kwlist[] = {"xyz", "up", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", (char **)kwlist, 
                                     &xyz, &up))
        return NULL;

    if (!PyArg_ParseTuple(xyz, "ddd", &xyz_d[0], &xyz_d[1], &xyz_d[2])) return NULL;
    if (!PyArg_ParseTuple(up,  "ddd", &up_d[0],  &up_d[1],  &up_d[2] )) return NULL;

    if (py_obj->quaternion) {
        py_obj->quaternion->lookat(xyz_d, up_d);
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_from_euler
 */
static PyObject *
python_quaternion_method_from_euler(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;

    double roll=0, pitch=0, yaw=0;
    int degrees=0;

    static const char *kwlist[] = {"roll", "pitch", "yaw", "degrees", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|dddi", (char **)kwlist, 
                                     &roll, &pitch, &yaw, &degrees))
        return NULL;

    if (py_obj->quaternion) {
        py_obj->quaternion->from_euler(roll, pitch, yaw, degrees);
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_from_rotation
 */
static PyObject *
python_quaternion_method_from_rotation(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;

    PyObject *axis;
    double angle=0;
    int degrees=0;
    double xyz_d[3];

    static const char *kwlist[] = {"axis", "angle", "degrees", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "Od|i", (char **)kwlist, 
                                     &axis, &angle, &degrees))
        return NULL;

    if (!PyArg_ParseTuple(axis, "ddd", &xyz_d[0], &xyz_d[1], &xyz_d[2])) return NULL;
    if (py_obj->quaternion) {
        py_obj->quaternion->from_rotation(angle, xyz_d, degrees);
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_str
 */
static PyObject *
python_quaternion_str(PyObject* self)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;
    if (quat_a->quaternion) {
        char buffer[1024];
        quat_a->quaternion->__str__(buffer, sizeof(buffer));
        return PyString_FromFormat("%s", buffer);
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_dealloc
 */
static void
python_quaternion_dealloc(PyObject *self)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    if (py_obj->quaternion) {
        delete(py_obj->quaternion);
        py_obj->quaternion = NULL;
    }
}

/*f python_quaternion_getattr
 */
static PyObject *
python_quaternion_getattr(PyObject *self, char *attr)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    
    if (py_obj->quaternion) {
        if (!strcmp(attr, "r")) {
            return PyFloat_FromDouble(py_obj->quaternion->r());
        }
        if (!strcmp(attr, "i")) {
            return PyFloat_FromDouble(py_obj->quaternion->i());
        }
        if (!strcmp(attr, "j")) {
            return PyFloat_FromDouble(py_obj->quaternion->j());
        }
        if (!strcmp(attr, "k")) {
            return PyFloat_FromDouble(py_obj->quaternion->k());
        }
        if (!strcmp(attr, "rijk")) {
            return Py_BuildValue("dddd",
                                 py_obj->quaternion->r(),
                                 py_obj->quaternion->i(),
                                 py_obj->quaternion->j(),
                                 py_obj->quaternion->k());
        }
    }
    return Py_FindMethod(python_quaternion_methods, self, attr);
}

/*a Python object
 */
/*f python_quaternion_from_c
 */
PyObject *
python_quaternion_from_c(c_quaternion *quaternion)
{
    t_PyObject_quaternion *py_obj;

    PyTypeObject_quaternion_frame.ob_type = &PyType_Type;

    py_obj = PyObject_New(t_PyObject_quaternion, &PyTypeObject_quaternion_frame);
    py_obj->quaternion = quaternion;

    return (PyObject *)py_obj;
}


/*f python_quaternion
 */
PyObject *
python_quaternion(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj;

    PyTypeObject_quaternion_frame.ob_type = &PyType_Type;

    static const char *kwlist[] = {"r", "i", "j", "k", NULL};
    double r=0.0, i=0.0, j=0.0, k=0.0;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|dddd", (char **)kwlist, 
                                     &r, &i, &j, &k))
        return NULL;
    py_obj = PyObject_New(t_PyObject_quaternion, &PyTypeObject_quaternion_frame);
    py_obj->quaternion = new c_quaternion(r,i,j,k);

    return (PyObject *)py_obj;
}

/*a Data sharing with other objects
 */
/*f python_quaternion_data
 */
extern int python_quaternion_data(PyObject* self, int id, void *data_ptr)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    if (!PyObject_TypeCheck(self, &PyTypeObject_quaternion_frame))
        return 0;

    ((c_quaternion **)data_ptr)[0] = py_obj->quaternion;
    return 1;
}

/*f python_quaternion_init
 */
void python_quaternion_init(void)
{
}
