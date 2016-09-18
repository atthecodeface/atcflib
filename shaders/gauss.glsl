// Requires
// -DX_NOT_Y=true|false
// -DNUM_WEIGHTS=9
// -DWEIGHTS=gauss_offset_weights9

out vec4 color;
in vec2 uv_to_frag;
uniform sampler2D texture_src;

void main()
{
     vec4[NUM_WEIGHTS] colors;
     gauss(texture_src, uv_to_frag.xy, X_NOT_Y, WEIGHTS, colors);
     color = vec4(0.0);
     for (int i=0; i<NUM_WEIGHTS; i++) {
          color += colors[i];
     }
}

