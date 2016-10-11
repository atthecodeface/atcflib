/*a Copyright
  
  This file 'gjslib_c.cpp' copyright Gavin J Stark 2016
  
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
#include "python_filter.h"
#include "python_texture.h"
#include "python_image_correlator.h"
#include "python_lens_projection.h"
#include "python_quaternion.h"

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
/*v gjslib_c_module_methods
 */
static PyMethodDef gjslib_c_module_methods[] =
{
    {NULL, NULL, 0, NULL}        /* Sentinel */
};

/*a C code for gjslib_c
 */
extern "C" void initgjslib_c( void )
{
    PyObject *module;
    if ( python_quaternion_init_premodule() ||
         python_image_correlator_init_premodule() ||
         python_texture_init_premodule() ||
         python_lens_projection_init_premodule() ||
         python_filter_init_premodule() ) {
        fprintf(stderr,"Failed initialization of classes\n");
        return;
    }
    module =  Py_InitModule3("gjslib_c", gjslib_c_module_methods, "C version of gjslib" );
    if (!module) {
        fprintf(stderr,"Failed initialization of module\n");
        return;
    }
    python_lens_projection_init_postmodule(module);
    python_texture_init_postmodule(module);
    python_image_correlator_init_postmodule(module);
    python_filter_init_postmodule(module);
    python_quaternion_init_postmodule(module);
}

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/

