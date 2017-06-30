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
#include "python_image_correlator.h"
#include "image_correlator.h"

/*a Defines
 */

/*a Types
 */
/*t t_PyObject_image_correlator
 */
typedef struct t_PyObject_image_correlator {
    PyObject_HEAD
    c_image_correlator *image_correlator;
    int handle;
} t_PyObject_image_correlator;

/*a Forward function declarations
 */
static int       python_image_correlator_init(PyObject *self, PyObject *args, PyObject *kwds);
static PyObject *python_image_correlator_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static void      python_image_correlator_dealloc(PyObject *self);
static PyObject *python_image_correlator_getattr(PyObject *self, char *attr);

static PyObject *python_image_correlator_method_add_point(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_image_correlator_method_add_mapping(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_image_correlator_method_mappings(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_image_correlator_method_get_mapping(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_image_correlator_method_create_propositions(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_image_correlator_method_propositions(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_image_correlator_method_get_proposition(PyObject* self, PyObject* args, PyObject *kwds);
static PyObject *python_image_correlator_method_find_best_mapping(PyObject* self, PyObject* args, PyObject *kwds);

static void      python_image_correlator_dealloc(PyObject *self);

/*a Static variables
 */
/*v python_image_correlator_methods
 */
static PyMethodDef python_image_correlator_methods[] = {
    {"add_point",   (PyCFunction)python_image_correlator_method_add_point,   METH_VARARGS|METH_KEYWORDS},
    {"add_mapping", (PyCFunction)python_image_correlator_method_add_mapping, METH_VARARGS|METH_KEYWORDS},
    {"mappings",    (PyCFunction)python_image_correlator_method_mappings,    METH_VARARGS|METH_KEYWORDS},
    {"get_mapping", (PyCFunction)python_image_correlator_method_get_mapping, METH_VARARGS|METH_KEYWORDS},
    {"create_propositions", (PyCFunction)python_image_correlator_method_create_propositions, METH_VARARGS|METH_KEYWORDS},
    {"propositions", (PyCFunction)python_image_correlator_method_propositions, METH_VARARGS|METH_KEYWORDS},
    {"get_proposition", (PyCFunction)python_image_correlator_method_get_proposition, METH_VARARGS|METH_KEYWORDS},
    {"find_best_mapping", (PyCFunction)python_image_correlator_method_find_best_mapping, METH_NOARGS},
    {NULL, NULL},
};

/*v PyTypeObject_image_correlator
 */
static PyTypeObject PyTypeObject_image_correlator = {
    PyObject_HEAD_INIT(NULL)
    0, // variable size
    "image_correlator", // type name
    sizeof(t_PyObject_image_correlator), // basic size
    0, // item size - zero for static sized object types
    python_image_correlator_dealloc, //py_engine_dealloc, /*tp_dealloc*/
    0, /*tp_print - basically deprecated */
    python_image_correlator_getattr, /*tp_getattr*/
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
    "Image_Correlator object",       /* tp_doc */
    0,		                   /* tp_traverse */
    0,		                   /* tp_clear */
    0,		                   /* tp_richcompare */
    0,		                   /* tp_weaklistoffset */
    0,		                   /* tp_iter */
    0,		                   /* tp_iternext */
    python_image_correlator_methods, /* tp_methods */
    0, //python_quaternion_members, /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    python_image_correlator_init,    /* tp_init */
    0,                         /* tp_alloc */
    python_image_correlator_new,     /* tp_new */
};

/*a Python image_correlator methods
 */
/*f python_image_correlator_method_add_point
 */
static PyObject *
python_image_correlator_method_add_point(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    const char *name;
    double x, y;

    static const char *kwlist[] = {"name", "x", "y", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sdd", (char **)kwlist, 
                                     &name, &x, &y))
        return NULL;

    if (py_obj->image_correlator) {
        int err;
        err = py_obj->image_correlator->add_mapping_point(name, x, y);
        if (err) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to add point");
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

/*f python_image_correlator_method_add_mapping
 */
static PyObject *
python_image_correlator_method_add_mapping(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    const char *name, *pv_name;
    int x, y;
    double power, r=0, i=0;

    static const char *kwlist[] = {"name", "pv", "x", "y", "power", "r", "i", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ssiid|dd", (char **)kwlist, 
                                     &name, &pv_name, &x, &y, &power, &r, &i))
        return NULL;

    if (py_obj->image_correlator) {
        int err;
        t_point_value pv;
        pv.x = x;
        pv.y = y;
        pv.value = power;
        pv.vec_x = r;
        pv.vec_y = i;
        err = py_obj->image_correlator->add_mapping_point_pv(name, pv_name, &pv);
        if (err) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to add point");
            return NULL;
        }
    }
    Py_RETURN_NONE;
}

/*f python_image_correlator_method_mappings
 */
static PyObject *
python_image_correlator_method_mappings(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    const char *name;
    static const char *kwlist[] = {"name"};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", (char **)kwlist, &name)) return NULL;

    if (py_obj->image_correlator) {
        PyObject *l;
        if (py_obj->image_correlator->mapping_points.count(name)==0)
            Py_RETURN_NONE;
        l = PyList_New(0);
        if (!l) return NULL;
        for (int i=0; ; i++) {
            const char *pv_name;
            pv_name = py_obj->image_correlator->get_mapping_point_pv_name(name, i);
            if (!pv_name) break;
            PyList_Append(l, PyString_FromString(pv_name));
        }
        return l;
    }
    Py_RETURN_NONE;
}

/*f python_image_correlator_method_get_mapping
 */
static PyObject *
python_image_correlator_method_get_mapping(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    const char *name, *pv_name;
    static const char *kwlist[] = {"name", "pv_name", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "ss", (char **)kwlist,
                                     &name, &pv_name)) return NULL;

    if (py_obj->image_correlator) {
        t_point_value *pv;
        pv = py_obj->image_correlator->get_mapping_point_pv(name, pv_name);
        if (!pv) {
            Py_RETURN_NONE;
        }
        return Py_BuildValue("iiddd", (int)pv->x, (int)pv->y, (double)pv->value, (double)pv->vec_x, (double)pv->vec_y);
    }
    Py_RETURN_NONE;
}

/*f python_image_correlator_method_create_propositions
 */
static PyObject *
python_image_correlator_method_create_propositions(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    double min_strength=0.0;
    static const char *kwlist[] = {"min_strength", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "d", (char **)kwlist,
                                     &min_strength)) return NULL;

    if (py_obj->image_correlator) {
        py_obj->image_correlator->create_propositions(min_strength, stderr);
    }
    Py_RETURN_NONE;
}

/*f python_image_correlator_method_propositions
 */
static PyObject *
python_image_correlator_method_propositions(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    const char *pv_name;
    static const char *kwlist[] = {"pv_name", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", (char **)kwlist,
                                     &pv_name)) return NULL;

    if (py_obj->image_correlator) {
        return PyInt_FromLong(py_obj->image_correlator->number_of_propositions(pv_name));
    }
    Py_RETURN_NONE;
}

/*f python_proposition_value
 */
static PyObject *
python_proposition_value(t_image_correlation_proposition *proposition)
{
    return Py_BuildValue("dddd",
                         (double)proposition->translation[0],
                         (double)proposition->translation[1],
                         (double)proposition->rotation,
                         (double)proposition->scale);
                         
}

/*f python_image_correlator_method_get_proposition
 */
static PyObject *
python_image_correlator_method_get_proposition(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    const char *pv_name;
    int n;
    static const char *kwlist[] = {"pv_name", "n", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "si", (char **)kwlist,
                                     &pv_name, &n)) return NULL;

    if (py_obj->image_correlator) {
        return python_proposition_value(py_obj->image_correlator->get_proposition(pv_name,n));
    }
    Py_RETURN_NONE;
}

/*f python_image_correlator_method_find_best_mapping
 */
static PyObject *
python_image_correlator_method_find_best_mapping(PyObject* self, PyObject* args, PyObject *kwds)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    if (py_obj->image_correlator) {
        t_image_correlation_proposition best_image_correlation_proposition;
        double strength = py_obj->image_correlator->find_best_mapping(&best_image_correlation_proposition, stderr);
        return Py_BuildValue("OO", PyFloat_FromDouble(strength),
                             python_proposition_value(&best_image_correlation_proposition));
    }
    Py_RETURN_NONE;
}

/*f python_image_correlator_dealloc
 */
static void
python_image_correlator_dealloc(PyObject *self)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    if (py_obj->image_correlator) {
        delete py_obj->image_correlator;
        py_obj->image_correlator = NULL;
    }
}

/*f python_image_correlator_getattr
 */
static PyObject *
python_image_correlator_getattr(PyObject *self, char *attr)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    
    if (py_obj->image_correlator) {
        if (!strcmp(attr, "points")) {
            c_image_correlator *ic = py_obj->image_correlator;
            PyObject *l;
            l = PyList_New(0);
            if (!l) return NULL;
            for (auto m:ic->mapping_points) {
                PyList_Append(l, PyString_FromString(m.first.c_str()));
            }
            return l;
        }
    }
    return Py_FindMethod(python_image_correlator_methods, self, attr);
}

/*a Python object
 */
/*f python_image_correlator_init_premodule
 */
int python_image_correlator_init_premodule(void)
{
    if (PyType_Ready(&PyTypeObject_image_correlator) < 0)
        return -1;
    return 0;
}

/*f python_image_correlator_init_postmodule
 */
void python_image_correlator_init_postmodule(PyObject *module)
{
    Py_INCREF(&PyTypeObject_image_correlator);
    PyModule_AddObject(module, "image_correlator", (PyObject *)&PyTypeObject_image_correlator);
}

/*f python_image_correlator_new
 */
static PyObject *
python_image_correlator_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    t_PyObject_image_correlator *py_obj;
    py_obj = (t_PyObject_image_correlator *)type->tp_alloc(type, 0);
    if (py_obj) {
        py_obj->image_correlator = NULL;
    }
    return (PyObject *)py_obj;
}

/*f python_image_correlator_init
 */
static int
python_image_correlator_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;

    int width=0, height=0, components, precision;
    const char *filename = NULL;

    static const char *kwlist[] = {"width", "height", "filename", "components", "precision", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|iisii", (char **)kwlist, 
                                     &width, &height, &filename, &components, &precision))
        return -1;

    py_obj->image_correlator = new c_image_correlator();
    return 0;
}

/*a Data sharing with other objects
 */
/*f python_image_correlator_data
 */
int
python_image_correlator_data(PyObject* self, int id, void *data_ptr)
{
    t_PyObject_image_correlator *py_obj = (t_PyObject_image_correlator *)self;
    if (!PyObject_TypeCheck(self, &PyTypeObject_image_correlator))
        return 0;

    ((c_image_correlator **)data_ptr)[0] = py_obj->image_correlator;
    return 1;
}

