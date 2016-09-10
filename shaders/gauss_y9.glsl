#version 330
out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_to_draw;

#define NUM_WEIGHTS 9

float intensity_at(in float x, in float y)
{
     return texture(texture_to_draw, vec2(x,y)).r;
}

const vec3 offset_weights[NUM_WEIGHTS] = vec3[](
vec3(0.0/1024.0, -4.0/1024.0, 0.000229f),
vec3(0.0/1024.0, -3.0/1024.0, 0.005977f),
vec3(0.0/1024.0, -2.0/1024.0, 0.060598f),
vec3(0.0/1024.0, -1.0/1024.0, 0.241732f),
vec3(0.0/1024.0,  0.0/1024.0, 0.382928f),
vec3(0.0/1024.0,  1.0/1024.0, 0.241732f),
vec3(0.0/1024.0,  2.0/1024.0, 0.060598f),
vec3(0.0/1024.0,  3.0/1024.0, 0.005977f),
vec3(0.0/1024.0,  4.0/1024.0, 0.000229f)
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
     //color=abs(color);
}

