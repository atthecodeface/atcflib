/*a Copyright
  
  This file 'python_wrapper.cpp' copyright Gavin J Stark 2016
  
  This is free software; you can redistribute it and/or modify it however you wish,
  with no obligations
  
  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*a Includes
 */
#include <Python.h>
#include <OpenGL/gl3.h>
#include "python_texture.h"

/*a Defines
 */

/*a Types
 */

/*f gl_get_errors
 */
int gl_get_errors(const char *msg)
{
    GLenum err;
    int num_errors;
    num_errors = 0;
    while ((err = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr,"OpenGL error %s : %d\n", msg, err);
        num_errors++;
    }
    return num_errors;
}

/*a Statics
 */
/*v python_wrapper_module_methods
 */
static PyMethodDef python_wrapper_module_methods[] =
{
    {"texture", (PyCFunction)python_texture, METH_VARARGS|METH_KEYWORDS, "Create a new texture object."},
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

/*a C code for python_wrapper
 */
extern "C" void initpython_wrapper( void )
{
    (void) Py_InitModule3("python_wrapper", python_wrapper_module_methods, "Python interface to wrapped thing" );
}

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/

