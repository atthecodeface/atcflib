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
 * @file          filter.h
 * @brief         Image filters
 *
 */

/*a Wrapper
 */
#ifdef __INC_FILTER
#else
#define __INC_FILTER

/*a Includes
 */
#include "texture.h"
#include "key_value.h"
#include "shader.h"

/*a Defines
 */
#define MAX_FILTER_TEXTURES 8

/*a Types
 */
/*t t_len_string
 */
typedef struct
{
    const char *ptr;
    int len;
} t_len_string;

/*t t_point_value
 */
typedef struct
{
    int x;
    int y;
    float value;
    float vec_x;
    float vec_y;
    int extra[4];
} t_point_value;

/*t t_exec_context
 */
typedef struct
{
    t_texture_ptr textures[16];
    t_point_value *points;
    int num_points;
    int use_ids;
} t_exec_context;

/*t t_filter_texture
 */
typedef struct {
    int ec_id;
    t_texture_ptr texture;
    GLint sampler_id;
} t_filter_texture;

/*t c_filter
 */
class c_filter
{
public:
    c_filter(void);
    virtual ~c_filter();
    void free_filter(void);
    int read_int_list(t_len_string *string, int *ints, int max_ints);
    int read_texture_int_list(t_len_string *string);
    void set_filename(const char *dirname, const char *suffix, t_len_string *filename, char **filter_filename);
    void set_key_values(t_len_string *ls, t_key_values *kvs);
    int uniform_set(const char *uniform, float value);
    void get_shader_defines(char **shader_defines);
    int  get_shader_uniform_ids(void);
    int  get_texture_uniform_ids(int num_dest);
    int set_texture_uniforms(t_exec_context *ec, int num_dest);
    int  set_shader_uniforms(void);
    int  get_value_from_key_value(t_key_value_entry_ptr kve);
    int bind_texture(int n, t_texture_ptr texture);
    t_texture_ptr bound_texture(t_exec_context *ec, int n);
    virtual int compile(void) {return 0;};
    virtual int execute(t_exec_context *ec) {return 0;};
    const char *parse_error;
    t_key_values option_key_values;
    GLuint filter_pid;
    int num_textures;
    t_filter_texture textures[MAX_FILTER_TEXTURES];
};

/*a External functions
 */
/*f filter_from_string
  filter string must be:
  <filter type>:<filename>(<options list>)[&<uniform_name>=<float>]
 */
c_filter *
filter_from_string(const char *optarg);

/*a Wrapper
 */
#endif

/*a Editor preferences and notes
mode: c ***
c-basic-offset: 4 ***
c-default-style: (quote ((c-mode . "k&r") (c++-mode . "k&r"))) ***
outline-regexp: "/\\\*a\\\|[\t ]*\/\\\*[b-z][\t ]" ***
*/
