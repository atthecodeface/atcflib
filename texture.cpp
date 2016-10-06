/*a Documentation
 */
/*a Includes
 */
#include <OpenGL/gl3.h>
#include <stdlib.h>
#include <stdio.h>
#include "lens_projection.h"
#include "texture.h"
#include "image_io.h"

/*a Types
 */
/*t t_texture
 */
typedef struct t_texture
{
    t_texture_header hdr;
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
texture_save(t_texture_ptr texture, const char *png_filename, int components, int conversion)
{
    unsigned char *image_pixels;
    int width, height;
    int ret;

    width = texture->hdr.width;
    height = texture->hdr.height;
    image_pixels = (unsigned char*)malloc(height*width*4*sizeof(unsigned char));

    glBindTexture(GL_TEXTURE_2D, texture->hdr.gl_id);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    if (conversion==0) {
        float *raw_img = (float *)texture->raw_buffer;
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, texture->raw_buffer);
        for (int j=0; j<height; j++){
            for (int i=0; i<width; i++){
                int p_in = (j*width+i)*4;
                int p_out = (j*width+i)*4;
                image_pixels[p_out+0] = 255.9*raw_img[p_in+0];
                image_pixels[p_out+1] = 255.9*raw_img[p_in+1];
                image_pixels[p_out+2] = 255.9*raw_img[p_in+2];
                image_pixels[p_out+3] = 255;//255.9*raw_img[p_in+3];
            }
        }
    } else {
        unsigned int*raw_img = (unsigned int *)texture->raw_buffer;
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, texture->raw_buffer);
        for (int j=0; j<height; j++){
            for (int i=0; i<width; i++){
                int p_in = (j*width+i)*4+components;
                int p_out = (j*width+i)*4;
                image_pixels[p_out+0] = (raw_img[p_in+0]>>0)&0xff; // B
                image_pixels[p_out+1] = (raw_img[p_in+0]>>16)&0xff; // G
                image_pixels[p_out+2] = (raw_img[p_in+0]>>24)&0xff; // R
                image_pixels[p_out+3] = 255;//(raw_img[p_in+0]>>8)&0xff; // A
            }
        }
    }

    ret = image_write_rgba(png_filename, image_pixels, width, height);
    free(image_pixels);
    return ret;
}

/*f texture_load
 */
t_texture_ptr 
texture_load(const char *image_filename, GLuint image_type)
{
    t_texture *texture;
    unsigned char *image_pixels;

    texture = (t_texture *)malloc(sizeof(t_texture));

    image_pixels = image_read_rgba(image_filename, &texture->hdr.width, &texture->hdr.height);
    if (!image_pixels) {
        fprintf(stderr,"Failed to read image file '%s'\n", image_filename);
        return NULL;
    }

    //Generate an OpenGL texture to return
    texture->hdr.gl_id = 0;
    glGenTextures(1,&texture->hdr.gl_id);
    glBindTexture(GL_TEXTURE_2D, texture->hdr.gl_id);

    //glPixelStorei(GL_UNPACK_ALIGNMENT,4);	
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,texture->hdr.width,texture->hdr.height,0,GL_RGBA,GL_UNSIGNED_BYTE,image_pixels);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    free(image_pixels);

    texture_buffers(texture);
    return texture;
}

/*f texture_create
 */
t_texture_ptr 
texture_create(int width, int height)
{
    t_texture *texture;

    texture = (t_texture *)malloc(sizeof(t_texture));
    texture->hdr.width = width;
    texture->hdr.height = height;

    texture->hdr.gl_id = 0;
    glGenTextures(1, &texture->hdr.gl_id);
    glBindTexture(GL_TEXTURE_2D, texture->hdr.gl_id);

    glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_RGBA32F, width, height, 0, // Texture is RGB with this width and height
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

    if (0) {
        fprintf(stderr,"Binding texture %d as target frame buffer\n", texture->hdr.gl_id);
    }

    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->hdr.gl_id, 0);

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
static GLuint VertexArrayID;
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
    if (!texture) {
        fprintf(stderr,"NULL texture passed to texture_attach_to_shader\n" );
        return;
    }
    if ((shader<0) || (shader>4)) {
        fprintf(stderr,"Shader number '%d' out of range in call to texture_attach_to_shader\n", shader );
        return;
    }
    if (t_u<0) {
        fprintf(stderr,"Bad uniform '%d' out of range in call to texture_attach_to_shader\n", t_u );
        return;
    }
    if (texture->hdr.gl_id==0) {
        fprintf(stderr,"Texture did not have gl_id in texture_attach_to_shader\n");
        return;
    }
    GL_GET_ERRORS;
    glActiveTexture(GL_TEXTURE0+shader);
    glBindTexture(GL_TEXTURE_2D, texture->hdr.gl_id);
    glUniform1i(t_u, shader);
    GL_GET_ERRORS;
}

/*f texture_draw_prepare
 */
void
texture_draw_prepare(void)
{
    GL_GET_ERRORS;

    glBindVertexArray(VertexArrayID);
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

/*f texture_draw_through_projections*/
void
texture_draw_through_projections(c_lens_projection *projections[2], int num_x_divisions, int num_y_divisions)
{
    float *vertices = (float *)malloc(5*sizeof(float)*(num_x_divisions+1)*(num_y_divisions+1));
    int vn = 0;
    for (int y=0; y<=num_y_divisions; y++) {
        float vy = ((float)y)/num_y_divisions;
        for (int x=0; x<=num_x_divisions; x++) {
            float vx = ((float)x)/num_x_divisions;
            vertices[5*vn+0] = vx; // x
            vertices[5*vn+1] = vy; // y
            vertices[5*vn+2] = 0.0; // z
            vertices[5*vn+3] = vx; // u
            vertices[5*vn+4] = vy; // v
        }
        vn++;
    }
    short *indices = (short *)malloc(sizeof(short)*2*(num_x_divisions+1)*num_y_divisions);
    int in=0;
    for (int y=0; y<num_y_divisions; y++) {
        for (int x=0; x<=num_x_divisions; x++) {
            indices[in++] = x + y*(num_x_divisions+1);
            indices[in++] = x + (y+1)*(num_x_divisions+1);
        }
    }

    GLuint VertexArrayID;
    GLuint vertex_buffer;
    GLuint index_buffer;
    glGenVertexArrays(1,&VertexArrayID);
    glBindVertexArray(VertexArrayID);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vn*5*sizeof(float), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, in*sizeof(unsigned short), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(0*sizeof(float)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    for (int y=0; y<num_y_divisions; y++) {
        // each row is 2*(num_x_divisions+1) indices, and 2*num_x_divisions triangles
        glDrawElements(GL_TRIANGLE_STRIP, 2*(num_x_divisions+1), GL_UNSIGNED_SHORT, NULL);
    }

    GL_GET_ERRORS;

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &index_buffer);
    glDeleteVertexArrays(1,&VertexArrayID);
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
texture_get_buffer(t_texture_ptr texture, int components)
{
    int a;
    a = GL_RGBA;
    if (components>=0) a=components;
    glBindTexture(GL_TEXTURE_2D, texture->hdr.gl_id);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, a, GL_FLOAT, texture->raw_buffer);
    return texture->raw_buffer;
}

/*f texture_get_buffer_uint
 */
void *
texture_get_buffer_uint(t_texture_ptr texture, int components)
{
    int a;
    a = GL_RGBA;
    if (components>=0) a=components;
    glBindTexture(GL_TEXTURE_2D, texture->hdr.gl_id);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, a, GL_UNSIGNED_INT, texture->raw_buffer);
    return texture->raw_buffer;
}

/*f texture_destroy
 */
void
texture_destroy(t_texture_ptr texture)
{
    if (texture->raw_buffer) {
        free(texture->raw_buffer);
        texture->raw_buffer = NULL;
    }
    if (texture->hdr.gl_id>0) {
        glDeleteTextures(1, &texture->hdr.gl_id);
        texture->hdr.gl_id = 0;
    }
}
