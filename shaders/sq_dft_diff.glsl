out float color;
in vec2 uv_to_frag; // from vertex shader
uniform sampler2D texture_src;
uniform sampler2D texture_base;
uniform float uv_base_x;
uniform float uv_base_y;

#define NUM_OFFSETS_SQ 81

void main(){
     float sum;
     vec2 src_xy;
     src_xy = vec2( (uv_base_x+0.5)/1024.0, (uv_base_y+0.5)/1024.0);
     //src_xy = vec2( (uv_base_x+0)/1024.0, (uv_base_y+0)/1024.0);
     src_xy = vec2( ((7*40)+0.5)/1024.0, 0.5/1024.0);

     float[NUM_CIRCLE_STEPS*4] colors;
     vec2[NUM_CIRCLE_STEPS*4] dft;
     float[NUM_CIRCLE_STEPS*4] dft_power;
     float[16]diffs;
     int i;
     texture_circle(texture_src, uv_to_frag, 8, circle_offsets_8, colors);
     dft32(colors, dft);
     dft32_power(dft, dft_power);
     for (int i=0; i<16; i++) {
          float diff;
          diff = dft_power[i]-texture(texture_base, vec2(src_xy.x + i/1024.0, src_xy.y)).r;
          diffs[i] = diff*diff;
     }
     sum = 0.0;
     for (int i=0; i<16; i++) {
          sum += diffs[i];
     }
     //color = 1.0 - sqrt(sum);
     color = 1.0 - 100*sum;
}
