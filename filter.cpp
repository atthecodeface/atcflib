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

/*t t_filter_glsl_parameters
 */
typedef struct
{
    int num_x_divisions;
    int num_y_divisions;
} t_filter_glsl_parameters;

/*t c_filter_glsl
 */
class c_filter_glsl : public c_filter
{
    static t_parameter_def parameter_defns[];
public:
    c_filter_glsl(t_len_string *filename, t_len_string *textures, t_len_string *parameter_string);
    ~c_filter_glsl();
    char *filter_filename;
    char *shader_defines;
    t_filter_glsl_parameters parameters;

    virtual int do_compile(void);
    virtual int do_execute(t_exec_context *ec);
};

/*v c_filter_glsl::parameter_defns
 */
t_parameter_def c_filter_glsl::parameter_defns[] = {
        {"num_x_divisions", 'i', offsetof(t_filter_glsl_parameters,num_x_divisions)},
        {"num_y_divisions", 'i', offsetof(t_filter_glsl_parameters,num_y_divisions)},
        {NULL, 0, 0}
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
    double minimum;
    double min_distance;
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

/*a c_filter constructor and destructor methods
 */
/*f c_filter constructor
 */
c_filter::c_filter(t_len_string *textures, t_len_string *parameter_string)
{
    int texture_ec_ids[MAX_FILTER_TEXTURES];

    parse_error = NULL;

    for (int i=0; i<MAX_FILTER_TIMERS; i++) {
        SL_TIMER_INIT(timers[i]);
    }
    filter_pid = 0;
    for (int i=0; i<MAX_FILTER_TEXTURES; i++) {
        this->textures[i].texture = NULL;
        this->textures[i].sampler_id = -1;
        this->textures[i].ec_id = -1;
    }
    num_projections = 0;
    for (int i=0; i<MAX_FILTER_PROJECTIONS; i++) {
        this->projections[i] = NULL;
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
            value_string=std::string(equals+1,value_end-equals-1);
        } else {
            key_string  =std::string(ptr,value_end-ptr);
            value_string=std::string("");
        }
        t_filter_parameter &fp = (*parameter_map)[key_string];
        fp.valid_values = fp_valid_string;
        fp.string = (const char *)malloc(strlen(value_string.c_str()+1));
        strcpy((char *)fp.string, value_string.c_str());
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

/*f c_filter::set_parameters_from_map
 */
void c_filter::set_parameters_from_map(t_parameter_def *parameter_defns, void *parameters)
{
    for (int i=0; parameter_defns[i].name; i++) {
        // change to find the parameter by name - then use default
        for (auto fpi = parameter_map->begin(); fpi != parameter_map->end(); ++fpi) {
            if (!fpi->first.compare(parameter_defns[i].name)) {
                get_parameter_value(&(fpi->second));
                char *p = ((char *)parameters) + parameter_defns[i].this_offset;
                if (parameter_defns[i].type=='i') {
                    ((int *)p)[0] = fpi->second.integer;
                }
                if (parameter_defns[i].type=='f') {
                    ((double *)p)[0] = fpi->second.real;
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

/*f c_filter::bind_projection
 */
int c_filter::bind_projection(int n, class c_lens_projection *projection)
{
    if ((n>=0) && (n<=MAX_FILTER_PROJECTIONS)) {
        projections[n] = projection;
        return 0;
    }
    return -1;
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
    parameters.num_x_divisions = 2;
    parameters.num_y_divisions = 2;
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
    int rc=0;
    SL_TIMER_ENTRY(timers[filter_timer_compile]);

    get_shader_defines(&shader_defines);
    filter_pid = shader_load_and_link(0, "shaders/vertex_shader.glsl", filter_filename, shader_defines);
    if (filter_pid==0) {
        parse_error = "Failed to load and link shader";
        rc = 1;
    }
    if ((rc==0) && get_shader_uniform_ids()) {
        parse_error = "Failed to get shader uniform ids";
        rc = 1;
    }
    if ((rc==0) && get_texture_uniform_ids(1)) {
        parse_error = "Failed to get texture uniform ids";
        rc = 1;
    }
    SL_TIMER_EXIT(timers[filter_timer_compile]);
    return rc;
}

/*f c_filter_glsl::do_execute
 */
int c_filter_glsl::do_execute(t_exec_context *ec)
{
    GL_GET_ERRORS;

    SL_TIMER_ENTRY(timers[filter_timer_execute]);

    set_parameters_from_map(parameter_defns, (void *)&parameters);

    texture_target_as_framebuffer(bound_texture(ec,num_textures-1));
    glUseProgram(filter_pid);

    set_shader_uniforms();
    set_texture_uniforms(ec, 1);

    if (projections[0] && projections[1]) {
        texture_draw_through_projections(projections,
                                         parameters.num_x_divisions,
                                         parameters.num_y_divisions);
    } else {
        texture_draw();
    }

    GL_GET_ERRORS;

    SL_TIMER_EXIT(timers[filter_timer_execute]);
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
    int rc=0;

    SL_TIMER_ENTRY(timers[filter_timer_compile]);

    get_shader_defines(&shader_defines);
    filter_pid = shader_load_and_link(0, "shaders/vertex_correlation_shader.glsl", filter_filename, shader_defines);
    if (filter_pid==0) {
        rc=1;
    }
    if ((rc==0) && (get_shader_uniform_ids())){
        rc=1;
    }
    if ((rc==0) && (get_texture_uniform_ids(1))) {
        rc=1;
    }

    SL_TIMER_EXIT(timers[filter_timer_compile]);

    return rc;
}

/*f c_filter_correlate::do_execute
 */
int c_filter_correlate::do_execute(t_exec_context *ec)
{
    GL_GET_ERRORS;
    SL_TIMER_ENTRY(timers[filter_timer_execute]);

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

    SL_TIMER_EXIT(timers[filter_timer_execute]);

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

    set_parameters_from_map(parameter_defns, (void *)&parameters);
    texture = bound_texture(ec, 0);

    if (0) {
        fprintf(stderr, "Saving to '%s'\n",save_filename);
    }
    return texture_save(texture, save_filename, parameters.components, parameters.conversion);
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
typedef struct
{
    double value_xy;
    int x;
    int place;
} t_xy_value_place;

int cmp_xy_value_place(const void *a, const void *b)
{
    const t_xy_value_place *pa, *pb;
    pa = (const t_xy_value_place *)a;
    pb = (const t_xy_value_place *)b;
    double diff = pa->value_xy - pb->value_xy;
    if (diff<0) return -1;
    return 1;
}

int c_filter_find::do_execute(t_exec_context *ec)
{
    t_texture *texture;
    const t_texture_header *texture_hdr;
    float *raw_img;
    float elements_minimum;
    int   n;
    int w, h;
    t_xy_value_place *h_points;

    SL_TIMER_ENTRY(timers[filter_timer_execute]);

    set_parameters_from_map(parameter_defns, (void *)&parameters);

    texture = bound_texture(ec, 0);

    texture_hdr = texture_header(texture);
    raw_img = (float *)texture_get_buffer(texture, GL_RGBA);
    w = texture_hdr->width;
    h = texture_hdr->height;

    elements_minimum = -1.0;

    SL_TIMER_ENTRY(timers[filter_timer_internal_1]);

    if (elements_minimum<parameters.minimum) elements_minimum=parameters.minimum;
    n=0;
    points   = (t_point_value *)malloc(sizeof(t_point_value)*parameters.max_elements);
    h_points = (t_xy_value_place *)malloc(sizeof(t_xy_value_place)*w);
    for (int y=parameters.perimeter; y<h-parameters.perimeter; y++) {
        int num_h_points = 0;
        for (int x=parameters.perimeter; x<w-parameters.perimeter; x++) {
            float value_xy;
            value_xy = raw_img[(y*w+x)*4+0];
            if (value_xy<=elements_minimum) continue;
            h_points[num_h_points].x        = x;
            h_points[num_h_points].value_xy = value_xy;
            num_h_points++;
        }
        if (num_h_points==0)
            continue;
        if (num_h_points>1) {
            qsort(h_points, num_h_points, sizeof(t_xy_value_place), cmp_xy_value_place);
        }
        //fprintf(stderr,"%d:Merging %d points, total %d so far\n",y,num_h_points,n);
        int i, j;
        j=0;
        for (i=0; (i<n) && (j<num_h_points);) {
            if (points[i].value<h_points[j].value_xy) {
                i++;
            }
            if (i+j>=parameters.max_elements) break;
            h_points[j].place = i+j;
            j++;
        }
        while ((j<num_h_points) && (n+j<parameters.max_elements)) {
            h_points[j].place = n+j;
            j++;
        }
        // At this point, h_points[0 .. j-1].place is the index that h_points must end up at
        // So look at h_points[j-1]  = k1. Move elements k1-j .. n-1 down by j, and set element k from h_points[j-1]
        // Now look at h_points[j-2] = k2. Move elements k2 .. k1-1 down by j-1, and set element k2 from h_points[j-2]
        // Hence track 'bottom of range'=l, starting at l=n-j, and start with j=j-1
        // while j>=0:
        //   k = h_points[j].place
        //   move elements k-j..l-1 inclusive down by j+1
        //   set element[k] from h_points[j]
        //   j--, l=k
        // In visual: h_points = 2, 6 (CG); n=12 (ABDEFHIJKLMN); j=2; l=12-2=10; j=j-1=11
        // j=1, k=6, ABDEFHIJKLMN -> ABDEF__HIJKL -> ABDEF_GHIJKL, l=k, j=j-1=0
        //    move 5=k-j to 10=l (inc to exc) down by 2=j+1, set [6=k] = G, l=k-j=5, j=j-1=0
        // j=0, k=2, ABDEF_GHIJKLMN -> AB_DEFGHIJKL -> ABCDEFGHIJKL
        //    move 2=k-j to 5=l (inc to exc) down by 1=j+1, set [2=k] = C
        // if we don't start full (i.e. n<parameters.max_elements) then start
        // with l = min(n, parameters.max_elements-j)
        // in the above if we had started with n=8 and max of 12, then l=8
        // in the above if we had started with n=9 and max of 12, then l=9
        // in the above if we had started with n=10 and max of 12, then l=10
        // in the above if we had started with n=11 and max of 12, then l=10
        int l = parameters.max_elements-j;
        if (l>n) l=n;
        n = l+j;
        j--;
         while (j>=0) {
            int k = h_points[j].place;
            if (k-j < l) {
                memmove(&points[k+1], &points[k-j], sizeof(t_point_value)*(l-(k-j)));
            }
            int x = h_points[j].x;
            points[k].x     = x;
            points[k].y     = y;
            points[k].value = h_points[j].value_xy;
            points[k].vec_x = raw_img[(y*w+x)*4+1];
            points[k].vec_y = raw_img[(y*w+x)*4+2];
            l=k-j;
            j--;
         }
         elements_minimum = points[n-1].value;
    }

    SL_TIMER_EXIT(timers[filter_timer_internal_1]);
    SL_TIMER_ENTRY(timers[filter_timer_internal_2]);

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
    SL_TIMER_EXIT(timers[filter_timer_internal_2]);

    if (0) {
        for (int i=0; (i<n) && (i<10); i++) {
            fprintf(stderr,"%d: (%d,%d) = %f : %5.2f (%8.5f, %8.5f)\n",
                    i, points[i].x, points[i].y, points[i].value,
                    180/M_PI*atan2(points[i].vec_y, points[i].vec_x),
                    points[i].vec_x, points[i].vec_y
                );
        }
    }
    if (ec->points) free(ec->points);
    ec->points = points;
    ec->num_points = n;

    SL_TIMER_EXIT(timers[filter_timer_execute]);

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
    } else if (!strncmp(filter_type.ptr, "corr", 4)) {
        return new c_filter_correlate(&filename, &textures, &parameter_string);
    } else if (!strncmp(filter_type.ptr, "save", 4)) {
        return new c_filter_save(&filename, &textures, &parameter_string);
    }
    fprintf(stderr, "Failed to parse filter string '%s' - bad filter type probably\n", optarg);
    return NULL;
}

