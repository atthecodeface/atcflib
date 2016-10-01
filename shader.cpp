/*a Documentation
 */
/*a Includes
 */
#include <OpenGL/gl3.h>
#include <stdlib.h>
#include <stdio.h>
#include "shader.h"

/*a Types
 */

/*a Static methods
 */
/*f file_read
 */
static const char *
file_read(const char *filename)
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

/*a External functions
 */
static const char *shader_base_functions_filename="shaders/base_functions.glsl";
static const char *shader_base_functions_code;
extern int
shader_init(void)
{
    if (!shader_base_functions_code) {
        shader_base_functions_code = file_read(shader_base_functions_filename);
        if (!shader_base_functions_code) return 1;
    }
    return 0;
}

/*f shader_load
 */
GLuint
shader_load(const char *shader_filename, GLenum shader_type, const char *shader_defines)
{
    const char *shader_code_files[4];
    const char *shader_code;
    GLuint shader_id;
    GLint compile_result;

    shader_code = file_read(shader_filename);
    if (!shader_code) return 0;

    shader_id = glCreateShader(shader_type);
    shader_code_files[0] = "#version 330\n";
    shader_code_files[1] = "";
    if (shader_defines) 
        shader_code_files[1] = shader_defines;
    shader_code_files[2] = shader_base_functions_code;
    shader_code_files[3] = shader_code;
    if (0) {
        for (int i=0; i<4; i++) {
            fprintf(stderr,"%s\n",shader_code_files[i]);
        }
    }
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
GLuint
shader_load_and_link(GLuint program_id, const char *vertex_shader, const char *fragment_shader, const char *shader_defines)
{
    GLuint vertex_shader_id;
    GLuint fragment_shader_id;
    GLint link_result;

    if (shader_init()!=0)
        return 0;
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


/*f shader_delete
 */
void
shader_delete(GLuint program_id)
{
    if (program_id!=0) {
        glDeleteShader(program_id);
    }
}
