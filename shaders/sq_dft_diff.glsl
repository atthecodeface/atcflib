out vec4 color;
in vec2 uv_to_frag; // from vertex shader
uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform float uv_base_x;
uniform float uv_base_y;

#define NUM_OFFSETS_SQ 81

void main(){
    float sum;
    float angle_diff[8];
    vec2 src_xy;
     src_xy = vec2( (uv_base_x+0.5)/1024.0, (uv_base_y+0.5)/1024.0);

     float[NUM_CIRCLE_STEPS*4] colors;
     vec2[8] dft;
     float[8] dft_power;
     vec2[8] dft_normalized;
     float[8]diffs;
     int i;
     texture_circle(texture_0, uv_to_frag, CORR_CIRCLE_RADIUS, circle_offsets_8, colors);
     dft32_8(colors, dft);
     dft32_normalize_8(dft, dft_power, dft_normalized);
     diffs[0] = 0;
     for (int i=1; i<8; i++) {
          float diff;
          float ang;
          vec4 src_dft;
          src_dft = texture(texture_1, vec2(src_xy.x + i/1024.0, src_xy.y));
          diff = dft_power[i]-src_dft.x;
          angle_diff[i] = flt_angle(dft_normalized[i]) - src_dft.y*8.0;
          diffs[i] = diff*diff;
     }
     sum = 0.0;
     for (int i=0; i<8; i++) {
          sum += diffs[i];
     }
     sum = 32*sum*10.0;// + 32*dft_rotation_error(src_dft[1],src_dft[2],src_dft[3],dft_normalized[1],dft_normalized[2],dft_normalized[3]);
     color.x = 1.0 - sum;
     if (color.x<0) color.x=0;
     float angle_error;
     angle_error = angle_diff[1]*2 - angle_diff[2]; // -3 to +3 - note that angle_diff[1]==0.98 then that is about 0...
     angle_error = angle_error - floor(angle_error);
     angle_error = (angle_error>0.5)?(1.0-angle_error):angle_error;
     color.y = angle_error*2;
     color.z = (angle_diff[1]+1)/2;
     color.w = (angle_diff[3]+1)/2;
}
