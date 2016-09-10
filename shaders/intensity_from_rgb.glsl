out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_to_draw;

void main(){
     color = intensity_at(texture_to_draw, uv_to_frag.x, uv_to_frag.y);
}

