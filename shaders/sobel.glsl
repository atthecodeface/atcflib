out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_src;

float intensity_at(in float x, in float y)
{
     return texture(texture_src, vec2(x,y)).r;
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

