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
 * @file          texture.h
 * @brief         OpenGL texture handling
 *
 */

/*a Wrapper
 */
#ifdef __INC_TEXTURE
#else
#define __INC_TEXTURE

/*a Includes
 */
#include <OpenGL/gl3.h>

/*a Defines
 */
#define STR(x) #x
#define STRINGIFY(x) STR(x)
#define GL_GET_ERRORS do {} while (0);
#define VERBOSE
#ifdef VERBOSE
#undef GL_GET_ERRORS
#define GL_GET_ERRORS do { \
        gl_get_errors( __FILE__  STRINGIFY(__LINE__) ); \
    } while (0);
#endif

/*f gl_get_errors
 */
extern int gl_get_errors(const char *msg);

/*a Types
 */
/*t t_texture_ptr
 */
typedef struct t_texture *t_texture_ptr;

/*t t_texture_header
 */
typedef struct
{
    int width;
    int height;
    GLuint gl_id;
    GLuint format; //NOT USED AT PRESENT
} t_texture_header;

/*f texture_header
 */
inline t_texture_header *texture_header(t_texture_ptr texture) { return (t_texture_header *)texture; }

/*f texture_target_as_framebuffer
 */
extern int
texture_target_as_framebuffer(t_texture_ptr texture);

/*f texture_attach_to_shader
 */
extern void
texture_attach_to_shader(t_texture_ptr texture, int shader_number, GLint t_u);

/*f texture_draw_init
 */
extern void
texture_draw_init(void);

/*f texture_draw_prepare
 */
extern void
texture_draw_prepare(void);

/*f texture_draw_rectangle
 */
extern void
texture_draw_rectangle(void);

/*f texture_draw_tidy
 */
extern void
texture_draw_tidy(void);

/*f texture_draw_through_projections
 */
extern void
texture_draw_through_projections(class c_lens_projection *projections[2], int num_x_divisions, int num_y_divisions);

/*f texture_draw
 */
extern void
texture_draw(void);

/*f texture_get_buffer
 */
extern void *
texture_get_buffer(t_texture_ptr t_texture, int components);

/*f texture_get_buffer_uint
 */
extern void *
texture_get_buffer_uint(t_texture_ptr texture, int components);

/*f texture_save
 */
extern int
texture_save(t_texture_ptr texture, const char *png_filename, int components, int conversion);

/*f texture_load
 */
extern t_texture_ptr 
texture_load(const char *image_filename, GLuint image_type);

/*f texture_create
 */
extern t_texture_ptr 
texture_create(int width, int height);

/*f texture_destroy
 */
extern void 
texture_destroy(t_texture_ptr texture);

/*a Wrapper
 */
#endif

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/
