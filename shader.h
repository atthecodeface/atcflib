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
 * @file          shader.h
 * @brief         OpenGL glsl shader handling
 *
 */

/*a Wrapper
 */
#ifdef __INC_SHADER
#else
#define __INC_SHADER

/*a Includes
 */
#include <OpenGL/gl3.h>

/*a Defines
 */

/*a External functions
 */
extern int
shader_init(void);

extern GLuint
shader_load(const char *shader_filename, GLenum shader_type, const char *shader_defines);

/*f shader_load_and_link
 */
extern GLuint
shader_load_and_link(GLuint program_id, const char *vertex_shader, const char *fragment_shader, const char *shader_defines);

/*a Wrapper
 */
#endif

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/
