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
#include "python_vector.h"
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
static PyObject *python_quaternion_class_method_of_euler(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_class_method_of_rotation(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_class_method_of_sequence(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_class_method_identity(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_class_method_pitch(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_class_method_yaw(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_class_method_roll(PyObject* self, PyObject* args, PyObject *kwds);

static PyObject *python_quaternion_method_copy(PyObject* self);
static PyObject *python_quaternion_method_conjugate(PyObject* self);
static PyObject *python_quaternion_method_reciprocal(PyObject* self);
static PyObject *python_quaternion_method_modulus(PyObject* self);
static PyObject *python_quaternion_method_modulus_squared(PyObject* self);
static PyObject *python_quaternion_method_normalize(PyObject* self);
static PyObject *python_quaternion_method_get(PyObject* self);

static PyObject *python_quaternion_method_scale(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_lookat(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_add(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_multiply(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_rotate_vector(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_axis_angle(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_from_euler(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_to_euler(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_from_rotation(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_to_rotation(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_to_rotation_str(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_method_from_sequence(PyObject* self, PyObject* args, PyObject *kwds);

static PyObject *python_quaternion_method_binary_add(PyObject* self, PyObject* b);
static PyObject *python_quaternion_method_binary_subtract(PyObject* self, PyObject* b);
static PyObject *python_quaternion_method_binary_multiply(PyObject* self, PyObject* b);
static PyObject *python_quaternion_method_binary_divide(PyObject* self, PyObject* b);

static PyObject *python_quaternion_method_unary_abs(PyObject* self);
static int       python_quaternion_method_unary_nonzero(PyObject* self);
static PyObject *python_quaternion_method_unary_invert(PyObject* self);
static PyObject *python_quaternion_method_unary_negate(PyObject* self);

static PyObject *python_quaternion_getattr(PyObject *self, char *attr);
static PyObject *python_quaternion_str(PyObject *self);
static void      python_quaternion_dealloc(PyObject *self);

/*a Static variables
 */

/*v python_quaternion_methods
 */
static PyMethodDef python_quaternion_methods[] = {
    {"roll",          (PyCFunction)python_quaternion_class_method_roll,           METH_CLASS|METH_VARARGS|METH_KEYWORDS},
    {"pitch",         (PyCFunction)python_quaternion_class_method_pitch,          METH_CLASS|METH_VARARGS|METH_KEYWORDS},
    {"yaw",           (PyCFunction)python_quaternion_class_method_yaw,            METH_CLASS|METH_VARARGS|METH_KEYWORDS},
    {"identity",      (PyCFunction)python_quaternion_class_method_identity,       METH_CLASS|METH_VARARGS|METH_KEYWORDS},
    {"of_euler",      (PyCFunction)python_quaternion_class_method_of_euler,       METH_CLASS|METH_VARARGS|METH_KEYWORDS},
    {"of_rotation",   (PyCFunction)python_quaternion_class_method_of_rotation,    METH_CLASS|METH_VARARGS|METH_KEYWORDS},
    {"of_sequence",   (PyCFunction)python_quaternion_class_method_of_sequence,    METH_CLASS|METH_VARARGS|METH_KEYWORDS},

    {"copy",          (PyCFunction)python_quaternion_method_copy,              METH_NOARGS},
    {"conjugate",     (PyCFunction)python_quaternion_method_conjugate,         METH_NOARGS},
    {"reciprocal",    (PyCFunction)python_quaternion_method_reciprocal,        METH_NOARGS},
    {"modulus",       (PyCFunction)python_quaternion_method_modulus,           METH_NOARGS},
    {"modulus_squared", (PyCFunction)python_quaternion_method_modulus_squared, METH_NOARGS},
    {"normalize",     (PyCFunction)python_quaternion_method_normalize,         METH_NOARGS},
    {"get",           (PyCFunction)python_quaternion_method_get,               METH_NOARGS},
    {"get_matrix",    (PyCFunction)python_quaternion_method_scale,             METH_NOARGS},//
    {"get_matrix_as_lists",(PyCFunction)python_quaternion_method_scale,        METH_NOARGS},//

    {"scale",         (PyCFunction)python_quaternion_method_scale,          METH_VARARGS|METH_KEYWORDS},
    {"lookat",        (PyCFunction)python_quaternion_method_lookat,         METH_VARARGS|METH_KEYWORDS},
    {"from_sequence", (PyCFunction)python_quaternion_method_from_sequence,  METH_VARARGS|METH_KEYWORDS},
    {"from_euler",    (PyCFunction)python_quaternion_method_from_euler,     METH_VARARGS|METH_KEYWORDS},
    {"from_rotation", (PyCFunction)python_quaternion_method_from_rotation,  METH_VARARGS|METH_KEYWORDS},
    {"to_euler",      (PyCFunction)python_quaternion_method_to_euler,       METH_VARARGS|METH_KEYWORDS},
    {"to_rotation",   (PyCFunction)python_quaternion_method_to_rotation,    METH_VARARGS|METH_KEYWORDS},
    {"to_rotation_str",    (PyCFunction)python_quaternion_method_to_rotation_str,    METH_VARARGS|METH_KEYWORDS},
    {"add",           (PyCFunction)python_quaternion_method_add,            METH_VARARGS|METH_KEYWORDS},
    {"multiply",      (PyCFunction)python_quaternion_method_multiply,       METH_VARARGS|METH_KEYWORDS},
    {"rotate_vector", (PyCFunction)python_quaternion_method_rotate_vector,  METH_VARARGS|METH_KEYWORDS},
    {"axis_angle",    (PyCFunction)python_quaternion_method_axis_angle,     METH_VARARGS|METH_KEYWORDS},
    {"interpolate",   (PyCFunction)python_quaternion_method_scale,      METH_VARARGS|METH_KEYWORDS},//
    {NULL, NULL},
};

/*f python_quaternion_new
 */
static PyObject *
python_quaternion_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj;
    py_obj = (t_PyObject_quaternion *)type->tp_alloc(type, 0);
    if (py_obj) {
        py_obj->quaternion = NULL;
    }
    return (PyObject *)py_obj;
}

/*f python_quaternion_init
 */
static int
python_quaternion_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;

    static const char *kwlist[] = {"quat", "euler", "degrees", "r", "i", "j", "k", NULL};
    PyObject *quat=NULL, *euler=NULL;
    int degrees;
    double r=0.0, i=0.0, j=0.0, k=0.0;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!Oidddd", (char **)kwlist, 
                                     &PyDict_Type, &quat, &euler, &degrees,
                                     &r, &i, &j, &k))
        return -1;
    if (quat) {
        PyObject *obj;
        if ( (obj=PyDict_GetItemString(quat,"r"))!=NULL ) r = PyInt_AsLong(obj);
        if ( (obj=PyDict_GetItemString(quat,"i"))!=NULL ) i = PyInt_AsLong(obj);
        if ( (obj=PyDict_GetItemString(quat,"j"))!=NULL ) j = PyInt_AsLong(obj);
        if ( (obj=PyDict_GetItemString(quat,"k"))!=NULL ) k = PyInt_AsLong(obj);
        if (PyErr_Occurred()) return -1;
    }
    py_obj->quaternion = new c_quaternion(r,i,j,k);
    if (euler) {
        double rpy[3];
        if (!PyArg_ParseTuple(euler, "ddd", &rpy[0], &rpy[1], &rpy[2])) return -1;
        py_obj->quaternion->from_euler(rpy[0], rpy[1], rpy[2], degrees);
    }

    return 0;
}

/*v python_quaternion_number_methods
 */
static PyNumberMethods python_quaternion_number_methods = {
    python_quaternion_method_binary_add,
    python_quaternion_method_binary_subtract,
    python_quaternion_method_binary_multiply,
    python_quaternion_method_binary_divide,
    0,               /* binaryfunc nb_remainder;    __mod__ */
    0,               /* binaryfunc nb_divmod;       __divmod__ */
    0,               /* ternaryfunc nb_power;       __pow__ */
    python_quaternion_method_unary_negate,
    0,               /* unaryfunc nb_positive;      __pos__ */
    python_quaternion_method_unary_abs,
    python_quaternion_method_unary_nonzero,
    python_quaternion_method_unary_invert,
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

/*v PyTypeObject_quaternion_frame
 */
PyTypeObject PyTypeObject_quaternion_frame = {
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
	python_quaternion_str,     /* tp_str */
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Quaternion objects",       /* tp_doc */
    0,		                   /* tp_traverse */
    0,		                   /* tp_clear */
    0,		                   /* tp_richcompare */
    0,		                   /* tp_weaklistoffset */
    0,		                   /* tp_iter */
    0,		                   /* tp_iternext */
    python_quaternion_methods, /* tp_methods */
    0, //python_quaternion_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    python_quaternion_init,    /* tp_init */
    0,                         /* tp_alloc */
    python_quaternion_new,     /* tp_new */
};

/*a Python quaternion class methods
 */
/*f python_quaternion_class_method_of_euler
 */
static PyObject *
python_quaternion_class_method_of_euler(PyObject* cls, PyObject* args, PyObject *kwds)
{
    double roll, pitch, yaw;
    int degrees=0;
    c_quaternion *q;
    static const char *kwlist[] = {"roll", "pitch", "yaw", "degrees", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|dddi", (char **)kwlist, 
                                     &roll, &pitch, &yaw, &degrees))
        return NULL;
    q = new c_quaternion();
    q->from_euler(roll,pitch,yaw,degrees);
    return python_quaternion_from_c(q);
}

/*f python_quaternion_class_method_of_rotation
 */
static PyObject *
python_quaternion_class_method_of_rotation(PyObject* cls, PyObject* args, PyObject *kwds)
{
    PyObject *obj = PyObject_CallObject((PyObject *) &PyTypeObject_quaternion_frame, NULL);
    if (!obj) { return Py_None; }
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)obj;
    py_obj->quaternion = new c_quaternion();
    if (!python_quaternion_method_from_rotation(obj, args, kwds)) {
        Py_DECREF(py_obj); // THIS SHOULD PROBABLY ALWAYS BE DONE??
        // Does an OF_ROTATION have one too many refcount?
        return NULL;
    }
    return obj;
}

/*f python_quaternion_class_method_of_sequence
 */
static PyObject *
python_quaternion_class_method_of_sequence(PyObject* cls, PyObject* args, PyObject *kwds)
{
    PyObject *obj = PyObject_CallObject((PyObject *) &PyTypeObject_quaternion_frame, NULL);
    if (!obj) { return Py_None; }
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)obj;
    py_obj->quaternion = new c_quaternion();
    if (!python_quaternion_method_from_sequence(obj, args, kwds)) {
        Py_DECREF(py_obj);
        return NULL;
    }
    return obj;
}

/*f python_quaternion_class_method_roll
 */
static PyObject *
python_quaternion_class_method_roll(PyObject* cls, PyObject* args, PyObject *kwds)
{
    double angle;
    int degrees=0;
    c_quaternion *q;
    static const char *kwlist[] = {"angle", "degrees", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "d|i", (char **)kwlist, 
                                     &angle, &degrees))
        return NULL;
    q = new c_quaternion();
    q->from_euler(angle,0,0,degrees);
    return python_quaternion_from_c(q);
}

/*f python_quaternion_class_method_yaw
 */
static PyObject *
python_quaternion_class_method_yaw(PyObject* cls, PyObject* args, PyObject *kwds)
{
    double angle;
    int degrees=0;
    c_quaternion *q;
    static const char *kwlist[] = {"angle", "degrees", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "d|i", (char **)kwlist, 
                                     &angle, &degrees))
        return NULL;
    q = new c_quaternion();
    q->from_euler(0,0,angle,degrees);
    return python_quaternion_from_c(q);
}

/*f python_quaternion_class_method_pitch
 */
static PyObject *
python_quaternion_class_method_pitch(PyObject* cls, PyObject* args, PyObject *kwds)
{
    double angle;
    int degrees=0;
    c_quaternion *q;
    static const char *kwlist[] = {"angle", "degrees", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "d|i", (char **)kwlist, 
                                     &angle, &degrees))
        return NULL;
    q = new c_quaternion();
    q->from_euler(0,angle,0,degrees);
    return python_quaternion_from_c(q);
}

/*f python_quaternion_class_method_identity
 */
static PyObject *
python_quaternion_class_method_identity(PyObject* cls, PyObject* args, PyObject *kwds)
{
    c_quaternion *q;
    static const char *kwlist[] = {NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "", (char **)kwlist))
        return NULL;
    q = new c_quaternion(1.0,0.0,0.0,0.0);
    return python_quaternion_from_c(q);
}

/*a Python quaternion object methods
 */
/*f python_quaternion_method_copy
 */
static PyObject *
python_quaternion_method_copy(PyObject* self)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    if (py_obj->quaternion) {
        c_quaternion *q = new c_quaternion(py_obj->quaternion);
        return python_quaternion_from_c(q);
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_conjugate
 */
static PyObject *
python_quaternion_method_conjugate(PyObject* self)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    if (py_obj->quaternion) {
        py_obj->quaternion->conjugate();
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_reciprocal
 */
static PyObject *
python_quaternion_method_reciprocal(PyObject* self)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    if (py_obj->quaternion) {
        py_obj->quaternion->reciprocal();
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_modulus
 */
static PyObject *
python_quaternion_method_modulus(PyObject* self)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    if (py_obj->quaternion) {
        return PyFloat_FromDouble(py_obj->quaternion->modulus());
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_modulus_squared
 */
static PyObject *
python_quaternion_method_modulus_squared(PyObject* self)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    if (py_obj->quaternion) {
        return PyFloat_FromDouble(py_obj->quaternion->modulus_squared());
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_normalize
 */
static PyObject *
python_quaternion_method_normalize(PyObject* self)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    if (py_obj->quaternion) {
        py_obj->quaternion->normalize();
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_get
 */
static PyObject *
python_quaternion_method_get(PyObject* self)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    if (py_obj->quaternion) {
        return Py_BuildValue("dddd",
                             py_obj->quaternion->r(),
                             py_obj->quaternion->i(),
                             py_obj->quaternion->j(),
                             py_obj->quaternion->k());
    }
    Py_RETURN_NONE;
}

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

/*f python_quaternion_method_binary_add
 */
static PyObject *
python_quaternion_method_binary_add(PyObject* self, PyObject* b)
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

/*f python_quaternion_method_binary_subtract
 */
static PyObject *
python_quaternion_method_binary_subtract(PyObject* self, PyObject* b)
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

/*f python_quaternion_method_binary_multiply
 */
static PyObject *
python_quaternion_method_binary_multiply(PyObject* self, PyObject* b)
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

/*f python_quaternion_method_binary_divide
 */
static PyObject *
python_quaternion_method_binary_divide(PyObject* self, PyObject* b)
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

/*f python_quaternion_method_unary_invert
 */
static PyObject *
python_quaternion_method_unary_invert(PyObject* self)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;

    if (quat_a->quaternion) {
        c_quaternion *q = new(c_quaternion);
        *q = (*quat_a->quaternion);
        return python_quaternion_from_c(q->conjugate());
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_unary_negate
 */
static PyObject *
python_quaternion_method_unary_negate(PyObject* self)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;

    if (quat_a->quaternion) {
        c_quaternion *q = new(c_quaternion);
        *q = (*quat_a->quaternion);
        *q = -*q;
        return python_quaternion_from_c(q);
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_unary_nonzero
 */
static int
python_quaternion_method_unary_nonzero(PyObject* self)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;

    if (quat_a->quaternion) {
        if (quat_a->quaternion->r()!=0) return 1;
        if (quat_a->quaternion->i()!=0) return 1;
        if (quat_a->quaternion->j()!=0) return 1;
        if (quat_a->quaternion->k()!=0) return 1;
        return 0;
    }
    return 0;
}

/*f python_quaternion_method_unary_abs
 */
static PyObject *
python_quaternion_method_unary_abs(PyObject* self)
{
    t_PyObject_quaternion *quat_a = (t_PyObject_quaternion *)self;

    if (quat_a->quaternion) {
        return PyFloat_FromDouble(quat_a->quaternion->modulus());
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_add
 */
static PyObject *
python_quaternion_method_add(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;

    t_PyObject_quaternion *quat_b;
    double scale=1.0;
    static const char *kwlist[] = {"other", "scale", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|d", (char **)kwlist, &quat_b, &scale))
        return NULL;

    if (!PyObject_TypeCheck((PyObject *)quat_b, &PyTypeObject_quaternion_frame))
        return NULL;
    if (py_obj->quaternion && quat_b->quaternion) {
        py_obj->quaternion->add_scaled(quat_b->quaternion, scale);
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_multiply
 */
static PyObject *
python_quaternion_method_multiply(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;

    t_PyObject_quaternion *quat_b;
    int premultiply=0;
    static const char *kwlist[] = {"other", "premultiply", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|i", (char **)kwlist, &quat_b, &premultiply))
        return NULL;

    if (!PyObject_TypeCheck((PyObject *)quat_b, &PyTypeObject_quaternion_frame))
        return NULL;
    if (py_obj->quaternion && quat_b->quaternion) {
        py_obj->quaternion->multiply(quat_b->quaternion,premultiply);
        Py_INCREF(py_obj);
        return self;
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_rotate_vector
 */
static PyObject *
python_quaternion_method_rotate_vector(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    PyObject *vec_obj;
    static const char *kwlist[] = {"vector", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", (char **)kwlist,
                                     &PyTypeObject_vector_frame, &vec_obj))
        return NULL;

    if (py_obj->quaternion) {
        c_vector *vector;
        if (python_vector_data(vec_obj, 0, &vector)) {
            c_vector *v;
            v = new c_vector(*(py_obj->quaternion) * c_quaternion(*vector) * *(py_obj->quaternion->copy()->conjugate()));
            return python_vector_from_c(v);
        }
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_axis_angle
 */
static PyObject *
python_quaternion_method_axis_angle(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    t_PyObject_quaternion *other;
    PyObject *vec_obj;
    static const char *kwlist[] = {"other", "vector", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!O!", (char **)kwlist,
                                     &PyTypeObject_quaternion_frame, &other,
                                     &PyTypeObject_vector_frame, &vec_obj))
        return NULL;

    if (py_obj->quaternion && other->quaternion) {
        c_vector *vector;
        if (python_vector_data(vec_obj, 0, &vector)) {
            c_quaternion *v;
            v = new c_quaternion(py_obj->quaternion->axis_angle(*other->quaternion, *vector));
            return python_quaternion_from_c(v);
        }
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

/*f python_quaternion_method_from_sequence
 */
static PyObject *
python_quaternion_method_from_sequence(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    PyObject *rotations;
    int degrees=0;
    static const char *kwlist[] = {"rotations", "degrees", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|d", (char **)kwlist, 
                                     &rotations, &degrees))
        return NULL;

    if (py_obj->quaternion) {
        (*py_obj->quaternion) = 1.0;
        PyObject *seq = PySequence_Fast(rotations, "Rotations must be a sequence of (type, angle)'s");
        int len = PySequence_Size(seq);
        for (int i=0; i<len; i++) {
            PyObject *item = PySequence_Fast_GET_ITEM(seq, i);
            double angle;
            const char *rot_type;
            if (PyArg_ParseTuple(item, "sd", &rot_type, &angle)) {
                if (!strcmp(rot_type, "roll"))   py_obj->quaternion->multiply(c_quaternion().from_euler(angle,0,0,degrees),1);
                if (!strcmp(rot_type, "pitch"))  py_obj->quaternion->multiply(c_quaternion().from_euler(0,angle,0,degrees),1);
                if (!strcmp(rot_type, "yaw"))    py_obj->quaternion->multiply(c_quaternion().from_euler(0,0,angle,degrees),1);
            }
        }
        Py_DECREF(seq);
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

    PyObject *vec_obj;
    double angle=0, cos_angle=0, sin_angle=0;
    int degrees=0;
    static const char *kwlist[] = {"axis", "angle", "cos_angle", "sin_angle", "degrees", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|dddi", (char **)kwlist, 
                                     &PyTypeObject_vector_frame, &vec_obj, &angle, &cos_angle, &sin_angle, &degrees))
        return NULL;

    if (py_obj->quaternion) {
        c_vector *vector;
        if (python_vector_data(vec_obj, 0, &vector)) {
            if ((cos_angle==sin_angle) && (cos_angle==0)) {
                py_obj->quaternion->from_rotation(angle, vector->coords(), degrees);
            } else {
                py_obj->quaternion->from_rotation(cos_angle, sin_angle, vector->coords());
            }
            Py_INCREF(py_obj);
            return self;
        }
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_to_euler
 */
static PyObject *
python_quaternion_method_to_euler(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    int degrees=0;
    static const char *kwlist[] = {"degrees", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", (char **)kwlist, &degrees))
        return NULL;
    if (py_obj->quaternion) {
        double rpy[3];
        double scale=1.0;
        py_obj->quaternion->as_euler(rpy);
        if (degrees) scale=180.0/M_PI;
        return Py_BuildValue("ddd",scale*rpy[0],scale*rpy[1],scale*rpy[2]);
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_to_rotation
 */
static PyObject *
python_quaternion_method_to_rotation(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    int degrees=0;
    static const char *kwlist[] = {"degrees", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", (char **)kwlist, &degrees))
        return NULL;
    if (py_obj->quaternion) {
        double axis[3];
        double angle = py_obj->quaternion->as_rotation(axis);
        if (degrees) angle*=180.0/M_PI;
        return Py_BuildValue("dO",angle,python_vector_from_c(new c_vector(3,axis)));
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_method_to_rotation
 */
static PyObject *
python_quaternion_method_to_rotation_str(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion *py_obj = (t_PyObject_quaternion *)self;
    int degrees=0;
    static const char *kwlist[] = {"degrees", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", (char **)kwlist, &degrees))
        return NULL;
    if (py_obj->quaternion) {
        double axis[3];
        double angle = py_obj->quaternion->as_rotation(axis);
        char buffer[1024];
        if (degrees) angle*=180.0/M_PI;
        sprintf(buffer, "%6.2f:(%lf,%lf,%lf)", angle, axis[0], axis[1], axis[2]);
        return PyString_FromString(buffer);
    }
    Py_RETURN_NONE;
}

/*a Python quaternion infrastructure mthods
 */
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
    PyObject *obj = PyObject_CallObject((PyObject *) &PyTypeObject_quaternion_frame, NULL);
    if (!obj) {
        return Py_None;
    }
    py_obj = (t_PyObject_quaternion *)obj;
    py_obj->quaternion = quaternion;

    return obj;
}

/*f python_quaternion_init_premodule
 */
int python_quaternion_init_premodule(void)
{
    if (PyType_Ready(&PyTypeObject_quaternion_frame) < 0)
        return -1;
    return 0;
}

/*f python_quaternion_init_postmodule
 */
void python_quaternion_init_postmodule(PyObject *module)
{
    Py_INCREF(&PyTypeObject_quaternion_frame);
    PyModule_AddObject(module, "quaternion", (PyObject *)&PyTypeObject_quaternion_frame);
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

