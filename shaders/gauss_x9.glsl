out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_src;

void main()
{
     float[NUM_WEIGHTS] colors;
     gauss(texture_src, uv_to_frag.xy, true, gauss_offset_weights9, colors);
     color = 0.0;
     for (int i=0; i<NUM_WEIGHTS; i++) {
          color += colors[i];
     }
}

