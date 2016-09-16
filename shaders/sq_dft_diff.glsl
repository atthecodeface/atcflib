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
     // Note - changed main to use src+7,7
     // RADIUS 12
     // 0: (487,667) = 0.932599
     // 1: (708,1010) = 0.920047
     // 2: (657,883) = 0.915259
     // 3: (548,381) = 0.900764
     // 4: (29,836) = 0.891968
     // 5: (152,767) = 0.890023
     // 6: (644,731) = 0.887374
     // 7: (18,843) = 0.885590
     // 8: (333,658) = 0.880465
     // 9: (941,822) = 0.878940
     // 10: (851,943) = 0.878633
     // RADIUS 10
     // 0: (638,535) = 0.924170
     // 1: (686,865) = 0.904367
     // 2: (309,782) = 0.887422
     // 3: (250,872) = 0.885033
     // 4: (275,871) = 0.877652
     // 5: (496,977) = 0.870845
     // 6: (645,862) = 0.859671
     // 7: (656,877) = 0.856310
     // 8: (762,971) = 0.849142
     // 9: (17,942) = 0.844778
     // 10: (642,733) = 0.843807
     // RADIUS 8
     // 0: (547,577) = 0.915761
     // 1: (565,716) = 0.891051
     // 2: (16,971) = 0.852360
     // 3: (616,561) = 0.826802
     // 4: (745,923) = 0.820159
     // 5: (91,516) = 0.818857
     // 6: (521,887) = 0.817607
     // 7: (550,384) = 0.816141
     // 8: (290,734) = 0.814591
     // 9: (171,743) = 0.807550
     // 10: (684,864) = 0.807008
     src_xy = vec2( ((4*40)+0.5)/1024.0, 0.5/1024.0);

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
