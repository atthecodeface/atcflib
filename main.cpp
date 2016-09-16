/*a Documentation
Example
./prog -i images/IMG_1664.JPG --filter='glsl:intensity_from_rgb(0,1)' --filter='glsl:gauss_x9(1,2)' --filter='glsl:gauss_y9(2,1)' --filter='glsl:harris(1,2)' --filter='find:a(2)' --filter='corr:correlation_copy_shader(0,3)'
 */
/*a Includes
 */
#define GL_GLEXT_PROTOTYPES
#define GLM_FORCE_RADIANS
#include <SDL.h> 
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "key_value.h"
#include "texture.h"

/*a Defines
 */

/*a Types
 */
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
    t_texture *textures[16];
    t_point_value *points;
    int num_points;
} t_exec_context;

/*t t_len_string
 */
typedef struct
{
    const char *ptr;
    int len;
} t_len_string;

/*t c_main
 */
class c_main
{
public:
    c_main(void);
    ~c_main();
    void check_sdl_error(void);
    int init(void);
    void exit(void);
    int create_window(void);

    SDL_Window    *window;
    SDL_GLContext glContext;
};

/*a Helper functions
 */
/*f gl_get_errors
 */
int gl_get_errors(const char *msg)
{
    GLenum err;
    int num_errors;
    num_errors = 0;
    while ((err = glGetError()) != GL_NO_ERROR) {
        fprintf(stderr,"OpenGL error %s : %d\n", msg, err);
        num_errors++;
    }
    return num_errors;
}

/*a Shader methods
 */
/*f shader_load
 */
const char *file_read(const char *filename)
{
    FILE *f;
    size_t file_length;
    char *ptr;

    f = fopen(filename,"r");
    if (!f) {
        fprintf(stderr, "Failed to open file '%s'\n",filename);
        return NULL;
    }
    fseek(f, 0L, SEEK_END);
    file_length = ftell(f);
    rewind(f);
    ptr = (char *)malloc(file_length+1);
    fread(ptr,1,file_length,f);
    fclose(f);
    ptr[file_length]=0;
    return ptr;
}
static const char *shader_base_functions_code;
GLuint shader_load(const char *shader_filename, GLenum shader_type, const char *shader_defines)
{
    static const char *shader_base_functions_filename="shaders/base_functions.glsl";
    const char *shader_code_files[4];
    const char *shader_code;
    GLuint shader_id;
    GLint compile_result;

    if (!shader_base_functions_code) {
        shader_base_functions_code = file_read(shader_base_functions_filename);
        if (!shader_base_functions_code) return 0;
    }

    shader_code = file_read(shader_filename);
    if (!shader_code) return 0;

    shader_id = glCreateShader(shader_type);
    shader_code_files[0] = "#version 330\n";
    shader_code_files[1] = "";
    if (shader_defines) 
        shader_code_files[1] = shader_defines;
    shader_code_files[2] = shader_base_functions_code;
    shader_code_files[3] = shader_code;
    glShaderSource(shader_id, 4, shader_code_files, NULL);
    glCompileShader(shader_id);
    free((void *)shader_code);

    glGetShaderiv(shader_id,GL_COMPILE_STATUS, &compile_result);
    if (compile_result==GL_FALSE) {
        char error_buf[256];
        //int line=1;
        glGetShaderInfoLog(shader_id, sizeof(error_buf), NULL, error_buf);
        fprintf(stderr," Failed to compile shader '%s'\n%s\n", shader_filename, error_buf);
        for (int i=0; i<4; i++) {
        }
        return 0;
    }

    return shader_id;
}

/*f shader_load_and_link
 */
GLuint shader_load_and_link(GLuint program_id, const char *vertex_shader, const char *fragment_shader, const char *shader_defines)
{
    GLuint vertex_shader_id;
    GLuint fragment_shader_id;
    GLint link_result;

    if (program_id==0) {
        program_id = glCreateProgram();
    }
    if ((vertex_shader_id=shader_load(vertex_shader, GL_VERTEX_SHADER, shader_defines))==0) {
        return 0;
    }
    if ((fragment_shader_id=shader_load(fragment_shader, GL_FRAGMENT_SHADER, shader_defines))==0) {
        return 0;
    }

    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);

    glGetProgramiv(program_id, GL_LINK_STATUS, &link_result);
    if (link_result==GL_FALSE) {
        char error_buf[256];
        glGetShaderInfoLog(program_id, sizeof(error_buf), NULL, error_buf);
        fprintf(stderr," Failure\n%s\n",error_buf);
        return 0;
    }

    glDetachShader(program_id, vertex_shader_id);
    glDetachShader(program_id, fragment_shader_id);

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    return program_id;
}

/*a Filters
 */
/*t c_filter
 */
class c_filter
{
public:
    c_filter(void);
    ~c_filter();
    void set_filename(const char *dirname, const char *suffix, t_len_string *filename, char **filter_filename);
    void set_uniforms(t_len_string *uniforms);
    void get_shader_defines(char **shader_defines);
    int  get_shader_uniform_ids(GLuint filter_pid);
    int  get_value_from_key_value(t_key_value_entry_ptr kve);
    virtual int compile(void) {return 0;};
    virtual int execute(t_exec_context *ec) {return 0;};
    const char *parse_error;
    t_key_values uniform_key_values;
};

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
    GLuint filter_pid;
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
    GLuint filter_pid;
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
int c_filter::get_shader_uniform_ids(GLuint filter_pid)
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
    if (get_shader_uniform_ids(filter_pid))
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
        fprintf(stderr,"Got %d.%d.%d.%d\n", uniform_texture_src_id,
                uniform_texture_base_id,
                uniform_texture_base_x,
                uniform_texture_base_y);
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

    fprintf(stderr,"Use %d.%d.%d.%d\n", uniform_texture_src_id,
                uniform_texture_base_id,
                uniform_texture_base_x,
                uniform_texture_base_y);
    if (uniform_texture_base_id>=0) {
        GL_GET_ERRORS;

        texture_attach_to_shader(ec->textures[texture_base], 1, uniform_texture_base_id);
        if (uniform_texture_base_x>=0) {glUniform1f(uniform_texture_base_x,40*2);}
        if (uniform_texture_base_y>=0) {glUniform1f(uniform_texture_base_y,0);}
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
    if (get_shader_uniform_ids(filter_pid))
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
    for (int i=0; i<n; i++) {
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

/*f filter_from_string
  filter string must be:
  <filter type>:<filename>(<options list>)[&<uniform_name>=<float>]
 */
static c_filter *filter_from_string(const char *optarg)
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

/*a Main methods
 */
/*f c_main constructor
 */
c_main::c_main(void)
{
    window = NULL;
}

/*f c_main::check_sdl_error
 */
void
c_main::check_sdl_error(void)
{
	const char *error;
    error = SDL_GetError();
	if (*error != '\0') {
		fprintf(stderr,"SDL Error: %s\n", error);
		SDL_ClearError();
	}
}

/*f c_main::init
*/
int c_main::init(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        return 0;
    }

    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    check_sdl_error();

    fprintf(stderr,"Main Init() completed\n");
    return 1;
}

/*f c_main::exit
*/
void
c_main::exit(void)
{
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}

/*f c_main::create_window
 */
int
c_main::create_window()
{
    int flags;
    int width;
    int height;
    width = 64;
    height = 64;

    flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("OpenGL Test",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,width, height, flags);
    check_sdl_error();

    if (!window) {
        return 0;
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        return 0;
    }

    if (1) {
        int major, minor;
        SDL_GL_GetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, &major );
        SDL_GL_GetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, &minor );
        check_sdl_error();
        fprintf(stderr, "Using OpenGL version %d.%d\n",major,minor);
    }
    return 1;
}

/*a Options
 */
/*v long_options
*/
static struct option long_options[] =
{
    {"filter",   required_argument, 0, 'f'},
    {"infile",   required_argument, 0, 'i'},
    {"textures", required_argument, 0, 'n'},
    {0, 0, 0, 0}
};

/*t t_option_list
*/
typedef struct
{
    int num;
    const char *strings[256];
} t_option_list;

/*t t_options 
*/
typedef struct
{
    int num_textures;
    t_option_list images;
    t_option_list filters;
} t_options;

/*f option_add_to_list
*/
static void option_add_to_list(t_option_list *option_list, const char *string)
{
    option_list->strings[option_list->num++] = string;
}

/*f get_options
*/
static int get_options(int argc, char **argv, t_options *options)
{
    int c;
    options->num_textures = 0;
    options->images.num=0;
    options->filters.num=0;
    while (1)
    {
        int option_index = 0;

        c = getopt_long (argc, argv, "f:i:n:",
                         long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'f':
            option_add_to_list(&options->filters, optarg);
            break;
        case 'i':
            option_add_to_list(&options->images, optarg);
            break;
        case 'n':
            options->num_textures = atoi(optarg);
            break;
        default:
            break;
        }
    }
    return 1;
}

/*a Toplevel
*/
/*f main
 */
int main(int argc,char *argv[])
{
    int i;
    c_main *m;
    c_filter *filters[256];
    t_options options;
    t_exec_context ec;

    m = new c_main();
    if (m->init()==0) {
        fprintf(stderr,"Initializtion failed\n");
        return 4;
    }
    if (!m->create_window()) {
        fprintf(stderr,"Create window failed\n");
        return 4;
    }

    if (get_options(argc, argv, &options)==0) {
        return 4;
    }

    for (int i=0; i<options.filters.num; i++) {
        filters[i] = filter_from_string(options.filters.strings[i]);
    }

    int failures=0;
    for (int i=0; i<options.filters.num; i++) {
        if (!filters[i]) {
            failures++;
            continue;
        }
        if (filters[i]->parse_error) {
            fprintf(stderr, "Filter '%s' parse error: %s\n", options.filters.strings[i], filters[i]->parse_error);
            failures++;
            continue;
        }
        if (filters[i]->compile()!=0) failures++;
    }
    if (failures>0) {
        exit(4);
    }

    texture_draw_init();
    for (i=0; i<options.images.num; i++) {
        ec.textures[i] = texture_load(options.images.strings[i], GL_RGB);
    }
    for (i=options.images.num; i<options.images.num+options.num_textures; i++) {
        ec.textures[i] = texture_create(GL_R32F, 1024, 1024);
    }
    fprintf(stderr,"Made %d textures\n", i);
    ec.points = NULL;

    for (int i=0; i<options.filters.num; i++) {
        fprintf(stderr, "Execute '%s'\n", options.filters.strings[i]);
        filters[i]->execute(&ec);
    }

    m->exit();
    return 0;
}
