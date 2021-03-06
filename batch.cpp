/*a Documentation
Example
 */
/*a Includes
 */
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
#define NUM_MAPPINGS 70
#define MAX_POINTS_PER_MAPPING 30

//#define NUM_MAPPINGS 30
//#define MAX_POINTS_PER_MAPPING 30

//#define NUM_MAPPINGS 10
//#define MAX_POINTS_PER_MAPPING 10

//#define NUM_MAPPINGS 5
//#define MAX_POINTS_PER_MAPPING 10

// Want to check point 2 map 0 and point 1 map 0 and why that does not seem to count
//#define NUM_MAPPINGS 3
//#define MAX_POINTS_PER_MAPPING 3

// a, b are -PI to PI.
// a-b is -2*PI, -PI, 0, PI, to 2*PI
// Want       1,   0, 1,  0,   1
// cos(a-b) is 1, -1, 1,  -1,  1
// 1+cos(a-b) is 2, 0, 2,  0,  2
#define ROTATION_DIFF_STRENGTH(a,b) ( (1+cos((a)-(b))) / 2 )
//#define FFT_ROTATION(dy,dx) (atan2(-(dy),(dx)))
//#define FFT_ROTATION(dy,dx) (atan2(-(dy),(dx)))
//#define FFT_ROTATION(dy,dx) (atan2(dy,-dx))
// after fixing base functions, try:
#define FFT_ROTATION(dy,dx) (-atan2(dy,dx))

// DIST_FACTOR used to be 2 or 4
//#define DIST_FACTOR (10.0)
#define DIST_FACTOR (2.0)

#define PI (M_PI)
#define DEG(a) (180/PI*(a))
#define RAD(a) ((a)*PI/180)

/*a Types
 */
/*t t_proposition
 */
typedef struct
{
    float translation[2];
    float rotation;
    float scale;
} t_proposition;

/*c c_mapping
  Mappings are single source->target mappings
 */
class c_mapping
{
public:
    c_mapping(class c_mapping_point *src_pt,
              class c_mapping_point *src_other,
              t_point_value *tgt_pv,
              t_point_value *tgt_other_pv,
              t_proposition *proposition, float strength);
    float map_strength(t_proposition *proposition);
    float position_map_strength(t_proposition *proposition);
    void repr(char *buffer, int buf_size);
    class c_mapping *next_in_list;
    class c_mapping_point *src_pt;
    class c_mapping_point *src_other_pt;
    t_point_value tgt_pv;
    t_point_value tgt_other_pv;
    t_proposition proposition;
    float strength;
};

/*c c_mapping_point
    A point on the source image has a set of mapping beliefs, each with a concept of 'strength'
    These can be its ideas that it maps to particular points on the target with an image rotation,
    or it can be its ideas that the mapping for an image is a rotation and scale followed by a translation (tx, ty)
    This latter is built up from pairs of points, and is purer in our application as the rotation is derived
    from the pair of points rather than the phase of the FFT (which is more of a clue than a calculation)

    The point can be asked for its most-trusted proposition that it has not been told to ignore. It may have none.
    The main algorithm can combine these propositions from all source points to come up with a 'consensus proposition'
    Each point is then asked to use the consensus proposition to produce a new most-trusted proposition (even using data it has been told to ignore I think)
    The points may have none.
    A consensus propositino is again reached - and we repeat to get a most-agreed consensus.
    At this point the main algorithm outputs the most-agreed consensus, and informs each
    point to ignore propositions that support this consensus
    Repeat from the beginning until we have enough consensus propositions or there are no propositions

*/
class c_pv_match
{
public:
    c_pv_match(t_point_value *pv);
    class c_pv_match *next_in_list;
    t_point_value pv;
    float rotation;
};
class c_mapping_point
{
public:
    c_mapping_point(float x, float y);
    void add_match(t_point_value *pv);
    void add_mapping(c_mapping *mapping);
    c_mapping *find_strongest_belief(t_proposition *proposition, float *best_strength);
    float strength_in_belief(t_proposition *proposition);
    void diminish_belief_in_proposition(t_proposition *proposition);
    void repr(char *buffer, int buf_size);
    c_pv_match *matches;
    c_mapping *mappings;
    float coords[2];
};

/*f c_mapping::c_mapping
 */
c_mapping::c_mapping(class c_mapping_point *src_pt, class c_mapping_point *src_other_pt,
                     t_point_value *tgt_pv, t_point_value *tgt_other_pv,
                     t_proposition *proposition, float strength)
{
    this->next_in_list = NULL;
    this->src_pt = src_pt;
    this->src_other_pt = src_other_pt;
    this->tgt_pv = *tgt_pv;
    this->tgt_other_pv = *tgt_other_pv;
    this->proposition = *proposition;
    this->strength = strength;
}

/*f c_mapping::map_strength
  how well does a proposition match this mapping's proposition?
  Return 0 for poor, 1 for good, sliding scale
 */
float c_mapping::map_strength(t_proposition *proposition)
{
    float strength;
    float dx, dy, translation_dist;
    t_proposition *tp;
    tp = &(this->proposition);
    strength = this->strength;
    if (proposition->scale < tp->scale) {
        strength *= proposition->scale / tp->scale;
    } else {
        strength *= tp->scale / proposition->scale;
    }
    dx = tp->translation[0] - proposition->translation[0];
    dy = tp->translation[1] - proposition->translation[1];
    translation_dist = sqrt(dx*dx + dy*dy);
    strength *= DIST_FACTOR / (DIST_FACTOR + translation_dist);
    strength *= ROTATION_DIFF_STRENGTH(tp->rotation,proposition->rotation);
    return strength;
}

/*f c_mapping::position_map_strength
  how well does a proposition match the points being mapped?
  Return 0 for poor, 1 for good, sliding scale
 */
float c_mapping::position_map_strength(t_proposition *proposition)
{
    float strength;
    float src_x, src_y;
    float cosang, sinang;
    float tgt_x, tgt_y;
    float dx, dy, dist;

    cosang = cos(proposition->rotation);
    sinang = sin(proposition->rotation);

    src_x = this->src_pt->coords[0];
    src_y = this->src_pt->coords[1];
    tgt_x = proposition->translation[0] + proposition->scale*(cosang*src_x - sinang*src_y);
    tgt_y = proposition->translation[1] + proposition->scale*(cosang*src_y + sinang*src_x);
    dx = tgt_x - this->tgt_pv.x;
    dy = tgt_y - this->tgt_pv.y;
    dist = sqrt(dx*dx+dy*dy);
    strength = this->strength * (DIST_FACTOR/(DIST_FACTOR+dist));

    if (0) { // Since many mappings may have the same 'other', it may have undue influence, so ignore it
        src_x = this->src_other_pt->coords[0];
        src_y = this->src_other_pt->coords[1];
        tgt_x = proposition->translation[0] + proposition->scale*(cosang*src_x - sinang*src_y);
        tgt_y = proposition->translation[1] + proposition->scale*(cosang*src_y + sinang*src_x);
        dx = tgt_x - this->tgt_other_pv.x;
        dy = tgt_y - this->tgt_other_pv.y;
        dist = sqrt(dx*dx+dy*dy);
        strength = this->strength * (4.0/(4.0+dist));
    }

    strength *= ROTATION_DIFF_STRENGTH(this->proposition.rotation,proposition->rotation);
    return strength;
}

/*f c_mapping::repr
 */
void c_mapping::repr(char *buffer, int buf_size)
{
    snprintf(buffer, buf_size, "(%4d,%4d) -> (%4d,%4d) : (%4d,%4d) -> (%4d,%4d) : (%8.2f,%8.2f) %6.2f %5.3f    %8.5f",
             (int) this->src_pt->coords[0],
             (int) this->src_pt->coords[1],
             (int) this->tgt_pv.x,
             (int) this->tgt_pv.y,
             (int) this->src_other_pt->coords[0],
             (int) this->src_other_pt->coords[1],
             (int) this->tgt_other_pv.x,
             (int) this->tgt_other_pv.y,
             this->proposition.translation[0],
             this->proposition.translation[1],
             DEG(this->proposition.rotation),
             this->proposition.scale,
             this->strength);
    buffer[buf_size-1] = 0;
}

/*f c_pv_match::c_pv_match
 */
c_pv_match::c_pv_match(t_point_value *pv)
{
    this->next_in_list = NULL;
    this->pv = *pv;
    this->rotation = FFT_ROTATION(pv->vec_y, pv->vec_x);
}

/*f c_mapping_point::c_mapping_point
 */
c_mapping_point::c_mapping_point(float x, float y)
{
    this->coords[0] = x;
    this->coords[1] = y;
    this->matches = NULL;
    this->mappings = NULL;
}

/*f c_mapping_point::add_match
 */
void c_mapping_point::add_match(t_point_value *pv)
{
    c_pv_match *pvm;
    pvm = new c_pv_match(pv);
    pvm->next_in_list = this->matches;
    this->matches = pvm;
}

/*f c_mapping_point::add_mapping
 */
void c_mapping_point::add_mapping(c_mapping *mapping)
{
    mapping->next_in_list = mappings;
    mappings = mapping;
}

/*f c_mapping_point::find_strongest_belief
 */
c_mapping *c_mapping_point::find_strongest_belief(t_proposition *proposition, float *best_strength)
{
    
    c_mapping *best_mapping;
    float max_strength;
    max_strength = 0;
    best_mapping = NULL;
    for (c_mapping *m=mappings; m; m=m->next_in_list) {
        float s;
        if (proposition) {
            s = m->position_map_strength(proposition);
        } else {
            s = m->strength;
        }
        if (s>max_strength) {
            max_strength = s;
            best_mapping = m;
        }
    }
    *best_strength = max_strength;
    return best_mapping;
}

/*f c_mapping_point::strength_in_belief
 */
float c_mapping_point::strength_in_belief(t_proposition *proposition)
{
    float strength = 0;
    for (c_mapping *m=mappings; m; m=m->next_in_list) {
        strength += m->position_map_strength(proposition);
    }
    return strength;
}
    
/*f c_mapping_point::diminish_belief_in_proposition
 */
void c_mapping_point::diminish_belief_in_proposition(t_proposition *proposition)
{
    for (c_mapping *m=mappings; m; m=m->next_in_list) {
        float s;
        s = m->position_map_strength(proposition);
        //fprintf(stderr,"Diminish %p %f %f\n", m, s, m->strength);
        if (s>0) {
            s /= m->strength;
            m->strength *= (1-s)*(1-s);
        }
    }
}

/*f diminish_mappings_by_proposition
 */
static void diminish_mappings_by_proposition(c_mapping_point *mappings[], int num_mappings, t_proposition *proposition)
{
    for (int p=0; p<NUM_MAPPINGS; p++) {
        if (mappings[p]) {
            mappings[p]->diminish_belief_in_proposition(proposition);
        }
    }
}

/*f strength_of_proposition
 */
static float strength_of_proposition(c_mapping_point *mappings[], int num_mappings, t_proposition *proposition)
{
    float total_strength;
    total_strength = 0;
    for (int p=0; p<num_mappings; p++) {
        if (mappings[p]) {
            float s_in_b;
            float s_in_m;
            c_mapping *m;
            s_in_b = mappings[p]->strength_in_belief(proposition);
            m = mappings[p]->find_strongest_belief(proposition, &s_in_m);
            if (m) { total_strength += s_in_m; }
            //total_strength += s_in_b;
        }
    }
    return total_strength;
}

/*f try_delta_proposition
 */
static float try_delta_proposition(c_mapping_point *mappings[], int num_mappings, float current_strength, t_proposition *proposition, t_proposition *delta, float scale)
{
    float delta_strength;
    t_proposition keep;
    keep = *proposition;
    proposition->translation[0] += scale*delta->translation[0];
    proposition->translation[1] += scale*delta->translation[1];
    proposition->rotation       += scale*delta->rotation;
    proposition->scale          += scale*delta->scale;
    delta_strength = strength_of_proposition(mappings, num_mappings, proposition);
    if (delta_strength>current_strength)
    {
        if (0) {
            fprintf(stderr,"Improved strength %8.4f from %8.4f: Translation (%8.2f,%8.2f) rotation %6.2f scale %6.4f\n",
                    delta_strength, current_strength, proposition->translation[0], proposition->translation[1], DEG(proposition->rotation), proposition->scale);
        }
        current_strength = delta_strength;
    } else {
        *proposition = keep;
    }
    return current_strength;
}

/*f tweak_proposition
 */
static float tweak_proposition(c_mapping_point *mappings[], int num_mappings, t_proposition *proposition, float scale)
{
    float total_strength;
    t_proposition delta_cp;

    total_strength = strength_of_proposition(mappings, num_mappings, proposition);

    delta_cp.translation[0] = 0;
    delta_cp.translation[1] = 0;
    delta_cp.scale = 0;
    delta_cp.rotation = 0;

    delta_cp.scale = 0.0;
    delta_cp.rotation = RAD(0.3);

    total_strength = try_delta_proposition(mappings, num_mappings, total_strength, proposition, &delta_cp,  1.0*scale );
    total_strength = try_delta_proposition(mappings, num_mappings, total_strength, proposition, &delta_cp, -1.0*scale );

    delta_cp.scale = 0.0;
    delta_cp.rotation = RAD(0.1);

    total_strength = try_delta_proposition(mappings, num_mappings, total_strength, proposition, &delta_cp,  1.0*scale );
    total_strength = try_delta_proposition(mappings, num_mappings, total_strength, proposition, &delta_cp, -1.0*scale );

    delta_cp.translation[0] = 0.1;
    delta_cp.translation[1] = 0;

    total_strength = try_delta_proposition(mappings, num_mappings, total_strength, proposition, &delta_cp,  1.0*scale );
    total_strength = try_delta_proposition(mappings, num_mappings, total_strength, proposition, &delta_cp, -1.0*scale );

    delta_cp.translation[0] = 0;
    delta_cp.translation[1] = 0.1;

    total_strength = try_delta_proposition(mappings, num_mappings, total_strength, proposition, &delta_cp,  1.0*scale );
    total_strength = try_delta_proposition(mappings, num_mappings, total_strength, proposition, &delta_cp, -1.0*scale );

    return total_strength;
}

/*f show_strength_in_mapping
 */
static void show_strength_in_mapping(c_mapping_point *mappings[], int num_mappings, t_proposition *proposition)
{
    float total_strength_in_belief, total_strength_in_max;

    total_strength_in_belief = 0;
    total_strength_in_max = 0;
    for (int p=0; p<NUM_MAPPINGS; p++) {
        if (mappings[p]) {
            float s_in_b;
            float s_in_m;
            c_mapping *m;
            s_in_b = mappings[p]->strength_in_belief(proposition);
            total_strength_in_belief += s_in_b;
            m = mappings[p]->find_strongest_belief(proposition, &s_in_m);
            if (m) {
                char buf[256];
                m->repr(buf, sizeof(buf));
                fprintf(stderr, "%f %f %s\n", s_in_b, s_in_m, buf);
                total_strength_in_max += s_in_m;
            }
        }
    }
    fprintf(stderr,"Strength %8.4f / %8.4f: Translation (%8.2f,%8.2f) rotation %6.2f scale %6.4f\n\n",
            total_strength_in_belief, total_strength_in_max,
            proposition->translation[0], proposition->translation[1], DEG(proposition->rotation), proposition->scale);
}

/*f find_best_mapping
 */
static float find_best_mapping(c_mapping_point *mappings[], int num_mappings, t_proposition *best_proposition)
{
    float best_mapping_strength;
    c_mapping *best_mapping;
    t_proposition cp;
    float total_strength;

    /*b Find best mapping to start with
     */
    best_mapping_strength = 0;
    best_mapping = NULL;
    for (int p=0; p<num_mappings; p++) {
        if (mappings[p]) {
            float strength;
            c_mapping *m = mappings[p]->find_strongest_belief(NULL, &strength);
            if (m && (strength>best_mapping_strength)) {
                best_mapping_strength = strength;
                best_mapping = m;

            }
        }
    }
    if (!best_mapping) return 0;

    /*b Starting with best mapping, iterate to improve - at least perturb from the max of a perfect match
     */
    cp = best_mapping->proposition;
    total_strength = 0;
    for (int i=0; i<1; i++) {
        t_proposition np;
        float np_dx, np_dy;
        np.translation[0] = 0;
        np.translation[1] = 0;
        np.rotation = 0;
        np_dy = 0;
        np_dx = 0;
        np.scale = 0;
        total_strength = 0;
        for (int p=0; p<NUM_MAPPINGS; p++) {
            if (mappings[p]) {
                float s_in_b;
                float s_in_m;
                c_mapping *m;
                s_in_b = mappings[p]->strength_in_belief(&cp);
                m = mappings[p]->find_strongest_belief(&cp, &s_in_m);
                if (m) {
                    float s;
                    /* s_in_m gave 
                       Strength 111.8511: Translation (  333.04, -180.99) rotation  23.75 scale 0.9963
                       s_in_b gave
                       Strength 560.7759: Translation (  331.71, -178.53) rotation  23.70 scale 0.9937
                       Wanted both to be about -4 more in the y
                    */
                    s = s_in_b;
                    np.translation[0] += s*m->proposition.translation[0];
                    np.translation[1] += s*m->proposition.translation[1];
                    np.scale     += s*m->proposition.scale;
                    np_dx        += s*cos(m->proposition.rotation);
                    np_dy        += s*sin(m->proposition.rotation);
                    total_strength += s;
                    if (0) {
                        char buf[256];
                        m->repr(buf, sizeof(buf));
                        fprintf(stderr, "%f %f %s\n", s_in_b, s_in_m, buf);
                    }
                }
            }
        }
        if (total_strength==0) return 0;
        cp.translation[0] = np.translation[0]/total_strength;
        cp.translation[1] = np.translation[1]/total_strength;
        cp.scale     = np.scale/total_strength;
        cp.rotation  = atan2(np_dy, np_dx);
    }

    if (0) {
        fprintf(stderr,"Pre-tweak strength %8.4f: Translation (%8.2f,%8.2f) rotation %6.2f scale %6.4f\n\n",
                total_strength, cp.translation[0], cp.translation[1], DEG(cp.rotation), cp.scale);
    }
    for (int i=0; i<30; i++) {
        float last_strength = total_strength;
        total_strength = tweak_proposition(mappings, num_mappings, &cp, 10.0);
        if (total_strength<=last_strength) break;
    }
    for (int i=0; i<30; i++) {
        float last_strength = total_strength;
        total_strength = tweak_proposition(mappings, num_mappings, &cp, 1.0);
        if (total_strength<=last_strength) break;
    }
    for (int i=0; i<30; i++) {
        float last_strength = total_strength;
        total_strength = tweak_proposition(mappings, num_mappings, &cp, 0.1);
        if (total_strength<=last_strength) break;
    }
    if (0) {
        show_strength_in_mapping(mappings, num_mappings, &cp);
    }

    *best_proposition = cp;
    return total_strength;

}

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
    {"infile", required_argument, 0, 'i'},
    {"orient", required_argument, 0, 'o'},
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
    int orientation;
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
        case 'o':
            options->orientation = atoi(optarg);
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
    ec.use_ids = 1;

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
    int match_filter_start;
    num_filters = 0;
    if ((options.orientation&1)==0) {
        filters[num_filters++] = filter_from_string("glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0");
        //filters[num_filters++] = filter_from_string("glsl:yuv_from_rgb(0,2)&-DINTENSITY_XSCALE=(0.5*3456.0/5184.0)&-DINTENSITY_XOFS=0.1&-DINTENSITY_YSCALE=0.5&-DINTENSITY_YOFS=-0.1");
    } else {
        filters[num_filters++] = filter_from_string("glsl:yuv_from_rgb(0,2)&-DINTENSITY_YSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_XSCALE=1.0&-DINTENSITY_YOFS=0.0");
        //filters[num_filters++] = filter_from_string("glsl:yuv_from_rgb(0,2)&-DINTENSITY_YSCALE=(0.5*3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_XSCALE=0.5&-DINTENSITY_YOFS=0.0");
    }
    if ((options.orientation&2)==0) {
        filters[num_filters++] = filter_from_string("glsl:yuv_from_rgb(1,3)&-DINTENSITY_XSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_YSCALE=1.0&-DINTENSITY_YOFS=0.0");
        //filters[num_filters++] = filter_from_string("glsl:yuv_from_rgb(1,3)&-DINTENSITY_XSCALE=(0.5*3456.0/5184.0)&-DINTENSITY_XOFS=0.05&-DINTENSITY_YSCALE=0.5&-DINTENSITY_YOFS=0.0");
    } else {
        filters[num_filters++] = filter_from_string("glsl:yuv_from_rgb(1,3)&-DINTENSITY_YSCALE=(3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_XSCALE=1.0&-DINTENSITY_YOFS=0.0");
        //filters[num_filters++] = filter_from_string("glsl:yuv_from_rgb(1,3)&-DINTENSITY_YSCALE=(0.5*3456.0/5184.0)&-DINTENSITY_XOFS=0.0&-DINTENSITY_XSCALE=0.5&-DINTENSITY_YOFS=0.0");
    }
    filters[num_filters++] = filter_from_string("glsl:harris(2,4)&-DNUM_OFFSETS=25&-DOFFSETS=offsets_2d_25");
    filters[num_filters++] = filter_from_string("find:a(4)");
    filters[num_filters++] = filter_from_string("glsl:circle_dft(2,5)&-DDFT_CIRCLE_RADIUS=8&-DCIRCLE_COMPONENT=g");
    filters[num_filters++] = filter_from_string("glsl:circle_dft(3,6)&-DDFT_CIRCLE_RADIUS=8&-DCIRCLE_COMPONENT=g");
    filters[num_filters++] = filter_from_string("save:test_a.png(2)");
    filters[num_filters++] = filter_from_string("save:test_b.png(3)");
    filters[num_filters++] = filter_from_string("save:test_h.png(4)");
    init_filter_end = num_filters;
    match_filter_start = num_filters;
    filters[num_filters++] = filter_from_string("glsl:circle_dft_diff(5,6,7)");
    filters[num_filters++] = filter_from_string("glsl:circle_dft_diff(5,6,8)");
    filters[num_filters++] = filter_from_string("glsl:circle_dft_diff(5,6,9)");
    filters[num_filters++] = filter_from_string("glsl:circle_dft_diff(5,6,10)");
    filters[num_filters++] = filter_from_string("glsl:circle_dft_diff_combine(7,8,9,10,11)&-DDISCRETE_CIRCLE_OFS=discrete_circle_offsets_4_32&-DNUM_OFFSETS=32");
    //filters[num_filters++] = filter_from_string("save:test_e.png(10)");
    //filters[num_filters++] = filter_from_string("fndf:a(7,8,9,10)");
    //filters[num_filters++] = filter_from_string("save:test_d7.png(7)");
    //filters[num_filters++] = filter_from_string("save:test_d8.png(8)");
    //filters[num_filters++] = filter_from_string("save:test_d9.png(9)");
    //filters[num_filters++] = filter_from_string("save:test_d10.png(10)");
    //filters[num_filters++] = filter_from_string("save:test_d11.png(11)");
    filters[num_filters++] = filter_from_string("find:a(11)&min_distance=2.5&max_elements=100&minimum=0.1");

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
        ec.textures[i] = texture_create(1024, 1024);
    }
    fprintf(stderr,"Made %d textures\n", i);
    ec.points = NULL;

    for (int i=0; i<init_filter_end; i++) {
        filters[i]->execute(&ec);
    }

    t_point_value pts[NUM_MAPPINGS*4];
    for (int i=0, j=0; (i<ec.num_points) && (i<NUM_MAPPINGS); i++) {
        pts[j++] = ec.points[i];
        pts[j++] = ec.points[i];
        pts[j++] = ec.points[i];
        pts[j++] = ec.points[i];
    }
    for (int i=0; i<NUM_MAPPINGS*4; i+=4) {
        pts[i+0].x += 4; // 4 is magic to match c_filter_find4
        pts[i+1].y += 4;
        pts[i+2].x -= 4;
        pts[i+3].y -= 4;
    }

    c_mapping_point *mappings[NUM_MAPPINGS];

    for (int p=0; p<NUM_MAPPINGS; p++) {
        mappings[p] = NULL;
    }

    for (int p=0; p<NUM_MAPPINGS; p++)
    {
        int f;
        mappings[p] = new c_mapping_point(pts[p*4].x-4,pts[p*4].y);

        fprintf(stderr,"Point %d (%d,%d)\n",p,pts[p*4].x-4,pts[p*4].y);
        f = match_filter_start;
        for (int i=0; i<4; i++) {
            filters[f]->uniform_set("uv_base_x",pts[p*4+i].x);
            filters[f]->uniform_set("uv_base_y",pts[p*4+i].y);
            filters[f]->execute(&ec);
            f++;
        }
        for (int i=match_filter_start+4; i<num_filters; i++) {
            filters[i]->execute(&ec);
        }

        for (int i=0; (i<ec.num_points) && (i<MAX_POINTS_PER_MAPPING); i++) {
            t_point_value *pv;
            float fft_rotation;
            pv = &(ec.points[i]);
            fft_rotation = FFT_ROTATION(pv->vec_y,pv->vec_x);
            mappings[p]->add_match(pv);

            for (int pp=0; pp<p; pp++) {
                c_mapping_point *pm = mappings[pp];

                for (c_pv_match *pmpv=pm->matches; pmpv; pmpv=pmpv->next_in_list) {
                    float tgt_dx, tgt_dy, tgt_l;
                    float src_dx, src_dy, src_l;
                    float scale, rotation;
                    float cos_rot_diff;

                    cos_rot_diff = cos(fft_rotation - pmpv->rotation);
                    if (cos_rot_diff<0.90) {
                        if (0) {
                            fprintf(stderr, "Dropping (%d,%d)->(%d,%d) and (%d,%d)->(%d,%d) as rotations mismatch (%f, %f, %f)\n",
                                    (int)mappings[p]->coords[0], (int)mappings[p]->coords[1], pv->x, pv->y,
                                    (int)pm->coords[0], (int)pm->coords[1], pmpv->pv.x, pmpv->pv.y,
                                    DEG(fft_rotation), DEG(pmpv->rotation), cos_rot_diff );
                        }
                        continue;
                    }
                    if (0) {
                        fprintf(stderr, "Not initially dropping (%d,%d)->(%d,%d) and (%d,%d)->(%d,%d) as rotations don't mismatch too much (%f, %f, %f)\n",
                                (int)mappings[p]->coords[0], (int)mappings[p]->coords[1], pv->x, pv->y,
                                (int)pm->coords[0], (int)pm->coords[1], pmpv->pv.x, pmpv->pv.y,
                                DEG(fft_rotation), DEG(pmpv->rotation), cos_rot_diff );
                    }

                    tgt_dx = pv->x - pmpv->pv.x;
                    tgt_dy = pv->y - pmpv->pv.y;
                    src_dx = mappings[p]->coords[0] - pm->coords[0];
                    src_dy = mappings[p]->coords[1] - pm->coords[1];

                    tgt_l = sqrt(tgt_dx*tgt_dx + tgt_dy*tgt_dy);
                    src_l = sqrt(src_dx*src_dx + src_dy*src_dy);

                    scale = tgt_l / src_l;

                    rotation = atan2(tgt_dy, tgt_dx) - atan2(src_dy, src_dx);
                    rotation = (rotation<-PI) ? (rotation+2*PI) : rotation;
                    rotation = (rotation>PI) ? (rotation-2*PI) : rotation;

                    /* There is a mapping src -> tgt that is tgt = scale*rotation*src + translation
                       Hence (m.tgt - pm.tgt) = scale * rotation * (m.src-pm.src)
                       or tgt_dxy = scale*rotation*(src_dxy)
                       Hence (tgt.dxy).(src.dxy) = (scale*rotation*(src.dxy)) . (src.dxy)
                       Hence (tgt.dxy).(src.dxy) = scale*cos(rotation)*((src.dxy).(src.dxy))
                       Hence scale*cos(rotation) = (tgt.dxy).(src.dxy) / (src_l^2)
                       Hence scale*cos(rotation) = (tgt_l * src_l * cosang) / (src_l^2)
                       Hence scale*cos(rotation) = tgt_l/src_l * cosang
                       Hence cos(rotation) should match cosang, and scale is tgt_l/src_l
                       Note that if two angles are approximately equal, then cos(diff) is near 1.0
                       angdiff should be about rotation
                    */
                    cos_rot_diff = cos(fft_rotation - rotation);

                    if (0) {
                        fprintf(stderr,"Checking\n");
                        fprintf(stderr," (%d,%d) -> (%d,%d)  and   (%d,%d) -> (%d,%d)\n",
                                (int)mappings[p]->coords[0], (int)mappings[p]->coords[1], pv->x, pv->y,
                                (int)pm->coords[0], (int)pm->coords[1], pmpv->pv.x, pmpv->pv.y);
                        fprintf(stderr,"   src_dxy (%f,%f) tgt_dxy (%f,%f) src_l %f tgt_l %f\n",
                                src_dx, src_dy, tgt_dx, tgt_dy, src_l, tgt_l );
                        fprintf(stderr,"   scale %f rotation %f fft_rotation %f pm_fft_rotation %f\n",
                                scale, DEG(rotation), DEG(fft_rotation), DEG(pmpv->rotation) );
                        fprintf(stderr,"   cos_rot_diff %f\n",
                                cos_rot_diff );
                    }
                    // Losing (cos_rot_diff>0.90) &&
                    if ( (cos_rot_diff>0.90) && (scale>0.95) && (scale<1.05)) {
                        float cos_rot, sin_rot;
                        float translation_x, translation_y;

                        if (0) {
                            fprintf(stderr,"Adding\n");
                            fprintf(stderr," (%d,%d) -> (%d,%d)  and   (%d,%d) -> (%d,%d)\n",
                                    (int)mappings[p]->coords[0], (int)mappings[p]->coords[1], pv->x, pv->y,
                                    (int)pm->coords[0], (int)pm->coords[1], pmpv->pv.x, pmpv->pv.y);
                            fprintf(stderr,"   src_dxy (%f,%f) tgt_dxy (%f,%f) src_l %f tgt_l %f\n",
                                    src_dx, src_dy, tgt_dx, tgt_dy, src_l, tgt_l );
                            fprintf(stderr,"   scale %f rotation %f fft_rotation %f pm_fft_rotation %f\n",
                                    scale, DEG(rotation), DEG(fft_rotation), DEG(pmpv->rotation) );
                            fprintf(stderr,"   cos_rot_diff %f\n",
                                    cos_rot_diff );
                        }

                        /*
                          tgt_pt = scale*rotation*src_pt + translation
                          Hence translation = tgt_pt - scale*rotation*src_pt
                        */
                        cos_rot = cos(rotation);
                        sin_rot = sin(rotation);
                        translation_x = pv->x - scale*(cos_rot*mappings[p]->coords[0] - sin_rot*mappings[p]->coords[1]);
                        translation_y = pv->y - scale*(cos_rot*mappings[p]->coords[1] + sin_rot*mappings[p]->coords[0]);
                        /* following is redundant, as expected
                        translation_x += pmpv->pv.x - scale*(cos_rot*pm->coords[0] - sin_rot*pm->coords[1]);
                        translation_y += pmpv->pv.y - scale*(cos_rot*pm->coords[1] + sin_rot*pm->coords[0]);
                        translation_x /= 2;
                        translation_y /= 2;
                        */

                        t_proposition proposition;
                        proposition.translation[0] = translation_x;
                        proposition.translation[1] = translation_y;
                        proposition.rotation = rotation;
                        proposition.scale = scale;
                        float strength;
                        strength = 100*pv->value*pmpv->pv.value;
                        mappings[p]->add_mapping( new c_mapping(mappings[p], pm, pv, &(pmpv->pv), &proposition, strength ) );
                    }
                }
            }
        }
    }

    if (0) {
        fprintf(stderr,"********************************************************************************\n");
        t_proposition prop;
        prop.translation[0]=168;
        prop.translation[1]=-126;
        prop.scale=1;
        prop.rotation = DEG(-13.0);
        for (int p=0; p<NUM_MAPPINGS; p++) {
            fprintf(stderr,"Mapping point %p\n",mappings[p]);
            if (mappings[p]) {
                char buf[256];
                float s;
                for (c_mapping *m = mappings[p]->mappings; m; m=m->next_in_list) {
                    m->repr(buf,sizeof(buf));
                    s = m->position_map_strength(&prop);
                    fprintf(stderr, "%f: %s\n", s, buf);
                }
            }
        }
        fprintf(stderr,"********************************************************************************\n");
    }

    if (0) {
        for (int p=0; p<NUM_MAPPINGS; p++) {
            fprintf(stderr,"Mapping point %p\n",mappings[p]);
            if (mappings[p]) {
                char buf[256];
                float strength;
                c_mapping *m = mappings[p]->find_strongest_belief(NULL, &strength);
                fprintf(stderr,"  Mapping %p %f\n",m,strength);
                if (m) {
                    m->repr(buf,sizeof(buf));
                    fprintf(stderr, "%f, %s\n", strength, buf);
                }
            }
        }
        fprintf(stderr,"********************************************************************************\n");
    }

    for (int i=0; i<10; i++) {
        t_proposition best_proposition;
        float strength;
        strength = find_best_mapping(mappings, NUM_MAPPINGS, &best_proposition);
        if (strength==0) break;
        fprintf(stderr,"Best proposition %d\nStrength %8.4f: Translation (%8.2f,%8.2f) rotation %6.2f scale %6.4f\n",
                i,
                strength,
                best_proposition.translation[0], best_proposition.translation[1],
                DEG(best_proposition.rotation), best_proposition.scale );
        fprintf(stderr,"Gimp: scale %8.4f; rotate about center %8.4f; offset (%8.2f,%8.2f)\n\n",
                best_proposition.scale, DEG(best_proposition.rotation),
                best_proposition.translation[0]-512 + 512*best_proposition.scale*(cos(best_proposition.rotation)-sin(best_proposition.rotation)),
                best_proposition.translation[1]-512 + 512*best_proposition.scale*(cos(best_proposition.rotation)+sin(best_proposition.rotation)) );
        diminish_mappings_by_proposition(mappings, NUM_MAPPINGS, &best_proposition);
    }

    m->exit();
    return 0;
}

