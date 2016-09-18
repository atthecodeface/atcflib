out float color;
in vec2 uv_to_frag; // from vertex shader
uniform sampler2D texture_src;
uniform sampler2D texture_base;
uniform float uv_base_x;
uniform float uv_base_y;

#define NUM_OFFSETS_SQ 81

void main(){
     float sum;
     vec2[8] src_dft;
     vec2 src_xy;
     src_xy = vec2( (uv_base_x+0.5)/1024.0, (uv_base_y+0.5)/1024.0);
     //src_xy = vec2( ((1*40)+0.5)/1024.0, 0.5/1024.0);

     src_dft[1] = vec2(texture(texture_base, vec2(src_xy.x + 1.0/1024.0, src_xy.y + 2.0/1024)).r, 
                       texture(texture_base, vec2(src_xy.x + 1.0/1024.0, src_xy.y + 4.0/1024)).r );

     src_dft[2] = vec2(texture(texture_base, vec2(src_xy.x + 2.0/1024.0, src_xy.y + 2.0/1024)).r, 
                       texture(texture_base, vec2(src_xy.x + 2.0/1024.0, src_xy.y + 4.0/1024)).r );

     src_dft[3] = vec2(texture(texture_base, vec2(src_xy.x + 3.0/1024.0, src_xy.y + 2.0/1024)).r, 
                       texture(texture_base, vec2(src_xy.x + 3.0/1024.0, src_xy.y + 4.0/1024)).r );

     float[NUM_CIRCLE_STEPS*4] colors;
     vec2[8] dft;
     float[8] dft_power;
     vec2[8] dft_normalized;
     float[8]diffs;
     int i;
     texture_circle(texture_src, uv_to_frag, CORR_CIRCLE_RADIUS, circle_offsets_8, colors);
     dft32_8(colors, dft);
     dft32_normalize_8(dft, dft_power, dft_normalized);
     diffs[0] = 0;
     for (int i=1; i<8; i++) {
          float diff;
          //diff = sqrt(dft_power[i])-sqrt(texture(texture_base, vec2(src_xy.x + i/1024.0, src_xy.y + 0.0/1024)).r);
          diff = dft_power[i]-texture(texture_base, vec2(src_xy.x + i/1024.0, src_xy.y + 0.0/1024)).r;
          diffs[i] = diff*diff;
     }
     sum = 0.0;
     for (int i=0; i<8; i++) {
          sum += diffs[i];
     }
     sum = 32*sum*10.0 + 32*dft_rotation_error(src_dft[1],src_dft[2],src_dft[3],dft_normalized[1],dft_normalized[2],dft_normalized[3]);
     //color = 1.0 - sqrt(sum);
     color = 1.0 - sum;
     if (color<0) color=0;
}
