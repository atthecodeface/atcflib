/*a Documentation
Example
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
#include "shader.h"
#include "filter.h"

/*a Defines
 */

/*a Types
 */
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
    {"infile",   required_argument, 0, 'i'},
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
    t_option_list images;
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
    options->images.num=0;
    while (1)
    {
        int option_index = 0;

        c = getopt_long (argc, argv, "f:i:n:",
                         long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'i':
            option_add_to_list(&options->images, optarg);
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
    int num_filters;
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

    shader_init();

    if (get_options(argc, argv, &options)==0) {
        return 4;
    }

    int init_filter_end;
    int corr_filter_start, corr_filter_end;
    int match_filter_start;
    num_filters = 0;
    filters[num_filters++] = filter_from_string("glsl:intensity_from_rgb(0,2)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0");
    filters[num_filters++] = filter_from_string("glsl:intensity_from_rgb(1,3)&-DSELECTED_VALUE=r&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0");
    filters[num_filters++] = filter_from_string("glsl:harris(2,4)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25");
    filters[num_filters++] = filter_from_string("find:a(4)");
    init_filter_end = num_filters;
    corr_filter_start = num_filters;
    filters[num_filters++] = filter_from_string("corr:correlation_dft_circle(2,5)&-DCORR_CIRCLE_RADIUS=4");
    filters[num_filters++] = filter_from_string("corr:correlation_dft_circle(2,6)&-DCORR_CIRCLE_RADIUS=5");
    filters[num_filters++] = filter_from_string("corr:correlation_dft_circle(2,7)&-DCORR_CIRCLE_RADIUS=8");
    filters[num_filters++] = filter_from_string("corr:correlation_dft_circle(2,8)&-DCORR_CIRCLE_RADIUS=8");
    corr_filter_end = num_filters;
    match_filter_start = num_filters;
    filters[num_filters++] = filter_from_string("glsl:sq_dft_diff(7,3,11)&-DCORR_CIRCLE_RADIUS=8");
    filters[num_filters++] = filter_from_string("glsl:sum_circle(11,10)&-DCOPY_CIRCLE_RADIUS=4"); // copy radius should match point offset
    filters[num_filters++] = filter_from_string("save:test_diff.png(11)");
    filters[num_filters++] = filter_from_string("save:test_circle.png(10)");
    filters[num_filters++] = filter_from_string("find:a(11)");
    /*
    filters[num_filters++] = filter_from_string("glsl:sq_dft_diff(5,3,9)&-DCORR_CIRCLE_RADIUS=4");
    filters[num_filters++] = filter_from_string("glsl:sq_dft_diff(6,3,10)&-DCORR_CIRCLE_RADIUS=5");
    filters[num_filters++] = filter_from_string("glsl:sq_dft_diff(7,3,11)&-DCORR_CIRCLE_RADIUS=6");
    filters[num_filters++] = filter_from_string("glsl:sq_dft_diff(8,3,12)&-DCORR_CIRCLE_RADIUS=8");
    filters[num_filters++] = filter_from_string("glsl:alu_buffers(9,10,13)&-DOP=(src_a*src_b)");
    filters[num_filters++] = filter_from_string("glsl:alu_buffers(11,12,14)&-DOP=(src_a*src_b)");
    filters[num_filters++] = filter_from_string("glsl:alu_buffers(13,14,15)&-DOP=(src_a*src_b)");
    filters[num_filters++] = filter_from_string("find:a(15)");
    */

    int failures=0;
    for (int i=0; i<num_filters; i++) {
        if (!filters[i]) {
            failures++;
            continue;
        }
        if (filters[i]->parse_error) {
            fprintf(stderr, "Filter '%d' parse error: %s\n", i, filters[i]->parse_error);
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
    fprintf(stderr,"Read %d images\n", i);
    for (i=options.images.num; i<16; i++) {
        ec.textures[i] = texture_create(GL_R32F, 1024, 1024);
    }
    fprintf(stderr,"Made %d textures\n", i);
    ec.points = NULL;

    for (int i=0; i<init_filter_end; i++) {
        filters[i]->execute(&ec);
    }
    t_point_value pts[80];
    for (int i=0, j=0; (i<ec.num_points) && (i<20); i++) {
        pts[j++] = ec.points[i];
        pts[j++] = ec.points[i];
        pts[j++] = ec.points[i];
        pts[j++] = ec.points[i];
    }
    for (int i=0; i<80; i+=4) {
        pts[i+0].x -= 4;
        pts[i+1].x += 4;
        pts[i+2].y -= 4;
        pts[i+3].y += 4;
    }
    for (int i=0; (i<ec.num_points) && (i<80); i++) {
        ec.points[i] = pts[i];
    }
    for (int i=corr_filter_start; i<corr_filter_end; i++) {
        filters[i]->execute(&ec);
    }
    for (int p=0; p<3; p++)
    {
        fprintf(stderr,"Point %d (%d,%d)\n",p,pts[p].x,pts[p].y);
        for (int i=match_filter_start; i<num_filters; i++) {
            filters[i]->uniform_set("uv_base_x",p*40);//pts[p].x);
            filters[i]->uniform_set("uv_base_y",0);//pts[p].y);
            filters[i]->execute(&ec);
        }
    }

    m->exit();
    return 0;
}
