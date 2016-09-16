#define STR(x) #x
#define STRINGIFY(x) STR(x)
#define GL_GET_ERRORS do {} while (0);
#define VERBOSE
#ifdef VERBOSE
#undef GL_GET_ERRORS
#define GL_GET_ERRORS do { \
        gl_get_errors( __FILE__  STRINGIFY(__LINE__) ); \
    } while (0);
#endif

/*f gl_get_errors
 */
extern int gl_get_errors(const char *msg);

/*a Types
 */
/*t t_texture_ptr
 */
typedef struct t_texture *t_texture_ptr;

/*t t_texture_header
 */
typedef struct
{
    int width;
    int height;
} t_texture_header;

/*f texture_header
 */
inline t_texture_header *texture_header(t_texture_ptr texture) { return (t_texture_header *)texture; }

/*f texture_target_as_framebuffer
 */
extern int
texture_target_as_framebuffer(t_texture_ptr texture);

/*f texture_attach_to_shader
 */
extern void
texture_attach_to_shader(t_texture_ptr texture, int shader_number, GLint t_u);

/*f texture_draw_init
 */
extern void
texture_draw_init(void);

/*f texture_draw_prepare
 */
extern void
texture_draw_prepare(void);

/*f texture_draw_rectangle
 */
extern void
texture_draw_rectangle(void);

/*f texture_draw_tidy
 */
extern void
texture_draw_tidy(void);

/*f texture_draw
 */
extern void
texture_draw(void);

/*f texture_get_buffer
 */
extern void *
texture_get_buffer(t_texture_ptr t_texture);

/*f texture_save
 */
extern int
texture_save(t_texture_ptr texture, const char *png_filename);

/*f texture_load
 */
extern t_texture_ptr 
texture_load(const char *image_filename, GLuint image_type);

/*f texture_create
 */
extern t_texture_ptr 
texture_create(GLuint format, int width, int height);

