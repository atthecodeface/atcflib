// The fragment shader gets an (x,y) output texture frame buffer pixel, since it is invoked for each such pixel
// For each such (x,y) it will get offset_xy in the range 0..out_size.xy
// The fragment shader will also get a uniform 'src_xy'
// Hence the fragment shader can user src_xy.xy + offset_xy.xy, or similarly, if it wishes

out float color;

in vec2 offset_xy;
uniform sampler2D texture_src;
uniform vec2 src_xy;

void main(){
     float[NUM_CIRCLE_STEPS*4] colors;
     vec2[NUM_CIRCLE_STEPS*4] dft;
     float[NUM_CIRCLE_STEPS*4] dft_power;
     int i;
     texture_circle(texture_src, (src_xy+vec2(0.5f,0.5f))/1024.0, 8, circle_offsets_8, colors);
     dft32(colors, dft);
     dft32_power(dft, dft_power);
     i = int(offset_xy.x);
     if (true) {
         color = dft_power[i&31];
     } else if (false) {
         color = dft[i&15].x/8;
         if (i>=16) {
             color = dft[i&15].y/8;
         }
     } else {
         color = dft[(i/2)&15].x/6;
         if ((i&1)!=0) {
             color = dft[(i/2)&15].y/6;
         }
     }
}

