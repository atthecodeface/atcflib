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

/*a Defines
 */
#define STR(x) #x
#define STRINGIFY(x) STR(x)
#define GL_GET_ERRORS do {} while (0);
#ifdef VERBOSE
#undef GL_GET_ERRORS
#define GL_GET_ERRORS do { \
        gl_get_errors( __FILE__  STRINGIFY(__LINE__) ); \
    } while (0);
#endif

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

/*t t_texture
 */
typedef struct
{
    GLuint gl_id;
    int width;
    int height;
    GLuint format;

    void *raw_buffer;
} t_texture;

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

/*f key_value_parse
 */
typedef struct t_key_value_entry
{
    struct t_key_value_entry *next;
    char *key;
    char *value;
    void *data;
} t_key_value_entry;
typedef struct
{
    t_key_value_entry *kv_head;
    t_key_value_entry *kv_tail;
    size_t extra_room;
} t_key_values;
static void key_value_add(t_key_values *kv, const char *key, int key_len, const char *value, int value_len)
{
    void *ptr;
    t_key_value_entry *kve;
    char *text_ptr;
    int kve_len;
    kve_len = sizeof(t_key_value_entry)+kv->extra_room;
    //fprintf(stderr,"add key value %s:%d %s:%d\n",key,key_len,value,value_len);
    ptr = malloc(kve_len + key_len+1 + value_len+1);
    kve = (t_key_value_entry *)ptr;
    text_ptr = ((char *)kve+kve_len);
    kve->key = text_ptr;
    strncpy(kve->key, key, key_len);
    kve->key[key_len]=0;
    text_ptr = text_ptr+key_len+1;
    kve->value = text_ptr;
    strncpy(kve->value, value, value_len);
    kve->value[value_len]=0;

    if (kv->kv_tail) {
        kv->kv_tail->next = kve;
    } else {
        kv->kv_head = kve;
    }
    kv->kv_tail = kve;
    kve->next = NULL;
}
static const char *key_value_parse(const char *string, const char *string_end, t_key_values *kv)
{
    const char *end;
    const char *equals;

    while (string[0]=='&') string++;
    end = strchr(string,'&');
    if (!end) {
        end = string_end;
    }
    if (end>string_end) end=string_end;
    equals = strchr(string,'=');
    if (equals>end) equals=NULL;
    if (!equals) {
        key_value_add(kv, string, end-string, NULL, 0);
    } else {
        key_value_add(kv, string, equals-string, equals+1, end-equals-1);
    }
    return end;
}
static t_key_value_entry *key_value_iter(t_key_values *kv, t_key_value_entry *kve)
{
    if (kve)
        return kve->next;
    return kv->kv_head;
}
static void key_value_init(t_key_values *kv, size_t extra_room)
{
    kv->kv_head = NULL;
    kv->kv_tail = NULL;
    kv->extra_room = extra_room;
}

/*f key_value_find
 */
static t_key_value_entry *key_value_find(const char *key)
{
    return NULL;
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
        int line=1;
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

/*a Texture functions
 */
/*f texture_save
 */
int texture_save(t_texture *texture, const char *png_filename)
{
    SDL_Surface *image;
    unsigned char *image_pixels;
    int ret;

    image = SDL_CreateRGBSurface(0, texture->width, texture->height,32,0,0,0,0);
    image_pixels = (unsigned char*)image->pixels;

    glBindTexture(GL_TEXTURE_2D, texture->gl_id);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if (0) {
        float *raw_img;
        raw_img = (float*)texture->raw_buffer;
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, texture->raw_buffer);
        for (int j=0;j<texture->height;j++){
            for (int i=0;i<texture->width; i++){
                image_pixels[4*(j*texture->width+i)+0]=255*raw_img[j*texture->width+i];            
                image_pixels[4*(j*texture->width+i)+1]=255*raw_img[j*texture->width+i];            
                image_pixels[4*(j*texture->width+i)+2]=255*raw_img[j*texture->width+i];            
                image_pixels[4*(j*texture->width+i)+3]=255*raw_img[j*texture->width+i];
            }
        }
    } else {
        char *raw_img = (char *)texture->raw_buffer;
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texture->raw_buffer);
        for (int j=0;j<texture->height;j++){
            for (int i=0;i<texture->width; i++){
                int p_in = (j*texture->width+i)*4;
                int p_out = (j*texture->width+i)*4;
                image_pixels[p_out+0]=raw_img[p_in+1];
                image_pixels[p_out+1]=raw_img[p_in+2];
                image_pixels[p_out+2]=raw_img[p_in+3];
                image_pixels[p_out+3]=1;
            }
        }
    }
    ret = IMG_SavePNG(image, png_filename);
    free(image);
    return ret;
}

/*f texture_buffers
 */
static void texture_buffers(t_texture *texture)
{
    texture->raw_buffer = malloc(texture->width * texture->height * 8); //sizeof(float));
}

/*f texture_load
 */
t_texture *texture_load(const char *image_filename, GLuint image_type)
{
    SDL_Surface *surface, *image_surface;
    SDL_PixelFormat sdl_pixel_format;
    t_texture *texture;

    texture = (t_texture *)malloc(sizeof(t_texture));

    fprintf(stderr,"Attempting image load from %s...\n",image_filename);

    image_surface=IMG_Load(image_filename);
    if (image_surface==NULL) {
        fprintf(stderr, "Failure to load image\n%s\n", SDL_GetError());
        return NULL;
    }

    sdl_pixel_format.palette = NULL;
    sdl_pixel_format.format = SDL_PIXELFORMAT_RGB888;
    sdl_pixel_format.BitsPerPixel = 24;
    sdl_pixel_format.BytesPerPixel = 8;
    sdl_pixel_format.Rmask=0x0000ff;
    sdl_pixel_format.Gmask=0x00ff00;
    sdl_pixel_format.Bmask=0xff0000;
    surface = SDL_ConvertSurface(image_surface, &sdl_pixel_format, 0 );
    if (surface==NULL) {
        fprintf(stderr, " Failure to convert image:\n%s\n", SDL_GetError());
        return 0;
    }
    texture->width  = surface->w;
    texture->height = surface->h;

    //Generate an OpenGL texture to return
    glGenTextures(1,&texture->gl_id);
    glBindTexture(GL_TEXTURE_2D, texture->gl_id);
    fprintf(stderr,"%s",SDL_GetError());

    //glPixelStorei(GL_UNPACK_ALIGNMENT,4);	
    //glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,16/*surface->w*/,16/*surface->h*/,0,GL_RGB,GL_UNSIGNED_BYTE,surface->pixels);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,surface->w,surface->h,0,GL_RGB,GL_UNSIGNED_BYTE,surface->pixels);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    SDL_FreeSurface(surface);
    SDL_FreeSurface(image_surface);

    texture_buffers(texture);
    return texture;
}

/*f texture_create
 */
t_texture *texture_create(GLuint format, int width, int height)
{
    t_texture *texture;

    texture = (t_texture *)malloc(sizeof(t_texture));
    texture->width = width;
    texture->height = height;

    glGenTextures(1, &texture->gl_id);
    glBindTexture(GL_TEXTURE_2D, texture->gl_id);

    glTexImage2D(GL_TEXTURE_2D, 0,
                 format, width, height, 0, // Texture is RGB with this width and height
                 GL_RGB, GL_UNSIGNED_BYTE, NULL); // data source type - NULL means no initial data

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
    texture_buffers(texture);
    return texture;
}

/*f texture_target_as_framebuffer
 */
static GLuint frame_buffer=0;
int texture_target_as_framebuffer(t_texture *texture)
{
    GL_GET_ERRORS;
    if (frame_buffer==0) {
        glGenFramebuffers(1, &frame_buffer);
    }
    glBindFramebuffer( GL_FRAMEBUFFER, frame_buffer ); //Tell OpenGL to render to the depth map from now on
    //glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->gl_id, 0);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->gl_id, 0);

    glViewport(0, 0, texture->width, texture->height);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr,"The frame buffer is not working\n");
    }

    GL_GET_ERRORS;
    return 1;
}

/*t texture_draw_init
 */
static GLuint texture_draw_buffers[2];
static void texture_draw_init(void)
{
    float vertices[3*2*3];
    float uvs[2*2*3];

    vertices[0*3+0] = 1.0f;
    vertices[0*3+1] = 0.0f;
    vertices[0*3+2] = 0.0f; 
    vertices[1*3+0] = 0.0f;
    vertices[1*3+1] = 1.0f;
    vertices[1*3+2] = 0.0f; 
    vertices[2*3+0] = 0.0f;
    vertices[2*3+1] = 0.0f;
    vertices[2*3+2] = 0.0f; 
    vertices[3*3+0] = 1.0f;
    vertices[3*3+1] = 0.0f;
    vertices[3*3+2] = 0.0f; 
    vertices[4*3+0] = 0.0f;
    vertices[4*3+1] = 1.0f;
    vertices[4*3+2] = 0.0f; 
    vertices[5*3+0] = 1.0f;
    vertices[5*3+1] = 1.0f;
    vertices[5*3+2] = 0.0f; 

    uvs[0*2+0] = 1.0f;
    uvs[0*2+1] = 0.0f;
    uvs[1*2+0] = 0.0f;
    uvs[1*2+1] = 1.0f;
    uvs[2*2+0] = 0.0f;
    uvs[2*2+1] = 0.0f;
    uvs[3*2+0] = 1.0f;
    uvs[3*2+1] = 0.0f;
    uvs[4*2+0] = 0.0f;
    uvs[4*2+1] = 1.0f;
    uvs[5*2+0] = 1.0f;
    uvs[5*2+1] = 1.0f;

    GL_GET_ERRORS;

    GLuint VertexArrayID;
    glGenVertexArrays(1,&VertexArrayID);
    glBindVertexArray(VertexArrayID);

    glGenBuffers(2, texture_draw_buffers);
    glBindBuffer(GL_ARRAY_BUFFER, texture_draw_buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, texture_draw_buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    GL_GET_ERRORS;
}

/*t texture_draw_prepare
 */
static void texture_draw_prepare(t_texture *texture, GLuint t_u)
{
    gl_get_errors("draw_prepare in");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture->gl_id);
    glUniform1i(t_u,0);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, texture_draw_buffers[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, texture_draw_buffers[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    GL_GET_ERRORS;
}

/*t texture_draw_do
 */
static void texture_draw_do(void)
{
    GL_GET_ERRORS;

    glDrawArrays(GL_TRIANGLES,0,6);

    GL_GET_ERRORS;
}

/*t texture_draw_tidy
 */
static void texture_draw_tidy(void)
{
    GL_GET_ERRORS;

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    GL_GET_ERRORS;
}

/*t texture_draw
 */
static void texture_draw(t_texture *texture, GLuint t_u)
{
    GL_GET_ERRORS;

    texture_draw_prepare(texture, t_u);
    texture_draw_do();
    texture_draw_tidy();

    GL_GET_ERRORS;
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
    int  get_value_from_key_value(t_key_value_entry *kve);
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
    GLuint uniform_texture_src_id;
    GLuint uniform_texture_base_id;
    GLuint uniform_texture_base_x;
    GLuint uniform_texture_base_y;
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
    GLuint uniform_texture_src_id;
    GLuint uniform_out_xy_id;
    GLuint uniform_out_size_id;
    GLuint uniform_src_xy_id;
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
    t_key_value_entry *kve;
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
int c_filter::get_value_from_key_value(t_key_value_entry *kve)
{
    t_filter_key_value_data *kvd;
    int value_len;
    kvd = (t_filter_key_value_data *)(&kve->data);
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
    t_key_value_entry *kve;
    int failures;
    failures = 0;
    kve = key_value_iter(&uniform_key_values, NULL);
    while (kve) {
        if (!strncmp("-U", kve->key, 2)) {
            t_filter_key_value_data *kvd;
            kvd = (t_filter_key_value_data *)(&kve->data);
            kvd->gl_id = glGetUniformLocation(filter_pid, kve->key+2);
            if (kvd->gl_id<=0) {
                fprintf(stderr, "Failed to find uniform '%s' in shader\n", kve->key+2);
                failures++;
            }
            if (get_value_from_key_value(kve)) {
                fprintf(stderr, "Failed to parse uniform value '%s' in shader\n", kve->value);
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
    uniform_texture_base_id = 0;
    uniform_texture_base_x = 0;
    uniform_texture_base_y = 0;
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

    if (uniform_texture_base_id!=0) {
        GL_GET_ERRORS;

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, ec->textures[texture_base]->gl_id);
        glUniform1i(uniform_texture_base_id, 1);
        if (uniform_texture_base_x) {glUniform1f(uniform_texture_base_x,(15.5+32*2)/1024.0);}
        if (uniform_texture_base_y) {glUniform1f(uniform_texture_base_y,15.5/1024.0);}
        GL_GET_ERRORS;
    }

    GL_GET_ERRORS;

    texture_draw(ec->textures[texture_src], uniform_texture_src_id);

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
    texture_target_as_framebuffer(ec->textures[texture_dest]);

    glClearColor(0.1,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(filter_pid);
    glUniform2f(uniform_out_size_id,32,32);
    texture_draw_prepare(ec->textures[texture_src], uniform_texture_src_id);

    float xsc, ysc;
    xsc = 1024.0 / (ec->textures[texture_src]->width);
    ysc = 1024.0 / (ec->textures[texture_src]->height);
    xsc = 1.0;
    ysc = 1.0;
    for (int i=0; i<ec->num_points; i++) {
        glUniform2f(uniform_out_xy_id,i*32,0);
        glUniform2f(uniform_src_xy_id,(ec->points[i].x-16)*xsc,(ec->points[i].y-16)*ysc);
        texture_draw_do();
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
    max_elements = 512;
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

    texture = ec->textures[texture_src];
    raw_img = (float *)texture->raw_buffer;
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, texture->raw_buffer);

    elements_minimum = -1.0;
    n=0;
    points = (t_point_value *)malloc(sizeof(t_point_value)*max_elements);
    for (int y=perimeter; y<texture->height-perimeter; y++) {
        for (int x=perimeter; x<texture->width-perimeter; x++) {
            int i;
            float value_xy;
            value_xy = raw_img[y*texture->width+x];
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
        ec.textures[i] = texture_create(GL_R16, 1024, 1024);
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
