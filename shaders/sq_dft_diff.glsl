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
     //src_xy = vec2( (uv_base_x+0)/1024.0, (uv_base_y+0)/1024.0);
     // Best Harris of smoothed IMG_1854.JPG are (883,867), (127,982), (788,885), (172,611), (595,694), (121,970), (917,860), (654,385)
     // Matches in IMG_1855.JPG are              None,       None,     (674,882),   None,    (463,694),    None,     None,    (548,383)
     // Using max=1.0
     // Using match of 1-10*(10*(sum of squares of (fft_power_sq - fft_power_sq) for (1..15)) + rotation error(1->2))
     // Best match closeness                      0.578      0.8379     0.904428   0.9649    0.94677
     // Best match point                                                (524,248) (245,700)  (697,839)
     // Notes                                                           Cannot do            looks like should do
     //                                                                                      (466,694) is 6th match of 0.702295
     // Without the rotation error
     //                                                                                      (466,694) is 11th match of 0.891306
     // FFT error must be                                                                    0.00109
     // Rotation (1->2) error must be                                                        0.019
     // With rotation error (1->3)
     //                                                                                      (466,694) is 5th match of 0.801522
     // Rotation (1->3) error must be                                                        0.0098
     //          
     // Inc rotation 1->2
     //0: (697,839) = 0.946770
     //1: (686,873) = 0.777848
     //2: (624,976) = 0.755788
     //3: (476,684) = 0.748361
     //4: (237,711) = 0.725051
     //5: (501,827) = 0.704485
     //6: (466,694) = 0.702295
     //7: (547,806) = 0.658399
     // Inc rotation 1->3
     // 0: (764,913) = 0.926105
     // 1: (972,965) = 0.892018
     // 2: (697,839) = 0.826252
     // 3: (628,975) = 0.819499
     // 4: (466,694) = 0.801522
     // 5: (476,684) = 0.773966
     // 6: (478,700) = 0.769120
     //
     // Similar match at (697, 839)
     // Similar match at (624-628, 975-976)
     // which is about right
     src_xy = vec2( ((4*40)+0.5)/1024.0, 0.5/1024.0);

     src_dft[1] = vec2(texture(texture_base, vec2(src_xy.x + 1.0/1024.0, src_xy.y + 2.0/1024)).r, 
                       texture(texture_base, vec2(src_xy.x + 1.0/1024.0, src_xy.y + 4.0/1024)).r );

     src_dft[2] = vec2(texture(texture_base, vec2(src_xy.x + 2.0/1024.0, src_xy.y + 2.0/1024)).r, 
                       texture(texture_base, vec2(src_xy.x + 2.0/1024.0, src_xy.y + 4.0/1024)).r );

     src_dft[3] = vec2(texture(texture_base, vec2(src_xy.x + 3.0/1024.0, src_xy.y + 2.0/1024)).r, 
                       texture(texture_base, vec2(src_xy.x + 3.0/1024.0, src_xy.y + 4.0/1024)).r );

     float[NUM_CIRCLE_STEPS*4] colors;
     vec2[NUM_CIRCLE_STEPS*4] dft;
     float[NUM_CIRCLE_STEPS*4] dft_power;
     vec2[NUM_CIRCLE_STEPS*2] dft_normalized;
     float[16]diffs;
     int i;
     texture_circle(texture_src, uv_to_frag, 8, circle_offsets_8, colors);
     dft32(colors, dft);
     dft32_normalize(dft, dft_power, dft_normalized);
     diffs[0] = 0;
     for (int i=1; i<16; i++) {
          float diff;
          //diff = sqrt(dft_power[i])-sqrt(texture(texture_base, vec2(src_xy.x + i/1024.0, src_xy.y + 0.0/1024)).r);
          diff = dft_power[i]-texture(texture_base, vec2(src_xy.x + i/1024.0, src_xy.y + 0.0/1024)).r;
          diffs[i] = diff*diff;
     }
     sum = 0.0;
     for (int i=0; i<16; i++) {
          sum += diffs[i];
     }
     sum = sum*10.0 + dft_rotation_error(src_dft,dft_normalized);
     //color = 1.0 - sqrt(sum);
     color = 1.0 - 10*sum;
}
