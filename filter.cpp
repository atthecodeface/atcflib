/*a Documentation
 */
/*a Includes
 */
#define GL_GLEXT_PROTOTYPES
#define GLM_FORCE_RADIANS
#include <SDL.h> 
#include <SDL_opengl.h>
#include <stdlib.h>
#include <string.h>
#include "filter.h"

/*a Types
 */

/*a c_filter methods
 */
/*t c_filter_save
 */
class c_filter_save : public c_filter
{
public:
    c_filter_save(t_len_string *filename, t_len_string *options_list, t_len_string *uniforms);
    char *save_filename;
    int texture_to_save;

    virtual int execute(t_exec_context *ec);
};

/*t c_filter_glsl
 */
class c_filter_glsl : public c_filter
{
public:
    c_filter_glsl(t_len_string *filename, t_len_string *options_list, t_len_string *uniforms);
    char *filter_filename;
    GLint uniform_texture_src_id;
    GLint uniform_texture_base_id;
    GLint uniform_texture_base_x;
    GLint uniform_texture_base_y;
    int texture_src;
    int texture_dest;
    int texture_base;
    char *shader_defines;

    virtual int compile(void);
    virtual int execute(t_exec_context *ec);
};

/*t c_filter_correlate
 */
class c_filter_correlate : public c_filter
{
public:
    c_filter_correlate(t_len_string *filename, t_len_string *options_list, t_len_string *uniforms);
    char *filter_filename;
    int texture_src;
    int texture_dest;
    GLint uniform_texture_src_id;
    GLint uniform_out_xy_id;
    GLint uniform_out_size_id;
    GLint uniform_src_xy_id;
    char *shader_defines;

    virtual int compile(void);
    virtual int execute(t_exec_context *ec);
};
enum
{
    uniform_value_type_int,
    uniform_value_type_float
};
typedef struct
{
    int    value_type;
    int    value_int;
    float  value_float;
    GLint  gl_id;
} t_filter_key_value_data;

/*t c_filter_find
 */
class c_filter_find : public c_filter
{
public:
    c_filter_find(t_len_string *filename, t_len_string *options_list, t_len_string *uniforms);
    int texture_src;
    int texture_dest;
    int perimeter;
    float minimum;
    float min_distance;
    int max_elements;
    int num_elements;
    t_point_value *points;

    virtual int compile(void);
    virtual int execute(t_exec_context *ec);
};

/*f c_filter constructor
 */
c_filter::c_filter(void)
{
    parse_error = NULL;
    key_value_init(&uniform_key_values, sizeof(t_filter_key_value_data));
    filter_pid = 0;
    return;
}

/*f c_filter::set_filename
 */
void c_filter::set_filename(const char *dirname, const char *suffix, t_len_string *filename, char **filter_filename)
{
    char *ptr;
    int buffer_length;
    buffer_length = filename->len+strlen("shaders/")+10;
    ptr = *filter_filename = (char *)malloc(buffer_length);
    if (dirname) {
        ptr = strcpy( *filter_filename,  dirname);
    }
    ptr = strncpy(ptr + strlen(ptr), filename->ptr, filename->len);
    ptr[filename->len]=0;
    if (suffix) {
        ptr = strcpy( ptr + filename->len, suffix);
    }
}

/*f c_filter::set_uniforms
 */
void c_filter::set_uniforms(t_len_string *uniforms)
{
    const char *ptr;
    fprintf(stderr,"Uniforms %s\n",uniforms->ptr);
    ptr = uniforms->ptr;
    while (ptr && ((ptr-uniforms->ptr)<uniforms->len)) {
        ptr = key_value_parse(ptr, uniforms->ptr+uniforms->len, &uniform_key_values);
    }
}

/*f c_filter::get_shader_defines
 */
void c_filter::get_shader_defines(char **shader_defines)
{
    t_key_value_entry_ptr kve;
    (*shader_defines) = (char *)malloc(1024);
    (*shader_defines)[0] = 0;
    kve = key_value_iter(&uniform_key_values, NULL);
    while (kve) {
        if (!strncmp("-D", kve->key, 2)) {
            sprintf((*shader_defines)+strlen((*shader_defines)),
                    "#define %s %s\n",
                    kve->key+2, kve->value);
        }
        kve = key_value_iter(&uniform_key_values, kve);
    }
}

/*f c_filter::get_value_from_key_value
 */
int c_filter::get_value_from_key_value(t_key_value_entry_ptr kve)
{
    t_filter_key_value_data *kvd;
    int value_len;
    kvd = key_value_entry_data(kve, t_filter_key_value_data);
    value_len = strlen(kve->value);
    if (value_len<1) return 1;
    if (kve->value[value_len-1]=='f') {
        if (sscanf(kve->value, "%ff", &kvd->value_float)!=1) return 1;
        kvd->value_type = uniform_value_type_float;
        return 0;
    }
    if (sscanf(kve->value, "%d", &kvd->value_int)!=1) return 1;
    kvd->value_type = uniform_value_type_int;
    return 0;
}

/*f c_filter::get_shader_uniform_ids
 */
int c_filter::get_shader_uniform_ids(void)
{
    t_key_value_entry_ptr kve;
    int failures;
    failures = 0;
    kve = key_value_iter(&uniform_key_values, NULL);
    while (kve) {
        if (!strncmp("-U", kve->key, 2)) {
            t_filter_key_value_data *kvd;
            kvd = key_value_entry_data(kve, t_filter_key_value_data);
            kvd->gl_id = glGetUniformLocation(filter_pid, kve->key+2);
            if (kvd->gl_id<=0) {
                fprintf(stderr, "Failed to find uniform '%s' in shader\n", kve->key+2);
                failures++;
            }
            if (get_value_from_key_value(kve)) {
                fprintf(stderr, "Failed to parse uniform value '%s' in shader\n", key_value_entry_value(kve));
                failures++;
            }
        }
        kve = key_value_iter(&uniform_key_values, kve);
    }
    return failures;
}

/*f c_filter::uniform_set
 */
int c_filter::uniform_set(const char *uniform, float value)
{
    GLint gl_id;
    if (filter_pid==0)
        return 0;
    glUseProgram(filter_pid);
    gl_id = glGetUniformLocation(filter_pid, uniform);
    GL_GET_ERRORS;
    if (gl_id>0) {
        glUniform1f(gl_id, value);
        GL_GET_ERRORS;
        return 0;
    }
    GL_GET_ERRORS;
    return 1;
}

/*f c_filter destructor
 */
c_filter::~c_filter(void)
{
    return;
}

/*f c_filter_glsl constructor
 */
c_filter_glsl::c_filter_glsl(t_len_string *filename, t_len_string *options_list, t_len_string *uniforms) : c_filter()
{
    set_filename("shaders/", ".glsl", filename, &filter_filename);
    set_uniforms(uniforms);
    filter_pid = 0;
    uniform_texture_src_id = 0;
    if (sscanf(options_list->ptr,"%d,%d,%d",&texture_base,&texture_src,&texture_dest)==3) {
    } else {
        texture_base = 0;
        if (sscanf(options_list->ptr,"%d,%d",&texture_src,&texture_dest)!=2) {
            parse_error = "Failed to parse GLSL texture options - need '(<src>,<dst>)' texture numbers";
        }
    }
}

/*f c_filter_glsl::compile
 */
int c_filter_glsl::compile(void)
{
    get_shader_defines(&shader_defines);
    filter_pid = shader_load_and_link(0, "shaders/vertex_shader.glsl", filter_filename, shader_defines);
    if (filter_pid==0) {
        return 1;
    }
    if (get_shader_uniform_ids())
        return 1;
    uniform_texture_src_id = glGetUniformLocation(filter_pid, "texture_src");
    uniform_texture_base_id = -1;
    uniform_texture_base_x = -1;
    uniform_texture_base_y = -1;
    if (texture_base!=0) {
        gl_get_errors("before get uniforms");
        uniform_texture_base_id = glGetUniformLocation(filter_pid, "texture_base");
        uniform_texture_base_x = glGetUniformLocation(filter_pid, "uv_base_x");
        uniform_texture_base_y = glGetUniformLocation(filter_pid, "uv_base_y");
        gl_get_errors("after get uniforms");
    }
    return 0;
}

/*f c_filter_glsl::execute
 */
int c_filter_glsl::execute(t_exec_context *ec)
{
    GL_GET_ERRORS;
    texture_target_as_framebuffer(ec->textures[texture_dest]);
    glUseProgram(filter_pid);

    GL_GET_ERRORS;

    if (uniform_texture_base_id>=0) {
        GL_GET_ERRORS;

        texture_attach_to_shader(ec->textures[texture_base], 1, uniform_texture_base_id);
        glActiveTexture(GL_TEXTURE0);
        GL_GET_ERRORS;
    }

    GL_GET_ERRORS;

    texture_attach_to_shader(ec->textures[texture_src], 0, uniform_texture_src_id);
    GL_GET_ERRORS;
    texture_draw();

    GL_GET_ERRORS;

    return 0;
}

/*f c_filter_correlate constructor
 */
c_filter_correlate::c_filter_correlate(t_len_string *filename, t_len_string *options_list, t_len_string *uniforms) : c_filter()
{
    set_filename("shaders/", ".glsl", filename, &filter_filename);
    filter_pid = 0;
    uniform_texture_src_id = 0;
    if (sscanf(options_list->ptr,"%d,%d",&texture_src,&texture_dest)!=2) {
        parse_error = "Failed to parse GLSL texture options - need '(<src>,<dst>)' texture numbers";
    }
}

/*f c_filter_correlate::compile
 */
int c_filter_correlate::compile(void)
{
    get_shader_defines(&shader_defines);
    filter_pid = shader_load_and_link(0, "shaders/vertex_correlation_shader.glsl", filter_filename, shader_defines);
    if (filter_pid==0) {
        return 1;
    }
    if (get_shader_uniform_ids())
        return 1;
    uniform_texture_src_id = glGetUniformLocation(filter_pid, "texture_src");
    uniform_out_xy_id      = glGetUniformLocation(filter_pid, "out_xy");
    uniform_out_size_id    = glGetUniformLocation(filter_pid, "out_size");
    uniform_src_xy_id      = glGetUniformLocation(filter_pid, "src_xy");
    return 0;
}

/*f c_filter_correlate::execute
 */
int c_filter_correlate::execute(t_exec_context *ec)
{
    GL_GET_ERRORS;
    texture_target_as_framebuffer(ec->textures[texture_dest]);

    glClearColor(0.2,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(filter_pid);
    glUniform2f(uniform_out_size_id,32,32);
    GL_GET_ERRORS;
    texture_attach_to_shader(ec->textures[texture_src], 0, uniform_texture_src_id);
    texture_draw_prepare();

    for (int i=0; i<ec->num_points; i++) {
        glUniform2f(uniform_out_xy_id,i*40,0);
        glUniform2f(uniform_src_xy_id,ec->points[i].x,ec->points[i].y);
        texture_draw_rectangle();
    }

    texture_draw_tidy();
    return 0;
}

/*f c_filter_find constructor
 */
c_filter_find::c_filter_find(t_len_string *filename, t_len_string *options_list, t_len_string *uniforms) : c_filter()
{
    perimeter = 10;
    minimum = 0.0;
    max_elements = 320;
    min_distance = 10.0;
    if (sscanf(options_list->ptr,"%d",&texture_src)!=1) {
        parse_error = "Failed to parse find texture options - need '(<src>)' texture number";
    }
}

/*f c_filter_find::compile
 */
int c_filter_find::compile(void)
{
    return 0;
}

/*f c_filter_find::execute
 */
int c_filter_find::execute(t_exec_context *ec)
{
    t_texture *texture;
    float *raw_img;
    float elements_minimum;
    int   n;
    const void *raw_buffer;
    const t_texture_header *texture_hdr;

    raw_buffer = texture_get_buffer(ec->textures[texture_src]);
    texture = ec->textures[texture_src];
    texture_hdr = texture_header(texture);
    raw_img = (float *)raw_buffer;

    elements_minimum = -1.0;
    n=0;
    points = (t_point_value *)malloc(sizeof(t_point_value)*max_elements);
    for (int y=perimeter; y<texture_hdr->height-perimeter; y++) {
        for (int x=perimeter; x<texture_hdr->width-perimeter; x++) {
            int i;
            float value_xy;
            value_xy = raw_img[y*texture_hdr->width+x];
            if (value_xy<=elements_minimum) continue;
            if (value_xy<minimum) continue;
            for (i=0; i<n; i++) {
                if (points[i].value<value_xy) break;
            }
            if (n==max_elements) n--;
            if (i<n) {
                memmove(&points[i+1], &points[i], sizeof(t_point_value)*(n-i));
            }
            n++;
            points[i].x=x;
            points[i].y=y;
            points[i].value = value_xy;
            if (n==max_elements) {
                elements_minimum = points[n-1].value;
            }
        }
    }
    float min_distance_sq;
    min_distance_sq = min_distance * min_distance;
    for (int i=0; i<n; i++) {
        int j;
        j = i+1;
        while (j<n) {
            float dx, dy, d_sq;
            dx = points[i].x-points[j].x;
            dy = points[i].y-points[j].y;
            d_sq = dx*dx+dy*dy;
            if (d_sq>min_distance_sq) {
                j++;
                continue;
            }
            n--;
            if (j<n) {
                memmove(&points[j], &points[j+1], sizeof(t_point_value)*(n-j));
            }
        }
    }
    for (int i=0; (i<n) && (i<10); i++) {
        fprintf(stderr,"%d: (%d,%d) = %f\n", i, points[i].x, points[i].y, points[i].value);
    }
    if (ec->points) free(ec->points);
    ec->points = points;
    ec->num_points = n;
    return 0;
}

/*f c_filter_save constructor
 */
c_filter_save::c_filter_save(t_len_string *filename, t_len_string *options_list, t_len_string *uniforms) : c_filter()
{
    set_filename(NULL, NULL, filename, &save_filename);
    if (sscanf(options_list->ptr,"%d",&texture_to_save)!=1) {
        parse_error = "Failed to parse save texture - need '(<src>)' texture number";
    }
}

/*f c_filter_save::execute
 */
int c_filter_save::execute(t_exec_context *ec)
{
    return texture_save(ec->textures[texture_to_save], save_filename);
}

/*a External functions
 */
/*f filter_from_string
  filter string must be:
  <filter type>:<filename>(<options list>)[&<uniform_name>=<float>]
 */
c_filter *
filter_from_string(const char *optarg)
{
    char ch[2]; // hack to get sscanf to be useful...
    t_len_string filter_type;
    t_len_string filename;
    t_len_string options_list;
    t_len_string uniforms;

    if ((sscanf(optarg,"%*[a-z]%n:%*[a-zA-Z0-9_.]%n(%*[0-9,]%n%c",
                &filter_type.len,
                &filename.len,
                &options_list.len,
                ch)!=1) || (ch[0]!=')')) {
        fprintf(stderr, "Failed to parse filter string '%s'\n", optarg);
        return NULL;
    }
    filter_type.ptr  = optarg;
    filename.ptr     = optarg + filter_type.len+1;
    options_list.ptr = optarg + filename.len+1;
    uniforms.ptr     = optarg + options_list.len+1;
    filename.len     = options_list.ptr-filename.ptr-1;
    options_list.len = uniforms.ptr-options_list.ptr-1;
    uniforms.len     = strlen(uniforms.ptr);
    if (!strncmp(filter_type.ptr, "glsl", 4)) {
        return new c_filter_glsl(&filename, &options_list, &uniforms);
    } else if (!strncmp(filter_type.ptr, "find", 4)) {
        return new c_filter_find(&filename, &options_list, &uniforms);
    } else if (!strncmp(filter_type.ptr, "corr", 4)) {
        return new c_filter_correlate(&filename, &options_list, &uniforms);
    } else if (!strncmp(filter_type.ptr, "save", 4)) {
        return new c_filter_save(&filename, &options_list, &uniforms);
    }
    fprintf(stderr, "Failed to parse filter string '%s' - bad filter type probably\n", optarg);
    return NULL;
}

