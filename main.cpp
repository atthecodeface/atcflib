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

/*a Types
 */
/*t t_texture
 */
typedef struct
{
    GLuint gl_id;
    int width;
    int height;
    GLuint format;
} t_texture;

/*t t_exec_context
 */
typedef struct
{
    t_texture *current_texture;
    t_texture *next_texture;
} t_exec_context;

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
GLuint shader_load(const char *shader_filename, GLenum shader_type)
{
    //Read all text
    fprintf(stderr, "Attempting shader load from %s...",shader_filename);
    FILE *f;
    size_t file_length;
    char *shader_code;
    GLuint shader_id;
    GLint compile_result;

    f = fopen(shader_filename,"r");
    fseek(f, 0L, SEEK_END);
    file_length = ftell(f);
    rewind(f);
    shader_code = (char *)malloc(file_length+1);
    fread(shader_code,1,file_length,f);
    shader_code[file_length]=0;

    shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &shader_code, NULL);
    glCompileShader(shader_id);
    free(shader_code);

    glGetShaderiv(shader_id,GL_COMPILE_STATUS, &compile_result);
    if (compile_result==GL_FALSE) {
        char error_buf[256];
        glGetShaderInfoLog(shader_id, sizeof(error_buf), NULL, error_buf);
        fprintf(stderr," Failure\n%s\n",error_buf);
        return 0;
    }
    fprintf(stderr," Success\n");
    return shader_id;
}

/*f shader_load_and_link
 */
GLuint shader_load_and_link(GLuint program_id, const char *vertex_shader, const char *fragment_shader)
{
    GLuint vertex_shader_id;
    GLuint fragment_shader_id;
    GLint link_result;

    if (program_id==0) {
        program_id = glCreateProgram();
    }
    if ((vertex_shader_id=shader_load(vertex_shader, GL_VERTEX_SHADER))==0) {
        return 0;
    }
    if ((fragment_shader_id=shader_load(fragment_shader, GL_FRAGMENT_SHADER))==0) {
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
void texture_save(t_texture *texture, const char *png_filename)
{
    SDL_Surface *image;
    unsigned char *image_pixels;

    image = SDL_CreateRGBSurface(0, texture->width, texture->height,32,0,0,0,0);
    image_pixels = (unsigned char*)image->pixels;

    glBindTexture(GL_TEXTURE_2D, texture->gl_id);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if (0) {
        float *raw_img;
        raw_img = (float*) malloc(texture->width * texture->height * sizeof(float));
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, raw_img);
        for (int j=0;j<texture->height;j++){
            for (int i=0;i<texture->width; i++){
                image_pixels[4*(j*texture->width+i)+0]=255*raw_img[j*texture->width+i];            
                image_pixels[4*(j*texture->width+i)+1]=255*raw_img[j*texture->width+i];            
                image_pixels[4*(j*texture->width+i)+2]=255*raw_img[j*texture->width+i];            
                image_pixels[4*(j*texture->width+i)+3]=255*raw_img[j*texture->width+i];
            }
        }
        free(raw_img);
    } else {
        char *raw_img;
        raw_img = (char*) malloc(texture->width * texture->height * 8);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, raw_img);
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
        free(raw_img);
    }
    IMG_SavePNG(image, png_filename);
    free(image);
}

/*f texture_load
 */
t_texture *texture_load(const char *image_filename, GLuint image_type)
{
    SDL_Surface *surface, *image_surface;
    SDL_PixelFormat sdl_pixel_format;
    t_texture *texture;

    texture = (t_texture *)malloc(sizeof(t_texture));

    fprintf(stderr,"Attempting image load from %s...",image_filename);

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
    fprintf(stderr," Success\n");
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
    return texture;
}

/*f texture_target_as_framebuffer
 */
static GLuint frame_buffer=0;
int texture_target_as_framebuffer(t_texture *texture)
{
    gl_get_errors("texture_target_as_framebuffer");
    if (frame_buffer==0) {
        glGenFramebuffers(1, &frame_buffer);
    }
    glBindFramebuffer( GL_FRAMEBUFFER, frame_buffer ); //Tell OpenGL to render to the depth map from now on
    //glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->gl_id, 0);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->gl_id, 0);
    gl_get_errors("texture_target_as_framebuffer 2");
    glViewport(0, 0, texture->width, texture->height);
    gl_get_errors("texture_target_as_framebuffer 3");
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr,"The frame buffer is not working\n");
    }
    gl_get_errors("texture_target_as_framebuffer 4");
    return 1;
}

/*t texture_draw
 */
static void texture_draw(t_texture *texture, GLuint t_u)
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

    GLuint VertexArrayID;
    glGenVertexArrays(1,&VertexArrayID);
    glBindVertexArray(VertexArrayID);
    GLuint buffers[2];
    glGenBuffers(2,buffers);
    glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,texture->gl_id);
    glUniform1i(t_u,0);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glDrawArrays(GL_TRIANGLES,0,6);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    gl_get_errors("draw k");
}

/*a Filters
 */
/*t c_filter
 */
class c_filter
{
public:
    c_filter(const char *optarg);
    ~c_filter();
    virtual int compile(void) {return 1;};
    virtual int execute(t_exec_context *ec) {return 1;};
};

/*t c_filter_glsl
 */
class c_filter_glsl : public c_filter
{
public:
    c_filter_glsl(const char *optarg);
    const char *filter_filename;
    char *uniform_names[16];
    GLuint filter_pid;
    GLuint uniform_texture_src_id;
    GLuint uniform_ids[16];

    virtual int compile(void);
    virtual int execute(t_exec_context *ec);
};

/*f c_filter constructor
 */
c_filter::c_filter(const char *optarg)
{
    return;
}

/*f c_filter destructor
 */
c_filter::~c_filter(void)
{
    return;
}

/*f c_filter_glsl constructor
 */
c_filter_glsl::c_filter_glsl(const char *optarg) : c_filter(optarg)
{
    filter_filename = optarg;
    filter_pid = 0;
    uniform_texture_src_id = 0;
    for (int i=0; i<16; i++) {
        uniform_ids[i] = 0;
    }
}

/*f c_filter_glsl::compile
 */
int c_filter_glsl::compile(void)
{
    filter_pid = shader_load_and_link(0, "shaders/vertex_shader.glsl", filter_filename);
    if (filter_pid==0) {
        return 1;
    }
    uniform_texture_src_id = glGetUniformLocation(filter_pid, "texture_to_draw");
    return 0;
}

/*f c_filter_glsl::execute
 */
int c_filter_glsl::execute(t_exec_context *ec)
{
    t_texture *next_texture;
    next_texture = ec->next_texture;
    texture_target_as_framebuffer(next_texture);
    glUseProgram(filter_pid);
    //glClearColor(0.5,0.3,0.4,1.0);
    //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    texture_draw(ec->current_texture, uniform_texture_src_id);
    ec->next_texture = ec->current_texture;
    ec->current_texture = next_texture;
    return 0;
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
    {"filter",  required_argument, 0, 'f'},
    {"infile",  required_argument, 0, 'i'},
    {"output",  required_argument, 0, 'o'},
    {0, 0, 0, 0}
};

/*t t_options 
*/
typedef struct
{
    const char *image_input_filename;
    const char *image_output_filename;
    int num_filters;
    char *filter_filenames[256];
} t_options;

/*f add_filter
*/
static void add_filter(t_options *options, const char *filename)
{
    int i;
    int buffer_length;
    i = options->num_filters;
    buffer_length = strlen(filename)+strlen("shaders/")+10;
    options->filter_filenames[i] = (char *)malloc(buffer_length);
    snprintf(options->filter_filenames[i],buffer_length,"shaders/%s.glsl",filename);
    options->num_filters++;
}

/*f get_options
*/
static int get_options(int argc, char **argv, t_options *options)
{
    int c;
    while (1)
    {
        int option_index = 0;

        c = getopt_long (argc, argv, "f:i:o:",
                         long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'f':
            add_filter(options, optarg);
            break;
        case 'i':
            options->image_input_filename = optarg;
            break;
        case 'o':
            options->image_output_filename = optarg;
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

    c_main *m;
    c_filter *filters[256];
    t_options options;

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

    if (!options.image_input_filename)  options.image_input_filename="images/IMG_1687.JPG";
    if (!options.image_output_filename) options.image_output_filename="test.png";

    for (int i=0; i<options.num_filters; i++) {
        filters[i] = new c_filter_glsl(options.filter_filenames[i]);
    }
    int failures=0;
    for (int i=0; i<options.num_filters; i++) {
        if (filters[i]->compile()!=0) failures++;
    }
    if (failures>0) {
        exit(4);
    }

    t_exec_context ec;
    t_texture *t, *t_db[2];
    t = texture_load(options.image_input_filename, GL_RGB);
    for (int i=0; i<2; i++) {
        t_db[i] = texture_create(GL_R16, 1024, 1024);
    }
    ec.current_texture = t;
    ec.next_texture    = t_db[0];
    for (int i=0; i<options.num_filters; i++) {
        filters[i]->execute(&ec);
        if (ec.next_texture==t) {
            ec.next_texture = t_db[1];
        }
    }        
    texture_save(ec.current_texture, options.image_output_filename);

    m->exit();
    return 0;
}
