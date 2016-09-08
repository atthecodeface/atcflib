#version 330
out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_to_draw;

float intensity_at(in float x, in float y)
{
     vec3 t=texture(texture_to_draw, vec2(x,y)).rgb;
     return sqrt(t.r*t.r + t.g*t.g + t.b*t.b)*0.8;
}

void main(){
     color = intensity_at(uv_to_frag.x, uv_to_frag.y);
}

