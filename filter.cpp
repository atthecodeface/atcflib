/*a Documentation
 */
/*a Includes
 */
#include <OpenGL/gl3.h> 
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "filter.h"
#define PI 3.1415926538

/*a Types
 */

/*a c_filter class and subclasses
 */
/*t t_parameter_def
 */
typedef struct t_parameter_def
{
    const char *name;
    char type;
    int this_offset;
} t_parameter_def;

/*t t_filter_save_parameters
 */
typedef struct
{
    int components;
    int conversion;
} t_filter_save_parameters;

/*t c_filter_save
 */
class c_filter_save : public c_filter
{
    static t_parameter_def parameter_defns[];
public:
    c_filter_save(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string);
    ~c_filter_save();
    char *save_filename;
    t_filter_save_parameters parameters;
    virtual int do_execute(t_exec_context *ec);
};

/*v c_filter_save::parameter_defns
 */
t_parameter_def c_filter_save::parameter_defns[] = {
        {"conversion", 'i', offsetof(t_filter_save_parameters,conversion)},
        {"components", 'i', offsetof(t_filter_save_parameters,components)},
        {NULL, 0, 0}
    };

/*t c_filter_glsl
 */
class c_filter_glsl : public c_filter
{
public:
    c_filter_glsl(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string);
    ~c_filter_glsl();
    char *filter_filename;
    char *shader_defines;

    virtual int do_compile(void);
    virtual int do_execute(t_exec_context *ec);
};

/*t c_filter_correlate
 */
class c_filter_correlate : public c_filter
{
public:
    c_filter_correlate(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string);
    ~c_filter_correlate();
    char *filter_filename;
    char *shader_defines;

    virtual int do_compile(void);
    virtual int do_execute(t_exec_context *ec);
};

/*t t_filter_find_parameters
 */
typedef struct
{
    int perimeter;
    float minimum;
    float min_distance;
    int max_elements;
} t_filter_find_parameters;

/*t c_filter_find
 */
class c_filter_find : public c_filter
{
    static t_parameter_def parameter_defns[];
public:
    c_filter_find(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string);
    ~c_filter_find();
    t_filter_find_parameters parameters;
    int num_elements;
    t_point_value *points;

    virtual int do_compile(void);
    virtual int do_execute(t_exec_context *ec);
};

/*v c_filter_find::parameter_defns
 */
t_parameter_def c_filter_find::parameter_defns[] = {
        {"perimeter", 'i', offsetof(t_filter_find_parameters,perimeter)},
        {"max_elements", 'i', offsetof(t_filter_find_parameters,max_elements)},
        {"min_distance", 'f', offsetof(t_filter_find_parameters,min_distance)},
        {"minimum", 'f', offsetof(t_filter_find_parameters,minimum)},
        {NULL, 0, 0}
    };

/*t t_filter_find4_parameters
 */
typedef struct
{
    int perimeter;
    float minimum;
    float min_distance;
    int max_elements;
} t_filter_find4_parameters;

/*t c_filter_find4
 */
class c_filter_find4 : public c_filter
{
    static t_parameter_def parameter_defns[];
public:
    c_filter_find4(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string);
    ~c_filter_find4();
    t_filter_find4_parameters parameters;
    int num_elements;
    t_point_value *points;

    virtual int do_compile(void);
    virtual int do_execute(t_exec_context *ec);
};

/*v c_filter_find4::parameter_defns
 */
t_parameter_def c_filter_find4::parameter_defns[] = {
        {"perimeter", 'i', offsetof(t_filter_find4_parameters,perimeter)},
        {"max_elements", 'i', offsetof(t_filter_find4_parameters,max_elements)},
        {"min_distance", 'f', offsetof(t_filter_find4_parameters,min_distance)},
        {"minimum", 'f', offsetof(t_filter_find4_parameters,minimum)},
        {NULL, 0, 0}
    };

/*a c_filter constructor and destructor methods
 */
/*f c_filter constructor
 */
c_filter::c_filter(t_len_string *textures, t_len_string *parameter_string)
{
    int texture_ec_ids[MAX_FILTER_TEXTURES];

    parse_error = NULL;

    filter_pid = 0;
    for (int i=0; i<MAX_FILTER_TEXTURES; i++) {
        this->textures[i].texture = NULL;
        this->textures[i].sampler_id = -1;
        this->textures[i].ec_id = -1;
    }

    num_textures = read_int_list(textures, texture_ec_ids, MAX_FILTER_TEXTURES);
    for (int i=0; i<num_textures; i++) {
        this->textures[i].ec_id = texture_ec_ids[i];
    }

    parameter_map = new std::map <std::string, t_filter_parameter>();

    const char *end = parameter_string->ptr+parameter_string->len;
    for (const char *ptr=parameter_string->ptr; ptr<end;) {
        const char *value_end;
        const char *equals;
        while (ptr[0]=='&') ptr++;
        value_end = strchr(ptr,'&');
        equals = strchr(ptr,'=');
        value_end = (!value_end) ? end : value_end;
        value_end = (value_end>end) ? end : value_end;
        equals = (equals>value_end) ? NULL : equals;

        std::string key_string;
        std::string value_string;
        if (equals) {
            key_string  =std::string(ptr,equals-ptr);
            value_string=std::string(equals+1,value_end-equals+1);
        } else {
            key_string  =std::string(ptr,value_end-ptr);
            value_string=std::string("");
        }
        t_filter_parameter &fp = (*parameter_map)[key_string];
        fp.valid_values = fp_valid_string;
        fp.string = value_string.c_str();
        ptr = value_end;
    }

    return;
}

/*f c_filter destructor
 */
c_filter::~c_filter(void)
{
    // delete strings in parameter_map
    delete parameter_map;
    if (filter_pid!=0) {
        shader_delete(filter_pid);
        filter_pid = 0;
    }
    return;
}

/*a c_filter parameter methods
 */
/*f c_filter::set_parameter(name, int value)
 */
int c_filter::set_parameter(const char *name, int value)
{
    auto &fp=(*parameter_map)[name];
    set_parameter(&fp, value );
    return 0;
}

/*f c_filter::set_parameter(filter parameter, int value)
 */
void c_filter::set_parameter(t_filter_parameter *fp, int value)
{
    fp->valid_values &= ~(fp_valid_string);
    fp->valid_values |= fp_valid_integer | fp_valid_real;
    fp->integer = value;
    fp->real = value;
}

/*f c_filter::set_parameter(name, double value)
 */
int c_filter::set_parameter(const char *name, double value)
{
    auto &fp=(*parameter_map)[name];
    set_parameter(&fp, value );
    return 0;
}

/*f c_filter::set_parameter(filter parameter, double value)
 */
void c_filter::set_parameter(t_filter_parameter *fp, double value)
{
    fp->valid_values &= ~(fp_valid_string | fp_valid_integer);
    fp->valid_values |= fp_valid_real;
    fp->real = value;
}

/*f c_filter::set_parameter(name, string value)
 */
int c_filter::set_parameter(const char *name, const char *value)
{
    auto &fp=(*parameter_map)[name];
    set_parameter(&fp, value );
    return 0;
}

/*f c_filter::set_parameter(filter parameter, string value)
 */
void c_filter::set_parameter(t_filter_parameter *fp, const char *value)
{
    fp->valid_values &= ~(fp_valid_real | fp_valid_integer);
    fp->valid_values |= fp_valid_string;
    fp->string = value;
}

/*f c_filter::unset_parameter
 */
int c_filter::unset_parameter(const char *name)
{
    parameter_map->erase(name);
    return 0;
}

/*c c_filter::get_parameter_value
 */
void c_filter::get_parameter_value(t_filter_parameter *fp)
{
    if (fp->valid_values & fp_valid_string) {
        double f;
        if (sscanf(fp->string, "%lf", &f)!=1) {
            if (f==(int)f) {
                set_parameter(fp, (int)f);
            } else {
                set_parameter(fp, f);
            }
        }
    }
}

/*f c_filter::set_parameters_from_string
 */
void c_filter::set_parameters_from_string(t_len_string *parameter_string, t_parameter_def *parameter_defns, void *parameters)
{
    for (int i=0; parameter_defns[i].name; i++) {
        // change to find the parameter by name - then use default
        for (auto fpi = parameter_map->begin(); fpi != parameter_map->end(); ++fpi) {
            if (!fpi->first.compare(parameter_defns[i].name)) {
                char *p = ((char *)parameters) + parameter_defns[i].this_offset;
                if (parameter_defns[i].type=='i') {
                    ((int *)p)[0] = atoi(fpi->second.string);
                }
                if (parameter_defns[i].type=='f') {
                    double f;
                    if (sscanf(fpi->second.string, "%lf", &f)!=1) {
                        parse_error = "Failed to parse float";
                    }
                    ((double *)p)[0] = f;
                }
            }
        }
    }
}

/*a c_filter other methods
 */
/*f c_filter::read_int_list
  requires string to not flow in to more digits
 */
int c_filter::read_int_list(t_len_string *string, int *ints, int max_ints)
{
    const char *ptr;
    int num_ints;
    ptr = string->ptr;
    num_ints = 0;
    while (ptr-string->ptr < string->len) {
        long value;
        char *next_ptr;
        value = strtol(ptr, &next_ptr, 0);
        if (ptr!=next_ptr) {
            ints[num_ints] = value;
            num_ints++;
            if (num_ints==max_ints)
                break;
        }
        while (next_ptr-string->ptr < string->len) {
            int ch;
            ch = next_ptr[0];
            if (!isspace(ch) && (ch!=','))
                break;
            next_ptr++;
        }
        ptr = next_ptr;
    }
    return num_ints;
}

/*f c_filter::set_filename
 */
void c_filter::set_filename(const char *dirname, const char *suffix, t_len_string *filename, char **filter_filename)
{
    char *ptr;
    int buffer_length;
    buffer_length = filename->len+strlen("shaders/")+10;
    ptr = *filter_filename = (char *)malloc(buffer_length);
    ptr[0] = 0;
    if (dirname) {
        ptr = strcpy( *filter_filename,  dirname);
    }
    ptr = strncpy(ptr + strlen(ptr), filename->ptr, filename->len);
    ptr[filename->len]=0;
    if (suffix) {
        ptr = strcpy( ptr + filename->len, suffix);
    }
}

/*f c_filter::get_shader_defines
 */
void c_filter::get_shader_defines(char **shader_defines)
{
    (*shader_defines) = (char *)malloc(1024);
    (*shader_defines)[0] = 0;
    for (auto fpi = parameter_map->begin(); fpi != parameter_map->end(); ++fpi) {
        if (!fpi->first.compare(0,2,"-D")) {
            if (fpi->second.valid_values & fp_valid_string) {
                sprintf((*shader_defines)+strlen((*shader_defines)),
                        "#define %s %s\n",
                        fpi->first.c_str()+2, fpi->second.string);
            }
        }
    }
}

/*f c_filter::get_texture_uniform_ids
 */
int c_filter::get_texture_uniform_ids(int num_dest)
{
    int failures;
    gl_get_errors("before get texture uniforms");
    failures = 0;
    for (int i=0; i<num_textures-num_dest; i++) {
        char buffer[32];
        sprintf(buffer, "texture_%d", i);
        textures[i].sampler_id = glGetUniformLocation(filter_pid, buffer);
        if (textures[i].sampler_id<0) {
            fprintf(stderr, "Failed to find sampler uniform '%s' in shader\n", buffer);
            failures++;
        }
    }
    gl_get_errors("after get texture uniforms");
    return failures;
}

/*f c_filter::bind_texture
 */
int c_filter::bind_texture(int n, t_texture_ptr texture)
{
    if ((n>=0) && (n<=MAX_FILTER_TEXTURES)) {
        textures[n].texture = texture;
        return 0;
    }
    return -1;
}

/*f c_filter::bound_texture
 */
t_texture_ptr c_filter::bound_texture(t_exec_context *ec, int n)
{
    t_texture_ptr texture;
    if ((n<0) || (n>=num_textures))
        return NULL;
    texture = textures[n].texture;
    if (ec) {
        if (ec->use_ids) {
            texture = ec->textures[textures[n].ec_id];
        }
    }
    return texture;
}

/*f c_filter::set_texture_uniforms
 */
int c_filter::set_texture_uniforms(t_exec_context *ec, int num_dest)
{
    int failures;
    gl_get_errors("before set texture uniforms");
    failures = 0;
    for (int i=0; i<num_textures; i++) {
        if (textures[i].sampler_id>=0) {
            t_texture_ptr texture;
            texture = bound_texture(ec, i);
            if (texture) {
                texture_attach_to_shader(texture, i, textures[i].sampler_id);
            }
        }
    }
    glActiveTexture(GL_TEXTURE0);
    gl_get_errors("after set texture uniforms");
    return failures;
}

/*f c_filter::get_shader_uniform_ids
 * Do this at compile time
 */
int c_filter::get_shader_uniform_ids(void)
{
    int failures;
    gl_get_errors("before get shader uniforms");
    failures = 0;
    for (auto fpi = parameter_map->begin(); fpi != parameter_map->end(); ++fpi) {
        fpi->second.gl_id = glGetUniformLocation(filter_pid, fpi->first.c_str());
        if (fpi->second.gl_id>=0) {
            fpi->second.valid_values |= fp_valid_gl_id;
        }
    }
    gl_get_errors("after get shader uniforms");
    return failures;
}

/*f c_filter::set_shader_uniforms
 * Do this at execute time
 */
int c_filter::set_shader_uniforms(void)
{
    int failures;
    gl_get_errors("before set shader uniforms");
    failures = 0;
    for (auto fpi = parameter_map->begin(); fpi != parameter_map->end(); ++fpi) {
        if (fpi->second.valid_values & fp_valid_gl_id) {
            if (!(fpi->second.valid_values & fp_valid_real)) {
                get_parameter_value(&(fpi->second));
            }
            if (fpi->second.valid_values & fp_valid_real) {
                glUniform1f(fpi->second.gl_id, fpi->second.real);
                fprintf(stderr,"Set shader id %s %d to %f\n",
                        fpi->first.c_str(),
                        fpi->second.gl_id,
                        fpi->second.real); 
            }
        }
    }
    gl_get_errors("after set shader uniforms");
    return failures;
}

/*f c_filter::uniform_set
 */
int c_filter::uniform_set(const char *uniform, float value)
{
    set_parameter(uniform, (double)value);
    return 1;
    GLint gl_id;
    if (filter_pid==0)
        return 0;
    glUseProgram(filter_pid);
    gl_id = glGetUniformLocation(filter_pid, uniform);
    GL_GET_ERRORS;
    if (gl_id>=0) {
        glUniform1f(gl_id, value);
        GL_GET_ERRORS;
        return 0;
    }
    GL_GET_ERRORS;
    return 1;
}

/*a c_filter_glsl methods
 */
/*f c_filter_glsl constructor
 */
c_filter_glsl::c_filter_glsl(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string)
    : c_filter(textures, parameter_string)
{
    set_filename("shaders/", ".glsl", filename, &filter_filename);
    if (num_textures<2) {
        parse_error = "Failed to parse GLSL texture options - need at least '(<src>+,<dst>)' texture numbers";
    }
}

/*f c_filter_glsl destructor
 */
c_filter_glsl::~c_filter_glsl()
{
    return;
}

/*f c_filter_glsl::do_compile
 */
int c_filter_glsl::do_compile(void)
{
    get_shader_defines(&shader_defines);
    filter_pid = shader_load_and_link(0, "shaders/vertex_shader.glsl", filter_filename, shader_defines);
    if (filter_pid==0) {
        parse_error = "Failed to load and link shader";
        return 1;
    }
    if (get_shader_uniform_ids()) {
        parse_error = "Failed to get shader uniform ids";
        return 1;
    }
    if (get_texture_uniform_ids(1)) {
        parse_error = "Failed to get texture uniform ids";
        return 1;
    }
    return 0;
}

/*f c_filter_glsl::do_execute
 */
int c_filter_glsl::do_execute(t_exec_context *ec)
{
    GL_GET_ERRORS;
    texture_target_as_framebuffer(bound_texture(ec,num_textures-1));
    glUseProgram(filter_pid);

    set_shader_uniforms();
    set_texture_uniforms(ec, 1);

    texture_draw();

    GL_GET_ERRORS;

    return 0;
}

/*a c_filter_correlate methods
 */
/*f c_filter_correlate constructor
 */
c_filter_correlate::c_filter_correlate(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string)
    : c_filter(textures, parameter_string)
{
    set_filename("shaders/", ".glsl", filename, &filter_filename);
    if (num_textures<2) {
        parse_error = "Failed to parse GLSL texture options - need at least '(<src>+,<dst>)' texture numbers";
    }
}

/*f c_filter_correlate destructor
 */
c_filter_correlate::~c_filter_correlate()
{
    return;
}

/*f c_filter_correlate::do_compile
 */
int c_filter_correlate::do_compile(void)
{
    get_shader_defines(&shader_defines);
    filter_pid = shader_load_and_link(0, "shaders/vertex_correlation_shader.glsl", filter_filename, shader_defines);
    if (filter_pid==0) {
        return 1;
    }
    if (get_shader_uniform_ids())
        return 1;
    if (get_texture_uniform_ids(1))
        return 1;
    //uniform_out_xy_id      = glGetUniformLocation(filter_pid, "out_xy");
    //uniform_out_size_id    = glGetUniformLocation(filter_pid, "out_size");
    //uniform_src_xy_id      = glGetUniformLocation(filter_pid, "src_xy");
    return 0;
}

/*f c_filter_correlate::do_execute
 */
int c_filter_correlate::do_execute(t_exec_context *ec)
{
    GL_GET_ERRORS;
    texture_target_as_framebuffer(bound_texture(ec,num_textures-1));

    glClearColor(0.2,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(filter_pid);
    //glUniform2f(uniform_out_size_id,32,32);

    set_shader_uniforms();
    set_texture_uniforms(ec, 1);

    GL_GET_ERRORS;
    texture_draw_prepare();

    //for (int i=0; i<ec->num_points; i++) {
    //glUniform2f(uniform_out_xy_id,i*40,0);
    //glUniform2f(uniform_src_xy_id,ec->points[i].x,ec->points[i].y);
    //texture_draw_rectangle();
    //}

    texture_draw_tidy();
    return 0;
}

/*a c_filter_find methods
 */
/*f c_filter_find constructor
 */
c_filter_find::c_filter_find(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string)
    : c_filter(textures, parameter_string)
{
    parameters.perimeter = 10;
    parameters.minimum = 0.0;
    parameters.max_elements = 320;
    parameters.min_distance = 10.0;

    set_parameters_from_string(parameter_string, parameter_defns, (void *)&parameters);
    if (num_textures!=1) {
        parse_error = "Failed to parse find texture options - need '(<src>)' texture number";
    }
}

/*f c_filter_find destructor
 */
c_filter_find::~c_filter_find()
{
    return;
}

/*f c_filter_find::do_compile
 */
int c_filter_find::do_compile(void)
{
    return 0;
}

/*f c_filter_find::do_execute
 */
int c_filter_find::do_execute(t_exec_context *ec)
{
    t_texture *texture;
    const t_texture_header *texture_hdr;
    float *raw_img;
    float elements_minimum;
    int   n;
    int w, h;

    texture = bound_texture(ec, 0);

    texture_hdr = texture_header(texture);
    raw_img = (float *)texture_get_buffer(texture, GL_RGBA);
    w = texture_hdr->width;
    h = texture_hdr->height;

    elements_minimum = -1.0;
    n=0;
    points = (t_point_value *)malloc(sizeof(t_point_value)*parameters.max_elements);
    for (int y=parameters.perimeter; y<h-parameters.perimeter; y++) {
        for (int x=parameters.perimeter; x<w-parameters.perimeter; x++) {
            int i;
            float value_xy;
            value_xy = raw_img[(y*w+x)*4+0];
            if (value_xy<=elements_minimum) continue;
            if (value_xy<parameters.minimum) continue;
            for (i=0; i<n; i++) {
                if (points[i].value<value_xy) break;
            }
            if (n==parameters.max_elements) n--;
            if (i<n) {
                memmove(&points[i+1], &points[i], sizeof(t_point_value)*(n-i));
            }
            n++;
            points[i].x=x;
            points[i].y=y;
            points[i].value = value_xy;
            points[i].vec_x = raw_img[(y*w+x)*4+1];
            points[i].vec_y = raw_img[(y*w+x)*4+2];
            if (n==parameters.max_elements) {
                elements_minimum = points[n-1].value;
            }
        }
    }
    float min_distance_sq;
    min_distance_sq = parameters.min_distance * parameters.min_distance;
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
        fprintf(stderr,"%d: (%d,%d) = %f : %5.2f (%8.5f, %8.5f)\n",
                i, points[i].x, points[i].y, points[i].value,
                180/PI*atan2(points[i].vec_y, points[i].vec_x),
                points[i].vec_x, points[i].vec_y
            );
    }
    if (ec->points) free(ec->points);
    ec->points = points;
    ec->num_points = n;
    return 0;
}

/*a c_filter_save methods
 */
/*f c_filter_save constructor
 */
c_filter_save::c_filter_save(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string)
    : c_filter(textures, parameter_string)
{
    set_filename(NULL, NULL, filename, &save_filename);
    parameters.conversion = 0;
    parameters.components = 0;

    set_parameters_from_string(parameter_string, parameter_defns, (void *)&parameters);
    if (num_textures!=1) {
        parse_error = "Failed to parse save texture - need '(<src>)' texture number";
    }
}

/*f c_filter_save destructor
 */
c_filter_save::~c_filter_save()
{
    return;
}

/*f c_filter_save::do_execute
 */
int c_filter_save::do_execute(t_exec_context *ec)
{
    t_texture_ptr texture;

    texture = bound_texture(ec, 0);

    if (0) {
        fprintf(stderr, "Saving to '%s'\n",save_filename);
    }
    return texture_save(texture, save_filename, parameters.components, parameters.conversion);
}

/*a c_filter_find4 methods
 */
/*f c_filter_find4 constructor
 */
c_filter_find4::c_filter_find4(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string)
    : c_filter(textures, parameter_string)
{
    parameters.perimeter = 10;
    parameters.minimum = 0.0;
    parameters.max_elements = 320;
    parameters.min_distance = 2.5;
    set_parameters_from_string(parameter_string, parameter_defns, (void *)&parameters);
    if (num_textures!=4) {
        parse_error = "Failed to parse find texture options - need '(<src>*4)' texture number";
    }
}

/*f c_filter_find4 destructor
 */
c_filter_find4::~c_filter_find4()
{
    return;
}

/*f c_filter_find4::do_compile
 */
int c_filter_find4::do_compile(void)
{
    return 0;
}

/*f c_filter_find4::do_execute
 */
int c_filter_find4::do_execute(t_exec_context *ec)
{
    float elements_minimum;
    int   n;
    const t_texture_header *texture_hdr[4];
    float *raw_img[4];
    int w, h;

    for (int i=0; i<4; i++) {
        const void *raw_buffer;
        t_texture *t;
        t = bound_texture(ec, i);
        raw_buffer = texture_get_buffer(t, GL_RGBA);
        texture_hdr[i] = texture_header(t);
        w = texture_hdr[i]->width;
        h = texture_hdr[i]->height;
        raw_img[i] = (float *)raw_buffer;
    }

    elements_minimum = -1.0;
    n=0;
    points = (t_point_value *)malloc(sizeof(t_point_value)*parameters.max_elements);
    // for points in cirlce radius 4, we have
    // 0.0 0.780360637414 1.53073250366 2.22227927729 2.82842524837 3.32587660639 3.69551660682 3.92314021565 4.0
    // or for 4 points, we have (dx,dy) = (0.00,4.00), (1.53,3.70), (2.83,2.83), (3.70,1.53)
    // or as integer offsets : (0,4), (2,4), (3,3), (4,2)
    // or for 8 points, we have (dx,dy) = (0.00,4.00), (0.78,0.392), (1.53,3.70), (2.22,3.33), (2.83,2.83), etc
    // or as integer offsets: (0,4), (1,4), (2,4), (2,3), (3,3), (3,2), (4,2), (4,1)
    static int offsets[32][2] = { {4,0}, {4,1}, {4,2}, {3,2}, {3,3}, {2,3}, {2,4}, {1,4},
                                  {0,4}, {-1,4}, {-2,4}, {-2,3}, {-3,3}, {-3,2}, {-4,2}, {-4,1},
                                  {-4,0}, {-4,-1}, {-4,-2}, {-3,-2}, {-3,-3}, {-2,-3}, {-2,-4}, {-1,-4},
                                  {0,-4}, {1,-4}, {2,-4}, {2,-3}, {3,-3}, {3,-2}, {4,-2}, {4,-1} };
    for (int y=parameters.perimeter; y<h-parameters.perimeter; y++) {
        for (int x=parameters.perimeter; x<w-parameters.perimeter; x++) {
            t_point_value best_pv_of_pt;
            float value;
            best_pv_of_pt.value = 0;
            for (int a=0; a<16; a++) {
                float vecx, vecy;
                int dx, dy;
                vecx = 0;
                vecy = 0;
                dx = offsets[a*2][0];
                dy = offsets[a*2][1];
                for (int i=0; i<4; i++) {
                    int xi, yi;
                    int tmp;
                    xi = x + dx;
                    yi = y + dy;
                    vecx += raw_img[i][(yi*w+xi)*4+0]; // From red
                    vecy += raw_img[i][(yi*w+xi)*4+1]; // From green
                    tmp = dy;
                    dy = dx;
                    dx = -tmp;
                }
                value = vecx*vecx+vecy*vecy;
                if (value <= elements_minimum) continue;
                if (value>best_pv_of_pt.value) {
                    best_pv_of_pt.value = value;
                    best_pv_of_pt.vec_x = vecx;
                    best_pv_of_pt.vec_y = vecy;
                    best_pv_of_pt.extra[0] = a;
                    best_pv_of_pt.extra[1] = dx;
                    best_pv_of_pt.extra[2] = dy;
                }
            }
            int i;
            if (best_pv_of_pt.value <= elements_minimum) continue;
            if (best_pv_of_pt.value < parameters.minimum) continue;
            for (i=0; i<n; i++) {
                if (points[i].value < best_pv_of_pt.value) break;
            }
            if (n==parameters.max_elements) n--;
            if (i<n) {
                memmove(&points[i+1], &points[i], sizeof(t_point_value)*(n-i));
            }
            n++;
            points[i].x=x;
            points[i].y=y;
            points[i].value = best_pv_of_pt.value;
            points[i].vec_x = best_pv_of_pt.vec_x;
            points[i].vec_y = best_pv_of_pt.vec_y;
            points[i].extra[0] = best_pv_of_pt.extra[0];
            points[i].extra[1] = best_pv_of_pt.extra[1];
            points[i].extra[2] = best_pv_of_pt.extra[2];
            if (n==parameters.max_elements) {
                elements_minimum = points[n-1].value;
            }
        }
    }
    float min_distance_sq;
    min_distance_sq = parameters.min_distance * parameters.min_distance;
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
    for (int i=0; (i<n) && (i<20); i++) {
        fprintf(stderr,"%d: (%d,%d) = %f : %5.2f (%8.5f, %8.5f) : %d %d %d\n", i, points[i].x, points[i].y, points[i].value,
                -360*atan2(points[i].vec_y, points[i].vec_x)/2/PI,
                points[i].vec_x, points[i].vec_y,
                points[i].extra[0], points[i].extra[1], points[i].extra[2] );
    }
    if (ec->points) free(ec->points);
    ec->points = points;
    ec->num_points = n;
    return 0;
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
    t_len_string textures;
    t_len_string parameter_string;

    if ((sscanf(optarg,"%*[a-z]%n:%*[a-zA-Z0-9_.]%n(%*[0-9,]%n%c",
                &filter_type.len,
                &filename.len,
                &textures.len,
                ch)!=1) || (ch[0]!=')')) {
        fprintf(stderr, "Failed to parse filter string '%s'\n", optarg);
        return NULL;
    }
    filter_type.ptr      = optarg;
    filename.ptr         = optarg + filter_type.len+1;
    textures.ptr         = optarg + filename.len+1;
    parameter_string.ptr = optarg + textures.len+1;
    filename.len         = textures.ptr-filename.ptr-1;
    textures.len         = parameter_string.ptr-textures.ptr-1;
    parameter_string.len = strlen(parameter_string.ptr);
    if (!strncmp(filter_type.ptr, "glsl", 4)) {
        return new c_filter_glsl(&filename, &textures, &parameter_string);
    } else if (!strncmp(filter_type.ptr, "find", 4)) {
        return new c_filter_find(&filename, &textures, &parameter_string);
    } else if (!strncmp(filter_type.ptr, "fndf", 4)) {
        return new c_filter_find4(&filename, &textures, &parameter_string);
    } else if (!strncmp(filter_type.ptr, "corr", 4)) {
        return new c_filter_correlate(&filename, &textures, &parameter_string);
    } else if (!strncmp(filter_type.ptr, "save", 4)) {
        return new c_filter_save(&filename, &textures, &parameter_string);
    }
    fprintf(stderr, "Failed to parse filter string '%s' - bad filter type probably\n", optarg);
    return NULL;
}

