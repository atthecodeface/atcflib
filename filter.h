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
} t_point_value;

/*t t_exec_context
 */
typedef struct
{
    t_texture_ptr textures[16];
    t_point_value *points;
    int num_points;
} t_exec_context;

/*t c_filter
 */
class c_filter
{
public:
    c_filter(void);
    ~c_filter();
    void set_filename(const char *dirname, const char *suffix, t_len_string *filename, char **filter_filename);
    void set_uniforms(t_len_string *uniforms);
    int uniform_set(const char *uniform, float value);
    void get_shader_defines(char **shader_defines);
    int  get_shader_uniform_ids(void);
    int  get_value_from_key_value(t_key_value_entry_ptr kve);
    virtual int compile(void) {return 0;};
    virtual int execute(t_exec_context *ec) {return 0;};
    const char *parse_error;
    t_key_values uniform_key_values;
    GLuint filter_pid;
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
