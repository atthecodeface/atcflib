// Requires
// -DNUM_WEIGHTS=9
// -DOFFSET_WEIGHTS=sobel_weights

out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_src;

void main()
{
     float[NUM_WEIGHTS] colors;
     for (int i=0; i<NUM_WEIGHTS; i++) {
          colors[i] = OFFSET_WEIGHTS[i].z * texture(texture_src, vec2(uv_to_frag.x+OFFSET_WEIGHTS[i].x, uv_to_frag.y+OFFSET_WEIGHTS[i].y)).r;
     }
     color = 0.0;
     for (int i=0; i<NUM_WEIGHTS; i++) {
          color += colors[i];
     }
}
