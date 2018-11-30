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
// Use -D__OCAML_MATH_OBJ_VERBOSE on compilation to be verbose,
// or uncomment the following
//#define __OCAML_MATH_OBJ_VERBOSE
#ifdef __OCAML_MATH_OBJ_VERBOSE
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
static void math_obj_finalize(value v)
{
    t_math_obj *m = math_obj_of_val(v);
    VERBOSE(stderr,"Ocaml math object finalize %p type %d ba %p\n", m, m->mt, Caml_ba_data_val(m->ba));
    caml_remove_global_root(&m->ba);
    switch (m->mt) {
    case MT_V_FLOAT:  {delete (m->ptr.vf); break; }
    case MT_V_DOUBLE: {delete (m->ptr.vd); break; }
    case MT_M_FLOAT:  {delete (m->ptr.mf); break; }
    case MT_M_DOUBLE: {delete (m->ptr.md); break; }
    }
    free(m);
}

static struct custom_operations custom_ops = {
    (char *)"atcf.math_obj",
    math_obj_finalize,
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
caml_atcf_alloc_math_obj(value *v, t_math_type mt, value *ba, int ofs, void *ptr)
{
    t_math_obj *mv = (t_math_obj *)malloc(sizeof(t_math_obj));
    *v = caml_alloc_custom(&custom_ops, sizeof(t_math_obj *), 0, 1);
    math_obj_of_val(*v) = mv;
    //caml_remove_global_root(v);
    mv->mt = mt;
    mv->ba = *ba;
    mv->ofs = ofs;
    mv->ptr.v = ptr;
    caml_register_global_root(&mv->ba);
    VERBOSE(stderr,"Allocked caml math object %p ptr %p type %d ba data %p ofs %d\n", mv, ptr, mt, Caml_ba_data_val(*ba), ofs);
}

extern int
math_obj_validate(t_math_obj *m, t_math_type mt, ...)
{
    if (m->mt != mt) {
        caml_invalid_argument("Expected different math obj type");
        return 0;
    }

    void *ba_data = Caml_ba_data_val(m->ba);
    switch (mt) {
    case MT_V_FLOAT:  {m->ptr.vf->set_coords((float *)ba_data+m->ofs); break; }
    case MT_V_DOUBLE: {m->ptr.vd->set_coords((double *)ba_data+m->ofs); break; }
    case MT_M_FLOAT:  {m->ptr.mf->set_values((float *)ba_data+m->ofs); break; }
    case MT_M_DOUBLE: {m->ptr.md->set_values((double *)ba_data+m->ofs); break; }
    }
    return 1;
}

extern int
math_obj_validate_is_v(t_math_obj *m)
{
    if (m->mt==MT_V_FLOAT) {
        return math_obj_validate(m, MT_V_FLOAT);
    }
    return math_obj_validate(m, MT_V_DOUBLE);
}

extern int
math_obj_validate_is_m(t_math_obj *m)
{
    if (m->mt==MT_M_FLOAT) {
        return math_obj_validate(m, MT_M_FLOAT);
    }
    return math_obj_validate(m, MT_M_DOUBLE);
}
