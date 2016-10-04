/** Copyright (C) 2016,  Gavin J Stark.  All rights reserved.
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
 * @file          python_quaternion.h
 * @brief         Python wrapper for quaternions
 *
 */

/*a Wrapper
 */
#ifdef __INC_PYTHON_QUATERNION
#else
#define __INC_PYTHON_QUATERNION

/*a Includes
 */
#include "quaternion.h"

/*a External functions
 */
extern void python_quaternion_init(void);
extern PyObject *python_quaternion(PyObject* self, PyObject* args, PyObject *kwds);
extern PyObject *python_quaternion_from_c(c_quaternion *quaternion);
extern int python_quaternion_data(PyObject* self, int id, void *data_ptr);

/*a Wrapper
 */
#endif

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/
