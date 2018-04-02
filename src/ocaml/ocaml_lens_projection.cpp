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
 * @file          ocaml_lens_projection.cpp
 * @brief         Lens projections and methods thereon
 *
 */

/*a Documentation
 * 
 * This is a wrapper around the ATCF library image/lens_projection class
 *
 * It provides ocaml wrappers for the methods in 'lens_projection.h'
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

#include <atcf/quaternion.h>
#include <atcf/lens_projection.h>
#include "ocaml_atcflib.h"

#include <stdio.h>

/*a Defines
 */
// Use -D__OCAML_LENS_PROJECTION_VERBOSE on compilation to be verbose,
// or uncomment the following
//#define __OCAML_LENS_PROJECTION_VERBOSE
#ifdef __OCAML_LENS_PROJECTION_VERBOSE
#define VERBOSE fprintf
#else
#define VERBOSE __discard
static void __discard(void *, ...) {}
#endif

/*a Statics
 */
static void finalize_lens_projection(value v)
{
    delete lens_projection_of_val(v);
}

static struct custom_operations custom_ops = {
    (char *)"atcf.lens_projection",
    finalize_lens_projection,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default
};

/*a Creation functions
 */
/*f caml_atcf_alloc_lens_projection
 *
 * Creates a lens_projection from a NEW c_lens_projection
 *
 */
extern value
caml_atcf_alloc_lens_projection(c_lens_projection *cm)
{
    value v = caml_alloc_custom(&custom_ops, sizeof(c_lens_projection *), 0, 1);
    lens_projection_of_val(v) = cm;
    VERBOSE(stderr,"Created lens_projection %p\n", cm);
    return v;
}

/*f atcf_lens_projection_create : r:unit -> NEW c_lens_projection
 *
 * Creates a lens_projection zero
 *
 */
extern "C"
CAMLprim value
atcf_lens_projection_create(void)
{
    CAMLparam0();
    VERBOSE(stderr,"Create lens_projection\n");
    CAMLreturn(caml_atcf_alloc_lens_projection(new c_lens_projection()));
}

/*f atcf_lens_projection_create_rijk : r:float -> i:float ...  -> NEW c_lens_projection
 *
 * Creates a lens_projection with r, i, j, k
 *
 */
extern "C"
CAMLprim value
atcf_lens_projection_create_rijk(value r, value i, value j, value k)
{
    CAMLparam4(r,i,j,k);
    VERBOSE(stderr,"Create lens_projection\n");
    c_lens_projection *lp=new c_lens_projection();
    CAMLreturn(caml_atcf_alloc_lens_projection(lp));
}

/*f atcf_lens_projection_destroy : c_lens_projection -> unit
 *
 * Destroys a lens_projection
 *
 */
extern "C"
CAMLprim void
atcf_lens_projection_destroy(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Destroy lens_projection %p\n", lens_projection_of_val(v));
    delete lens_projection_of_val(v);
    lens_projection_of_val(v) = NULL;
    CAMLreturn0;
}

/*f atcf_lens_projection_clone : c_lens_projection -> NEW c_lens_projection
 *
 * Clones a lens_projection
 *
 */
extern "C"
CAMLprim value
atcf_lens_projection_clone(value v)
{
    CAMLparam1(v);
    VERBOSE(stderr,"Clone lens_projection %p\n", lens_projection_of_val(v));
    CAMLreturn(caml_atcf_alloc_lens_projection(new c_lens_projection(*lens_projection_of_val(v))));
}

/*a Interrogation functions
 */
/*f atcf_lens_projection_info : c_lens_projection -> float array
 *
 * Return an array containing the sensor width, sensor height, focal length, frame width
 *
 */
extern "C"
CAMLprim value
atcf_lens_projection_info(value lp)
{
    double rpy[3];
    CAMLparam1(lp);
    c_lens_projection *clp = lens_projection_of_val(lp);
    value v = caml_alloc_float_array(4);
    Store_double_field(v,0,clp->get_sensor_width());
    Store_double_field(v,1,clp->get_sensor_height());
    Store_double_field(v,2,clp->get_focal_length());
    Store_double_field(v,3,clp->get_frame_width());
    CAMLreturn(v);
}

/*f atcf_lens_projection_orientation : c_lens_projection -> c_quaternion
 *
 * Function to find orientation quaternion of lens projection
 *
 */
extern "C"
CAMLprim value
atcf_lens_projection_orientation(value lp) {
    CAMLparam1(lp);
    c_quaternion<double> q = lens_projection_of_val(lp)->get_orientation();
    CAMLreturn(caml_atcf_alloc_quaternion(q.copy()));
}

/*f atcf_lens_projection_orientation_of_xy : c_lens_projection -> float -> float -> c_quaternion
 *
 * Function to find orientation quaternion of an (x,y) in a sensor of a lens projection
 *
 */
extern "C"
CAMLprim value
atcf_lens_projection_orientation_of_xy(value lp, value x, value y) {
    CAMLparam3(lp, x, y);
    double xy[2] = {x, y};
    c_quaternion<double> q = lens_projection_of_val(lp)->orientation_of_(xy);
    CAMLreturn(caml_atcf_alloc_quaternion(q.copy()));
}

/*f atcf_lens_projection_xy_of_orientation : c_lens_projection -> quaternion -> (float * float)
 *
 * Function to find sensor (x,y) of an orientation quaternion in a lens projection
 *
 */
extern "C"
CAMLprim value
atcf_lens_projection_xy_of_orientation(value lp, value q) {
    CAMLparam2(lp, q);
    double xy[2];
    lens_projection_of_val(lp)->xy_of_orientation(quaternion_of_val(q), xy);
    CAMLreturn(
}

