/** Copyright (C) 2017,  Gavin J Stark.  All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @file          ocaml_vector.cpp
 * @brief         Vectors and methods thereon
 *
 */

/*a Documentation
 * 
 * This is a wrapper around the ATCF library vector class
 *
 * It provides ocaml wrappers for the vector methods in 'vector.h'
 *
 */
/*a Includes
 */
#define CAML_NAME_SPACE 
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/intext.h>
#include <caml/threads.h>
#include <caml/bigarray.h>

#include "quaternion.h"
#include "vector.h"
#include "matrix.h"

#include "ocaml_atcflib.h"

#include <stdio.h>

/*a Defines
 */
// Use -D__OCAML_VECTOR_VERBOSE on compilation to be verbose,
// or uncomment the following
// #define __OCAML_VECTOR_VERBOSE
#ifdef __OCAML_VECTOR_VERBOSE
#define VERBOSE fprintf
#else
#define VERBOSE __discard
static void __discard(void *, ...) {}
#endif

/*a Statics
 */
/*f finalize
  Remove our value reference to the bigarray and delete the associated object
 */
static void finalize(value v)
{
    t_math_obj *m = math_obj_of_val(v);
    caml_remove_global_root(&m->ba);
    switch (m->mt) {
    case MT_V_FLOAT:  {delete (m->ptr.vf); break; }
    case MT_V_DOUBLE: {delete (m->ptr.vd); break; }
    case MT_M_FLOAT:  {delete (m->ptr.mf); break; }
    case MT_M_DOUBLE: {delete (m->ptr.md); break; }
    case MT_Q_FLOAT:  {delete (m->ptr.qf); break; }
    case MT_Q_DOUBLE: {delete (m->ptr.qd); break; }
    }
}

static struct custom_operations custom_ops = {
    (char *)"atcf.math_obj",
    finalize,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default
};

/*a Creation functions
 */
/*f caml_atcf_alloc_math_obj
 *
 * Creates a
 *
 */
extern void
caml_atcf_alloc_math_obj(value *v, t_math_type mt, value *ba, void *ptr)
{
    *v = caml_alloc_custom(&custom_ops, sizeof(t_math_obj), 0, 1);
    caml_remove_global_root(v);
    math_obj_of_val(*v)->mt = mt;
    math_obj_of_val(*v)->ba = *ba;
    caml_register_global_root(&math_obj_of_val(*v)->ba);
    math_obj_of_val(*v)->ptr.v = ptr;
    VERBOSE(stderr,"Allocked caml math object %p type %d ba %p\n", ptr, mt, Caml_ba_data_val(*ba));
}

extern int
math_obj_validate( value v, t_math_type mt, ...)
{
    t_math_obj *m = math_obj_of_val(v);
    if (m->mt != mt) {
        caml_invalid_argument("Expected different math obj type");
        return 0;
    }

    //switch (mt) {}
    return 1;
}
