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
 * @file          ocaml_timer.cpp
 * @brief         Timestamp interaction thereon
 *
 */

/*a Documentation
 * 
 * This is a simple OCaml set of function to use the x86 timestamp
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

#include <atcf/timer.h>

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

/*f timer_of_val c_vector from an OCAML value
 */
#define timer_of_val(v) (*((t_sl_timer *) Data_custom_val(v)))

/*a Statics
 */
static struct custom_operations custom_ops = {
    (char *)"atcf.timer",
    custom_finalize_default,
    custom_compare_default,
    custom_hash_default,
    custom_serialize_default,
    custom_deserialize_default,
    custom_compare_ext_default
};

/*f alloc_timer
 *
 * Creates a timer
 *
 */
static value
caml_atcf_alloc_timer(void)
{
    value v = caml_alloc_custom(&custom_ops, sizeof(t_sl_timer), 0, 1);
    SL_TIMER_INIT(timer_of_val(v));
    return v;
}

/*a Creation functions
 */
/*f atcf_time_alloc
 *
 * Creates a timer
 *
 */
extern "C"
CAMLprim value
atcf_timer_create(void)
{
    CAMLparam0();
    CAMLreturn(caml_atcf_alloc_timer());
}

/*f atcf_timer_init
 *
 */
extern "C"
CAMLprim void
atcf_timer_init(value v)
{
    CAMLparam1(v);
    SL_TIMER_INIT(timer_of_val(v));
    CAMLreturn0;
}

/*f atcf_timer_entry
 *
 */
extern "C"
CAMLprim void
atcf_timer_entry(value v)
{
    CAMLparam1(v);
    SL_TIMER_ENTRY(timer_of_val(v));
    CAMLreturn0;
}

/*f atcf_timer_exit
 *
 */
extern "C"
CAMLprim void
atcf_timer_exit(value v)
{
    CAMLparam1(v);
    SL_TIMER_EXIT(timer_of_val(v));
    CAMLreturn0;
}

/*f atcf_timer_value
 *
 */
extern "C"
CAMLprim value
atcf_timer_value(value v)
{
    CAMLparam1(v);
    CAMLreturn(caml_copy_int64(SL_TIMER_VALUE(timer_of_val(v))));
}

/*f atcf_timer_value_us
 *
 */
extern "C"
CAMLprim value
atcf_timer_value_us(value v)
{
    CAMLparam1(v);
    CAMLreturn(caml_copy_double(SL_TIMER_VALUE_US(timer_of_val(v))));
}

