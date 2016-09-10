#version 330
out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_to_draw;

#define NUM_WEIGHTS 12

float intensity_at(in float x, in float y)
{
     vec3 t=texture(texture_to_draw, vec2(x,y)).rgb;
     return sqrt(t.r*t.r + t.g*t.g + t.b*t.b)*0.8;
}

const vec3 offset_weights[NUM_WEIGHTS] = vec3[](
       vec3(-3.0/1024,  0.0/1024, -1.0f),
       vec3(-2.0/1024,  0.0/1024, -1.0f),
       vec3(-1.0/1024,  0.0/1024, -1.0f),
       vec3( 1.0/1024,  0.0/1024,  1.0f),
       vec3( 2.0/1024,  0.0/1024,  1.0f),
       vec3( 3.0/1024,  0.0/1024,  1.0f),
       vec3( 0.0/1024, -3.0/1024, -1.0f),
       vec3( 0.0/1024, -2.0/1024, -1.0f),
       vec3( 0.0/1024, -1.0/1024, -1.0f),
       vec3( 0.0/1024,  1.0/1024,  1.0f),
       vec3( 0.0/1024,  2.0/1024,  1.0f),
       vec3( 0.0/1024,  3.0/1024,  1.0f)
);
      

void main(){
     float[NUM_WEIGHTS] colors;
     for (int i=0; i<NUM_WEIGHTS; i++) {
          colors[i] = offset_weights[i].z * intensity_at(uv_to_frag.x+offset_weights[i].x, uv_to_frag.y+offset_weights[i].y);
     }
     color = 0.0;
     for (int i=0; i<NUM_WEIGHTS; i++) {
          color += colors[i];
     }
     color=abs(color);
}

