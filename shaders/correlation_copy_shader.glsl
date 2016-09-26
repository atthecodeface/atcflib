// The fragment shader gets an (x,y) output texture frame buffer pixel, since it is invoked for each such pixel
// For each such (x,y) it will get offset_xy in the range 0..out_size.xy
// The fragment shader will also get a uniform 'src_xy'
// Hence the fragment shader can user src_xy.xy + offset_xy.xy, or similarly, if it wishes
//
// Note that src_xy is 0->1023, and offset_xy is 0.5->+31.5 in 0.5 steps
// and the texture coords for the center of each pixel are 0.5/1024->1023.5/1024

out float color;

in vec2 offset_xy;
uniform sampler2D texture_0;
uniform vec2 src_xy;

void main(){
     vec2 texture_xy;
     texture_xy.x = (src_xy.x + offset_xy.x-16)/1024.0;
     texture_xy.y = (src_xy.y + offset_xy.y-16)/1024.0;

     color = texture(texture_0, texture_xy).r;
}

