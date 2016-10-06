out vec4 color;
in vec2 uv_to_frag; // from vertex shader
uniform sampler2D texture_0;
//uniform sampler2D texture_1;
uniform float uv_base_x;
uniform float uv_base_y;

#ifdef EXTRA_FRAGMENT_UNIFORMS
EXTRA_FRAGMENT_UNIFORMS
#endif

void main(){
    vec2 z, c;
    float zl;
    c = vec2(uv_to_frag.x*2-1.5,1.0-uv_to_frag.y*2);
    z = c;
    vec4 src_a = texture(texture_0, uv_to_frag);
    for (int i=0; i<128; i++) {
        // z = z^2 + c
        z = vec2(z.x*z.x-z.y*z.y+c.x, 2*z.x*z.y+c.y);
        zl = z.x*z.x+z.y*z.y;
        if (zl>4.0) break;
    }
    color = vec4(abs(z.x)*0.5,abs(z.y)*1.0,0.0,1.0);
    if (zl>4.0) {
        if (abs(z.x)>abs(z.y)) {
            color = vec4(abs(z.y)/abs(z.x),1.0,1.0,1.0);
        } else {
            color = vec4(1.0,abs(z.x)/abs(z.y),1.0,1.0);
        }
    }
}

