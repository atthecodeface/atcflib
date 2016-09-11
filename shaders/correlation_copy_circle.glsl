// The fragment shader gets an (x,y) output texture frame buffer pixel, since it is invoked for each such pixel
// For each such (x,y) it will get offset_xy in the range 0..out_size.xy
// The fragment shader will also get a uniform 'src_xy'
// Hence the fragment shader can user src_xy.xy + offset_xy.xy, or similarly, if it wishes

out float color;

in vec2 offset_xy;
uniform sampler2D texture_src;
uniform vec2 src_xy;

void main(){
     //     vec2 texture_xy;
     float[NUM_CIRCLE_STEPS*4] colors;
     int i;
     //     texture_xy.x = (src_xy.x + offset_xy.x)/1024.0;
     //     texture_xy.y = (src_xy.y + offset_xy.y)/1024.0;
     //     color = texture(texture_src, texture_xy).r;
     texture_circle(texture_src, src_xy/1024.0, circle_offsets_8, colors);
     i = int(offset_xy.x);
     color = colors[i];
     //     color = i/32.0;
}

