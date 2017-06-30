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
#include "timer.h"
#include "texture.h"
#include "shader.h"
#include <map>
#include <string>

/*a Defines
 */
#define MAX_FILTER_TEXTURES 8
#define MAX_FILTER_PROJECTIONS 2
#define MAX_FILTER_TIMERS 8
typedef enum
{
    filter_timer_compile,
    filter_timer_execute,
    filter_timer_internal_1,
    filter_timer_internal_2,
} t_filter_timer;

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

/*t fp_validity
 */
enum {
    fp_valid_string=1,
    fp_valid_real=2,
    fp_valid_integer=4,
    fp_valid_gl_id=8,
};

/*t t_filter_parameter
 */
typedef struct
{
    int valid_values;
    const char *string;
    double real;
    int integer;
    GLint gl_id;
} t_filter_parameter;

/*t c_filter
 */
class c_filter
{
private:
    virtual int do_compile(void) {return 0;};
    virtual int do_execute(t_exec_context *ec) {return 0;};

    std::map <std::string, t_filter_parameter> *parameter_map;

    void get_parameter_value(t_filter_parameter *fp);
    void set_parameter(t_filter_parameter *fp, double value);
    void set_parameter(t_filter_parameter *fp, int value);
    void set_parameter(t_filter_parameter *fp, const char *value);

    int read_int_list(t_len_string *string, int *ints, int max_ints);

protected:    
    void set_parameters_from_map(struct t_parameter_def *parameter_defns, void *parameters);
    void set_filename(const char *dirname, const char *suffix, t_len_string *filename, char **filter_filename);
    void get_shader_defines(char **shader_defines);
    int  get_shader_uniform_ids(void);
    int  get_texture_uniform_ids(int num_dest);
    int set_texture_uniforms(t_exec_context *ec, int num_dest);
    int  set_shader_uniforms(void);

public:
    c_filter(t_len_string *textures, t_len_string *parameters);
    virtual ~c_filter();
    int compile(void) {return this->do_compile();};
    int execute(t_exec_context *ec) {return this->do_execute(ec);};

    int uniform_set(const char *uniform, float value); // used in batch, needs to be replaced with set_parameter

    int set_parameter(const char *name, double value);
    int set_parameter(const char *name, int value);
    int set_parameter(const char *name, const char *value);
    int unset_parameter(const char *name);

    const char *parse_error;
    GLuint filter_pid;

    int bind_projection(int n, class c_lens_projection *projection);
    int bind_texture(int n, t_texture_ptr texture);
    t_texture_ptr bound_texture(t_exec_context *ec, int n);

    int num_textures;
    t_filter_texture textures[MAX_FILTER_TEXTURES];
    int num_projections;
    class c_lens_projection *projections[MAX_FILTER_PROJECTIONS];

    t_sl_timer timers[MAX_FILTER_TIMERS];
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
