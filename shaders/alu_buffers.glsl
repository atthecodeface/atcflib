out float color;
in vec2 uv_to_frag; // from vertex shader
uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform float uv_base_x;
uniform float uv_base_y;

#ifdef EXTRA_UNIFORMS
EXTRA_UNIFORMS
#endif

void main(){
     float src_a;
     float src_b;
     float x;
     float y;
     float dummy;
     dummy = uv_base_x + uv_base_y;
     x = uv_to_frag.x;
     y = uv_to_frag.y;
     src_a = texture(texture_0, uv_to_frag).r;
     src_b = texture(texture_1, uv_to_frag).r;
     color = OP;
}

