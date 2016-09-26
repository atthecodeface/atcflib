// Requires
// -DCOPY_CIRCLE_RADIUS=8

out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_0;

void main()
{
     float[32] colors;
     texture_circle(texture_0, uv_to_frag.xy, COPY_CIRCLE_RADIUS, circle_offsets_8, colors);
     color = 0.0;
     for (int i=0; i<32; i++) {
          color += colors[i];
     }
     color = color/16.0;
}

