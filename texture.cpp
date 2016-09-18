/*a Documentation
 */
/*a Includes
 */
#define GL_GLEXT_PROTOTYPES
#define GLM_FORCE_RADIANS
#include <SDL.h> 
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdlib.h>
#include "texture.h"

/*a Types
 */
/*t t_texture
 */
typedef struct t_texture
{
    t_texture_header hdr;
    GLuint gl_id;
    GLuint format;

    void *raw_buffer;
} t_texture;

/*a Static functions
 */
/*f texture_buffers
 * Create buffers etc for the texture
 */
static void
texture_buffers(t_texture *texture)
{
    texture->raw_buffer = malloc(texture->hdr.width * texture->hdr.height * 4*sizeof(float));
}

/*a External functions
 */
/*f texture_save
 */
int
texture_save(t_texture_ptr texture, const char *png_filename)
{
    SDL_Surface *image;
    unsigned char *image_pixels;
    int ret;

    image = SDL_CreateRGBSurface(0, texture->hdr.width, texture->hdr.height,32,0,0,0,0);
    image_pixels = (unsigned char*)image->pixels;

    glBindTexture(GL_TEXTURE_2D, texture->gl_id);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if (0) {
        float *raw_img;
        raw_img = (float*)texture->raw_buffer;
        glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, texture->raw_buffer);
        for (int j=0;j<texture->hdr.height;j++){
            for (int i=0;i<texture->hdr.width; i++){
                image_pixels[4*(j*texture->hdr.width+i)+0]=255*raw_img[j*texture->hdr.width+i];            
                image_pixels[4*(j*texture->hdr.width+i)+1]=255*raw_img[j*texture->hdr.width+i];            
                image_pixels[4*(j*texture->hdr.width+i)+2]=255*raw_img[j*texture->hdr.width+i];            
                image_pixels[4*(j*texture->hdr.width+i)+3]=255*raw_img[j*texture->hdr.width+i];
            }
        }
    } else {
        float *raw_img = (float *)texture->raw_buffer;
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, texture->raw_buffer);
        for (int j=0;j<texture->hdr.height;j++){
            for (int i=0;i<texture->hdr.width; i++){
                int p_in = (j*texture->hdr.width+i)*4;
                int p_out = (j*texture->hdr.width+i)*4;
                image_pixels[p_out+0] = raw_img[p_in+1];
                image_pixels[p_out+1] = raw_img[p_in+2];
                image_pixels[p_out+2] = raw_img[p_in+3];
                image_pixels[p_out+3] = 1;
            }
        }
    }
    ret = IMG_SavePNG(image, png_filename);
    free(image);
    return ret;
}

/*f texture_load
 */
t_texture_ptr 
texture_load(const char *image_filename, GLuint image_type)
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
    texture->hdr.width  = surface->w;
    texture->hdr.height = surface->h;

    //Generate an OpenGL texture to return
    glGenTextures(1,&texture->gl_id);
    glBindTexture(GL_TEXTURE_2D, texture->gl_id);

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
t_texture_ptr 
texture_create(GLuint format, int width, int height)
{
    t_texture *texture;

    texture = (t_texture *)malloc(sizeof(t_texture));
    texture->hdr.width = width;
    texture->hdr.height = height;

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
int
texture_target_as_framebuffer(t_texture_ptr texture)
{
    GL_GET_ERRORS;
    if (frame_buffer==0) {
        glGenFramebuffers(1, &frame_buffer);
    }
    glBindFramebuffer( GL_FRAMEBUFFER, frame_buffer );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->gl_id, 0);

    glViewport(0, 0, texture->hdr.width, texture->hdr.height);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        fprintf(stderr,"The frame buffer is not working\n");
    }

    GL_GET_ERRORS;
    return 1;
}

/*f texture_draw_init
 */
static GLuint texture_draw_buffers[2];
void
texture_draw_init(void)
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

/*f texture_attach_to_shader
 */
void
texture_attach_to_shader(t_texture_ptr texture, int shader, GLint t_u)
{
    GL_GET_ERRORS;
    glActiveTexture(GL_TEXTURE0+shader);
    glBindTexture(GL_TEXTURE_2D, texture->gl_id);
    glUniform1i(t_u, shader);
    GL_GET_ERRORS;
}

/*f texture_draw_prepare
 */
void
texture_draw_prepare(void)
{
    GL_GET_ERRORS;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, texture_draw_buffers[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, texture_draw_buffers[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    GL_GET_ERRORS;
}

/*f texture_draw_rectangle
 */
void
texture_draw_rectangle(void)
{
    GL_GET_ERRORS;
    glDrawArrays(GL_TRIANGLES,0,6);
    GL_GET_ERRORS;
}

/*f texture_draw_tidy
 */
void texture_draw_tidy(void)
{
    GL_GET_ERRORS;

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D,0);
    glActiveTexture(GL_TEXTURE0);
}

/*f texture_draw
 */
void
texture_draw(void)
{
    GL_GET_ERRORS;

    texture_draw_prepare();
    texture_draw_rectangle();
    texture_draw_tidy();

    GL_GET_ERRORS;
}


/*f texture_get_buffer
 */
void *
texture_get_buffer(t_texture_ptr texture)
{
    glBindTexture(GL_TEXTURE_2D, texture->gl_id);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, texture->raw_buffer);
    return texture->raw_buffer;
}
