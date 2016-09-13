#define STEP (1.0/1024)

#ifndef NUM_WEIGHTS
#define NUM_WEIGHTS 9
#endif

#ifndef NUM_OFFSETS
#define NUM_OFFSETS 9
#endif

#ifndef NUM_CIRCLE_STEPS
#define NUM_CIRCLE_STEPS 8
#endif

#ifndef CIRCLE_RADIUS
#define CIRCLE_RADIUS 1
#endif

const vec2 gauss_offset_weights9[9] = vec2[](
vec2(-4*STEP, 0.000229f),
vec2(-3*STEP, 0.005977f),
vec2(-2*STEP, 0.060598f),
vec2(-1*STEP, 0.241732f),
vec2( 0*STEP, 0.382928f),
vec2( 1*STEP, 0.241732f),
vec2( 2*STEP, 0.060598f),
vec2( 3*STEP, 0.005977f),
vec2( 4*STEP, 0.000229f)
);

const vec3 sobel_weights[9] = vec3[](
       vec3(-1*STEP, -1*STEP, -1.0f),
       vec3(-1*STEP,  0*STEP, -1.0f),
       vec3(-1*STEP,  1*STEP, -1.0f),
       vec3( 0*STEP, -1*STEP, -1.0f),
       vec3( 0*STEP,  0*STEP,  8.0f),
       vec3( 0*STEP,  1*STEP, -1.0f),
       vec3( 1*STEP, -1*STEP, -1.0f),
       vec3( 1*STEP,  0*STEP, -1.0f),
       vec3( 1*STEP,  1*STEP, -1.0f)
);
     
const vec2 circle_offsets_8[8] = vec2[](
    STEP*CIRCLE_RADIUS*vec2(1.0, 0.0),
    STEP*CIRCLE_RADIUS*vec2(0.980785280447, 0.195090321796),
    STEP*CIRCLE_RADIUS*vec2(0.923879532683, 0.382683431951),
    STEP*CIRCLE_RADIUS*vec2(0.831469612676, 0.55557023246),
    STEP*CIRCLE_RADIUS*vec2(0.707106781821, 0.707106780552),
    STEP*CIRCLE_RADIUS*vec2(0.555570233952, 0.831469611679),
    STEP*CIRCLE_RADIUS*vec2(0.382683433609, 0.923879531996),
    STEP*CIRCLE_RADIUS*vec2(0.195090323556, 0.980785280097)
    );

const vec2 offsets_2d_25[25] = vec2[](
    vec2(-2*STEP, -2*STEP),
    vec2(-2*STEP, -1*STEP),
    vec2(-2*STEP,    STEP),
    vec2(-2*STEP,  1*STEP),
    vec2(-2*STEP,  2*STEP),

    vec2(-1*STEP, -2*STEP),
    vec2(-1*STEP, -1*STEP),
    vec2(-1*STEP,    STEP),
    vec2(-1*STEP,  1*STEP),
    vec2(-1*STEP,  2*STEP),

    vec2( 0*STEP, -2*STEP),
    vec2( 0*STEP, -1*STEP),
    vec2( 0*STEP,    STEP),
    vec2( 0*STEP,  1*STEP),
    vec2( 0*STEP,  2*STEP),

    vec2( 1*STEP, -2*STEP),
    vec2( 1*STEP, -1*STEP),
    vec2( 1*STEP,    STEP),
    vec2( 1*STEP,  1*STEP),
    vec2( 1*STEP,  2*STEP),

    vec2( 2*STEP, -2*STEP),
    vec2( 2*STEP, -1*STEP),
    vec2( 2*STEP,    STEP),
    vec2( 2*STEP,  1*STEP),
    vec2( 2*STEP,  2*STEP)
);

const vec2 offsets_2d_81[81] = vec2[](
       vec2(-4*STEP, -4*STEP),
       vec2(-4*STEP, -3*STEP),
       vec2(-4*STEP, -2*STEP),
       vec2(-4*STEP, -1*STEP),
       vec2(-4*STEP,  0*STEP),
       vec2(-4*STEP,  1*STEP),
       vec2(-4*STEP,  2*STEP),
       vec2(-4*STEP,  3*STEP),
       vec2(-4*STEP,  4*STEP),

       vec2(-3*STEP, -4*STEP),
       vec2(-3*STEP, -3*STEP),
       vec2(-3*STEP, -2*STEP),
       vec2(-3*STEP, -1*STEP),
       vec2(-3*STEP,  0*STEP),
       vec2(-3*STEP,  1*STEP),
       vec2(-3*STEP,  2*STEP),
       vec2(-3*STEP,  3*STEP),
       vec2(-3*STEP,  4*STEP),

       vec2(-2*STEP, -4*STEP),
       vec2(-2*STEP, -3*STEP),
       vec2(-2*STEP, -2*STEP),
       vec2(-2*STEP, -1*STEP),
       vec2(-2*STEP,  0*STEP),
       vec2(-2*STEP,  1*STEP),
       vec2(-2*STEP,  2*STEP),
       vec2(-2*STEP,  3*STEP),
       vec2(-2*STEP,  4*STEP),

       vec2(-1*STEP, -4*STEP),
       vec2(-1*STEP, -3*STEP),
       vec2(-1*STEP, -2*STEP),
       vec2(-1*STEP, -1*STEP),
       vec2(-1*STEP,  0*STEP),
       vec2(-1*STEP,  1*STEP),
       vec2(-1*STEP,  2*STEP),
       vec2(-1*STEP,  3*STEP),
       vec2(-1*STEP,  4*STEP),

       vec2( 0*STEP, -4*STEP),
       vec2( 0*STEP, -3*STEP),
       vec2( 0*STEP, -2*STEP),
       vec2( 0*STEP, -1*STEP),
       vec2( 0*STEP,  0*STEP),
       vec2( 0*STEP,  1*STEP),
       vec2( 0*STEP,  2*STEP),
       vec2( 0*STEP,  3*STEP),
       vec2( 0*STEP,  4*STEP),

       vec2( 1*STEP, -4*STEP),
       vec2( 1*STEP, -3*STEP),
       vec2( 1*STEP, -2*STEP),
       vec2( 1*STEP, -1*STEP),
       vec2( 1*STEP,  0*STEP),
       vec2( 1*STEP,  1*STEP),
       vec2( 1*STEP,  2*STEP),
       vec2( 1*STEP,  3*STEP),
       vec2( 1*STEP,  4*STEP),

       vec2( 2*STEP, -4*STEP),
       vec2( 2*STEP, -3*STEP),
       vec2( 2*STEP, -2*STEP),
       vec2( 2*STEP, -1*STEP),
       vec2( 2*STEP,  0*STEP),
       vec2( 2*STEP,  1*STEP),
       vec2( 2*STEP,  2*STEP),
       vec2( 2*STEP,  3*STEP),
       vec2( 2*STEP,  4*STEP),

       vec2( 3*STEP, -4*STEP),
       vec2( 3*STEP, -3*STEP),
       vec2( 3*STEP, -2*STEP),
       vec2( 3*STEP, -1*STEP),
       vec2( 3*STEP,  0*STEP),
       vec2( 3*STEP,  1*STEP),
       vec2( 3*STEP,  2*STEP),
       vec2( 3*STEP,  3*STEP),
       vec2( 3*STEP,  4*STEP),

       vec2( 4*STEP, -4*STEP),
       vec2( 4*STEP, -3*STEP),
       vec2( 4*STEP, -2*STEP),
       vec2( 4*STEP, -1*STEP),
       vec2( 4*STEP,  0*STEP),
       vec2( 4*STEP,  1*STEP),
       vec2( 4*STEP,  2*STEP),
       vec2( 4*STEP,  3*STEP),
       vec2( 4*STEP,  4*STEP)
);

// from wikipedia, ITU-R YUV from RGB
vec3 intensity_at(in sampler2D texture_in, in float x, in float y)
{
     vec3 t=texture(texture_in, vec2(x,y)).rgb;
     vec3 yuv;
     yuv.r = 0.299*t.r + 0.587*t.g + 0.114*t.b;
     yuv.g = (-0.169*t.r -0.331*t.g + 0.499*t.b)+0.5;
     yuv.b = ( 0.499*t.r -0.418*t.g - 0.0813*t.b)+0.5;
     return yuv;
}

void texture_circle(in sampler2D texture_in, in vec2 centre_xy, in float radius, in vec2[NUM_CIRCLE_STEPS] circle_offsets, out float[NUM_CIRCLE_STEPS*4] colors)
{
    for (int i=0; i<NUM_CIRCLE_STEPS; i++) {
        colors[NUM_CIRCLE_STEPS*0+i] = texture(texture_in, vec2(centre_xy.x+radius*circle_offsets[i].x,
                                                                centre_xy.y+radius*circle_offsets[i].y)
                                                   ).r;
        //colors[NUM_CIRCLE_STEPS*0+i] = texture(texture_in, centre_xy+radius*circle_offsets[i]).r;
        colors[NUM_CIRCLE_STEPS*1+i] = texture(texture_in, vec2(centre_xy.x-radius*circle_offsets[i].y,
                                                                centre_xy.y+radius*circle_offsets[i].x)
                                                   ).r;
        colors[NUM_CIRCLE_STEPS*2+i] = texture(texture_in, vec2(centre_xy.x-radius*circle_offsets[i].x,
                                                                centre_xy.y-radius*circle_offsets[i].y)
                                                   ).r;
        //colors[NUM_CIRCLE_STEPS*2+i] = texture(texture_in, centre_xy-radius*circle_offsets[i]).r;
        colors[NUM_CIRCLE_STEPS*3+i] = texture(texture_in, vec2(centre_xy.x+radius*circle_offsets[i].y,
                                                                centre_xy.y-radius*circle_offsets[i].x)
                                                   ).r;
    }
}

void gauss(in sampler2D texture_in, in vec2 xy, in bool x_not_y, in vec2[NUM_WEIGHTS] offset_weights, out float[NUM_WEIGHTS] colors)
{
    float mx, my;
    mx = 0.0; my=1.0;
    if (x_not_y) {
        mx = 1.0; my=0.0;
    }
    for (int i=0; i<NUM_WEIGHTS; i++) {
        colors[i] = offset_weights[i].y * texture( texture_in,
                                                   vec2(xy.x+offset_weights[i].x*mx, xy.y+offset_weights[i].x*my)).r;
                                                   }
}

vec2 complex_mult(vec2 a, vec2 b)
{
    return vec2(a.r*b.r-a.g*b.g, a.r*b.g+a.g*b.r);
}

#define PI 3.1415926535897932384626433832795
// fd[k] = Sum(n=0..N-1)(td[n]*e^(2i.pi.k.n/N))
void dft32(in float[32] time_domain, out vec2[32]freq_domain)
{
    vec2[32]   fd_1;
    vec2[32]   fd_2;
    vec2[32]   fd_3;
    vec2[32]   fd_4;
    vec2 wk;
    // handle fft.py:fft2 when j=1 and 2 by simple mapping (dft4's, effectively)
    for (int i=0; i<8; i++) {
        int bri;
        bri = i+(((i&1)!=0)?3:0) - (((i&4)!=0)?3:0);
        fd_1[4*i+0]  = vec2(time_domain[bri+0]+time_domain[bri+8]+time_domain[bri+16]+time_domain[bri+24],0.0);
        fd_1[4*i+1]  = vec2(time_domain[bri+0]-time_domain[bri+16],-time_domain[bri+8]+time_domain[bri+24]);
        fd_1[4*i+2]  = vec2(time_domain[bri+0]-time_domain[bri+8]+time_domain[bri+16]-time_domain[bri+24],0.0);
        fd_1[4*i+3]  = vec2(time_domain[bri+0]-time_domain[bri+16],+time_domain[bri+8]-time_domain[bri+24]);
    }
    // handle fft.py:fft2 when j=4
    wk = vec2(1.0,0.0);
    for (int k=0; k<4; k++) {
        const vec2 wm = vec2(cos(-2*PI/8.0),sin(-2*PI/8.0));
        for (int i=0; i<4; i++) {
            vec2 fdm;
            fdm = complex_mult(wk,fd_1[4+k+i*8]);
            fd_2[  k+i*8] = fd_1[k+i*8]+fdm;
            fd_2[4+k+i*8] = fd_1[k+i*8]-fdm;
        }
        wk = complex_mult(wk,wm);
    }
    // handle fft.py:fft2 when j=8
    wk = vec2(1.0,0.0);
    for (int k=0; k<8; k++) {
        const vec2 wm = vec2(cos(-2*PI/16.0),sin(-2*PI/16.0));
        for (int i=0; i<2; i++) {
            vec2 fdm;
            fdm = complex_mult(wk, fd_2[8+k+i*16]);
            fd_3[  k+i*16] = fd_2[k+i*16]+fdm;
            fd_3[8+k+i*16] = fd_2[k+i*16]-fdm;
        }
        wk = complex_mult(wk,wm);
    }
    // handle fft.py:fft2 when j=16
    wk = vec2(1.0,0.0);
    for (int k=0; k<16; k++) {
        const vec2 wm = vec2(cos(-2*PI/32.0),sin(-2*PI/32.0));
        vec2 fdm;
        fdm = complex_mult(wk, fd_3[16+k]);
        fd_4[k]    = fd_3[k]+fdm;
        fd_4[k+16] = fd_3[k]-fdm;
        wk = complex_mult(wk,wm);
    }
    for (int j=0; j<32; j++) {
        freq_domain[j] = fd_4[j];
    }
    freq_domain[0] = vec2(0.0,0.0);
}

void dft32_power(in vec2[32] dft, out float[32] dft_power)
{
    float[32] dft_power_raw;
    float max;
    max = 0.1;
    for (int i=0; i<32; i++) {
        dft_power_raw[i] = dft[i].x*dft[i].x+dft[i].y*dft[i].y;
        if (dft_power_raw[i]>max) max=dft_power_raw[i];
    }
    for (int i=0; i<32; i++) {
        dft_power[i] = dft_power_raw[i]/max;
    }
}

