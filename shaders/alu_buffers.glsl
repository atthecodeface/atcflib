out float color;
in vec2 uv_to_frag; // from vertex shader
uniform sampler2D texture_src;
uniform sampler2D texture_base;
uniform float uv_base_x;
uniform float uv_base_y;

void main(){
     float src_a;
     float src_b;
     float x;
     float y;
     float dummy;
     dummy = uv_base_x + uv_base_y;
     x = uv_to_frag.x;
     y = uv_to_frag.y;
     src_a = texture(texture_src,  uv_to_frag).r;
     src_b = texture(texture_base, uv_to_frag).r;
     color = OP;
}

