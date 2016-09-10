// This shader will be invoked for a rectangle surrounding a single source texture pixel
// The rectangle should be (0.0,0.999)
// The source texture pixel is not required here - is is only needed in the fragment shader
// The output texture (framebuffer) pixel (top left) is given by out_xy.
// The output texture (framebuffer) area is out_size.xy pixels
// The fragment shader gets an (x,y) output texture frame buffer pixel, since it is invoked for each such pixel
// For each such (x,y) it will get offset_xy in the range 0..out_size.xy
// The fragment shader will also get a uniform 'src_xy'
// Hence the fragment shader can user src_xy.xy + offset_xy.xy, or similarly, if it wishes
layout(location=0) in vec2 vertex;

uniform vec2 out_size;
uniform vec2 out_xy;

out vec2 offset_xy;
void main()
{
    vec2 out_pixel;
    
    offset_xy.x = out_size.x*vertex.x;
    offset_xy.y = out_size.y*vertex.y;
    out_pixel.x = out_xy.x + offset_xy.x;
    out_pixel.y = out_xy.y + offset_xy.y;
    gl_Position = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    gl_Position.x = (out_pixel.x/1024.0)*2.0-1.0;
    gl_Position.y = (out_pixel.y/1024.0)*2.0-1.0;
}
