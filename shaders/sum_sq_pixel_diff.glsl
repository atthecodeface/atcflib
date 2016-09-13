out float color;
in vec2 uv_to_frag; // from vertex shader
uniform sampler2D texture_src;
uniform sampler2D texture_base;
uniform float uv_base_x;
uniform float uv_base_y;

#define NUM_OFFSETS_SQ 81

void main(){
     float sum;
     float[NUM_OFFSETS_SQ] diffs;
     vec2 src_xy;
     src_xy = vec2( (uv_base_x + 15.5)/1024.0,  (uv_base_y + 15.5)/1024.0);
     for (int i=0; i<NUM_OFFSETS_SQ; i++) {
          diffs[i] = ( texture(texture_src,  vec2(uv_to_frag.x+offsets_2d_81[i].x, uv_to_frag.y+offsets_2d_81[i].y)).r -
                       texture(texture_base, vec2(src_xy.x    +offsets_2d_81[i].x, src_xy.y    +offsets_2d_81[i].y)).r );
     }
     sum = 0.0;
     for (int i=0; i<NUM_OFFSETS_SQ; i++) {
          sum += diffs[i]*diffs[i];
     }
     //color = 1.0 - sqrt(sum/NUM_OFFSETS);
     color = 1.0 - sqrt(sum/NUM_OFFSETS_SQ);
     color = color * color * color;
}

