#version 330
out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_to_draw;

float intensity_at(in float x, in float y)
{
     vec3 t=texture(texture_to_draw, vec2(x,y)).rgb;
     return sqrt(t.r*t.r + t.g*t.g + t.b*t.b)*0.8;
}

const vec3 offset_weights[9] = vec3[](
       vec3(-1.0/1024, -1.0/1024, -1.0f),
       vec3(-1.0/1024,  0.0/1024, -1.0f),
       vec3(-1.0/1024,  1.0/1024, -1.0f),
       vec3( 0.0/1024, -1.0/1024, -1.0f),
       vec3( 0.0/1024,  0.0/1024,  8.0f),
       vec3( 0.0/1024,  1.0/1024, -1.0f),
       vec3( 1.0/1024, -1.0/1024, -1.0f),
       vec3( 1.0/1024,  0.0/1024, -1.0f),
       vec3( 1.0/1024,  1.0/1024, -1.0f)
);
      

void main(){
     float[9] colors;
     for (int i=0; i<9; i++) {
          colors[i] = offset_weights[i].z * intensity_at(uv_to_frag.x+offset_weights[i].x, uv_to_frag.y+offset_weights[i].y);
     }
     color = 0.0;
     for (int i=0; i<9; i++) {
          color += colors[i];
     }
     //     color = intensity_at(uv_to_frag.x, uv_to_frag.y);
     //color = 1.0f; //uv_to_frag.x;
}

