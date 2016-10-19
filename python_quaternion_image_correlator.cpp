/*a Copyright
  
  This file 'python_quaternion_image_correlator.cpp' copyright Gavin J Stark 2016
  
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
#include "python_quaternion_image_correlator.h"
#include "quaternion_image_correlator.h"

/*a Defines
 */

/*a Types
 */
/*t t_PyObject_quaternion_image_correlator
 */
typedef struct {
    PyObject_HEAD
    c_quaternion_image_correlator *quaternion_image_correlator;
} t_PyObject_quaternion_image_correlator;

/*a Forward function declarations
 */
static int       python_quaternion_image_correlator_init(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *python_quaternion_image_correlator_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static void      python_quaternion_image_correlator_dealloc(PyObject *self);
static PyObject *python_quaternion_image_correlator_getattr(PyObject *self, char *attr);
static int       python_quaternion_image_correlator_setattr(PyObject *o, char *attr_name, PyObject *v);
static PyObject *python_quaternion_image_correlator_str(PyObject *self);

static PyObject *python_quaternion_image_correlator_method_add_match(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_quaternion_image_correlator_method_create_mappings(PyObject* self, PyObject* args);
static PyObject *python_quaternion_image_correlator_method_score_orient(PyObject* self, PyObject* args, PyObject *kwds);

/*a Static variables
 */
/*v python_quaternion_image_correlator_methods
 */
PyMethodDef python_quaternion_image_correlator_methods[] = {
    {"add_match",       (PyCFunction)python_quaternion_image_correlator_method_add_match,       METH_VARARGS|METH_KEYWORDS},
    {"create_mappings", (PyCFunction)python_quaternion_image_correlator_method_create_mappings, METH_NOARGS},
    {"score_orient",    (PyCFunction)python_quaternion_image_correlator_method_score_orient,    METH_VARARGS|METH_KEYWORDS},
    {NULL, NULL},
};

/*v PyTypeObject_quaternion_image_correlator
 */
static PyTypeObject PyTypeObject_quaternion_image_correlator = {
    PyObject_HEAD_INIT(NULL)
    0, // variable size
    "quaternion_image_correlator", // type name
    sizeof(t_PyObject_quaternion_image_correlator), // basic size
    0, // item size - zero for static sized object types
    python_quaternion_image_correlator_dealloc, //py_engine_dealloc, /*tp_dealloc*/
    0, /*tp_print - basically deprecated */
    python_quaternion_image_correlator_getattr, /*tp_getattr*/
    python_quaternion_image_correlator_setattr, /*tp_setattr*/
    0, /*tp_compare*/
    0, /*tp_repr - ideally a represenation that is python that recreates this object */
    0, /*tp_as_number*/
    0, /*tp_as_sequence*/
    0, /*tp_as_mapping*/
    0, /*tp_hash */
	0, /* tp_call - called if the object itself is invoked as a method */
	python_quaternion_image_correlator_str, /* tp_str */
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Quaternion_Image_Correlator object",       /* tp_doc */
    0,		                   /* tp_traverse */
    0,		                   /* tp_clear */
    0,		                   /* tp_richcompare */
    0,		                   /* tp_weaklistoffset */
    0,		                   /* tp_iter */
    0,		                   /* tp_iternext */
    python_quaternion_image_correlator_methods, /* tp_methods */
    0, //python_quaternion_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    python_quaternion_image_correlator_init,    /* tp_init */
    0,                         /* tp_alloc */
    python_quaternion_image_correlator_new,     /* tp_new */
};

/*a Python quaternion_image_correlator object methods
 */
/*f python_quaternion_image_correlator_method_add_match
 */
static PyObject *
python_quaternion_image_correlator_method_add_match(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion_image_correlator *py_obj = (t_PyObject_quaternion_image_correlator *)self;

    double focal_length, frame_width;
    const char *lens_type;
    static const char *kwlist[] = {"focal_length", "frame_width", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "dds", (char **)kwlist, 
                                     &focal_length, &frame_width, &lens_type))
        return NULL;

    if (py_obj->quaternion_image_correlator) {
        //py_obj->quaternion_image_correlator->add_match();
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_image_correlator_method_create_mappings
 */
static PyObject *
python_quaternion_image_correlator_method_create_mappings(PyObject* self, PyObject* args)
{
    t_PyObject_quaternion_image_correlator *py_obj = (t_PyObject_quaternion_image_correlator *)self;

    if (py_obj->quaternion_image_correlator) {
        py_obj->quaternion_image_correlator->create_mappings();
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_image_correlator_method_score_orient
 */
static PyObject *
python_quaternion_image_correlator_method_score_orient(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_quaternion_image_correlator *py_obj = (t_PyObject_quaternion_image_correlator *)self;

    PyObject *orientation=NULL;
    static const char *kwlist[] = {"orientation", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", (char **)kwlist, 
                                     &PyTypeObject_quaternion_frame, &orientation))
        return NULL;

    if (py_obj->quaternion_image_correlator) {
        c_quaternion *quaternion;
        if (python_quaternion_data(orientation, 0, (void *)&quaternion)) {
            py_obj->quaternion_image_correlator->score_src_from_tgt(quaternion);
        }
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_image_correlator_str
 */
static PyObject *
python_quaternion_image_correlator_str(PyObject* self)
{
    t_PyObject_quaternion_image_correlator *py_obj = (t_PyObject_quaternion_image_correlator *)self;
    
    if (py_obj->quaternion_image_correlator) {
        char buffer[1024];
        //py_obj->quaternion_image_correlator->__str__(buffer, sizeof(buffer));
        buffer[0] = 0;
        return PyString_FromFormat("%s", buffer);
    }
    Py_RETURN_NONE;
}

/*f python_quaternion_image_correlator_dealloc
 */
static void
python_quaternion_image_correlator_dealloc(PyObject *self)
{
    t_PyObject_quaternion_image_correlator *py_obj = (t_PyObject_quaternion_image_correlator *)self;
    if (py_obj->quaternion_image_correlator) {
        delete(py_obj->quaternion_image_correlator);
        py_obj->quaternion_image_correlator = NULL;
    }
}

/*f python_quaternion_image_correlator_getattr
 */
static PyObject *
python_quaternion_image_correlator_getattr(PyObject *self, char *attr)
{
    t_PyObject_quaternion_image_correlator *py_obj = (t_PyObject_quaternion_image_correlator *)self;
    
    if (py_obj->quaternion_image_correlator) {
        c_quaternion_image_correlator *qic=py_obj->quaternion_image_correlator;
        if (!strcmp(attr, "min_cos_angle_src_q")) {
            return PyFloat_FromDouble(qic->min_cos_angle_src_q);
        }
        if (!strcmp(attr, "min_cos_angle_tgt_q")) {
            return PyFloat_FromDouble(qic->min_cos_angle_tgt_q);
        }
        if (!strcmp(attr, "min_cos_sep_score")) {
            return PyFloat_FromDouble(qic->min_cos_sep_score);
        }
        if (!strcmp(attr, "max_q_dist_score")) {
            return PyFloat_FromDouble(qic->max_q_dist_score);
        }
        if (!strcmp(attr, "max_angle_diff_ratio")) {
            return PyFloat_FromDouble(qic->max_angle_diff_ratio);
        }
    }
    return Py_FindMethod(python_quaternion_image_correlator_methods, self, attr);
}

/*f python_quaternion_image_correlator_getattr
 */
static int       
python_quaternion_image_correlator_setattr(PyObject *self, char *attr, PyObject *v)
{
    t_PyObject_quaternion_image_correlator *py_obj = (t_PyObject_quaternion_image_correlator *)self;
    
    if (py_obj->quaternion_image_correlator) {
        c_quaternion_image_correlator *qic = py_obj->quaternion_image_correlator;
        if (!strcmp(attr, "min_cos_angle_src_q")) {
            double v_d = PyFloat_AsDouble(v);
            if (!PyErr_Occurred()) {
                qic->min_cos_angle_src_q = v_d;
                return 0;
            }
        }
        if (!strcmp(attr, "min_cos_angle_tgt_q")) {
            double v_d = PyFloat_AsDouble(v);
            if (!PyErr_Occurred()) {
                qic->min_cos_angle_tgt_q = v_d;
                return 0;
            }
        }
        if (!strcmp(attr, "min_cos_sep_score")) {
            double v_d = PyFloat_AsDouble(v);
            if (!PyErr_Occurred()) {
                qic->min_cos_sep_score = v_d;
                return 0;
            }
        }
        if (!strcmp(attr, "max_q_dist_score")) {
            double v_d = PyFloat_AsDouble(v);
            if (!PyErr_Occurred()) {
                qic->max_q_dist_score = v_d;
                return 0;
            }
        }
        if (!strcmp(attr, "max_angle_diff_ratio")) {
            double v_d = PyFloat_AsDouble(v);
            if (!PyErr_Occurred()) {
                qic->max_angle_diff_ratio = v_d;
                return 0;
            }
        }
    }
    return -1;
}

/*a Python object
 */
/*f python_quaternion_image_correlator_init_premodule
 */
int python_quaternion_image_correlator_init_premodule(void)
{
    if (PyType_Ready(&PyTypeObject_quaternion_image_correlator) < 0)
        return -1;
    return 0;
}

/*f python_quaternion_image_correlator_init_postmodule
 */
void python_quaternion_image_correlator_init_postmodule(PyObject *module)
{
    Py_INCREF(&PyTypeObject_quaternion_image_correlator);
    PyModule_AddObject(module, "quaternion_image_correlator", (PyObject *)&PyTypeObject_quaternion_image_correlator);
}

/*f python_quaternion_image_correlator_new
 */
static PyObject *
python_quaternion_image_correlator_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    t_PyObject_quaternion_image_correlator *py_obj;
    py_obj = (t_PyObject_quaternion_image_correlator *)type->tp_alloc(type, 0);
    if (py_obj) {
        py_obj->quaternion_image_correlator = NULL;
    }
    return (PyObject *)py_obj;
}

/*f python_quaternion_image_correlator_init
 */
static int
python_quaternion_image_correlator_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    t_PyObject_quaternion_image_correlator *py_obj = (t_PyObject_quaternion_image_correlator *)self;

    static const char *kwlist[] = {NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "", (char **)kwlist
            ))
        return -1;

    py_obj->quaternion_image_correlator = new c_quaternion_image_correlator();
    if (!py_obj->quaternion_image_correlator)
        return -1;

    return 0;
}

/*a External data access
 */
/*f python_quaternion_image_correlator_data
 */
int
python_quaternion_image_correlator_data(PyObject* self, int id, void *data_ptr)
{
    t_PyObject_quaternion_image_correlator *py_obj = (t_PyObject_quaternion_image_correlator *)self;
    if (!PyObject_TypeCheck(self, &PyTypeObject_quaternion_image_correlator))
        return 0;

    ((c_quaternion_image_correlator **)data_ptr)[0] = py_obj->quaternion_image_correlator;
    return 1;
}


