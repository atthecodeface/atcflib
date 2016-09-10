#version 330
out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_to_draw;

#define NUM_OFFSETS 25
#define step (1.0/1024.0)

const vec2 offsets[NUM_OFFSETS] = vec2[](
       vec2(-2.0/1024, -2.0/1024),
       vec2(-2.0/1024, -1.0/1024),
       vec2(-2.0/1024,  0.0/1024),
       vec2(-2.0/1024,  1.0/1024),
       vec2(-2.0/1024,  2.0/1024),

       vec2(-1.0/1024, -2.0/1024),
       vec2(-1.0/1024, -1.0/1024),
       vec2(-1.0/1024,  0.0/1024),
       vec2(-1.0/1024,  1.0/1024),
       vec2(-1.0/1024,  2.0/1024),

       vec2( 0.0/1024, -2.0/1024),
       vec2( 0.0/1024, -1.0/1024),
       vec2( 0.0/1024,  0.0/1024),
       vec2( 0.0/1024,  1.0/1024),
       vec2( 0.0/1024,  2.0/1024),

       vec2( 1.0/1024, -2.0/1024),
       vec2( 1.0/1024, -1.0/1024),
       vec2( 1.0/1024,  0.0/1024),
       vec2( 1.0/1024,  1.0/1024),
       vec2( 1.0/1024,  2.0/1024),

       vec2( 2.0/1024, -2.0/1024),
       vec2( 2.0/1024, -1.0/1024),
       vec2( 2.0/1024,  0.0/1024),
       vec2( 2.0/1024,  1.0/1024),
       vec2( 2.0/1024,  2.0/1024)
);

float intensity_at(in float x, in float y)
{
     return texture(texture_to_draw, vec2(x,y)).r;
}

void main(){
     int i;
     float Ix[NUM_OFFSETS], Iy[NUM_OFFSETS];
     float Ix2, Iy2, Ixy;
     for (i=0; i<NUM_OFFSETS; i++) {
          Ix[i] = intensity_at(uv_to_frag.x + offsets[i].x + step/2.0, uv_to_frag.y + offsets[i].y) - intensity_at(uv_to_frag.x + offsets[i].x - step/2.0, uv_to_frag.y + offsets[i].y);
          Iy[i] = intensity_at(uv_to_frag.x + offsets[i].x, uv_to_frag.y + offsets[i].y + step/2.0) - intensity_at(uv_to_frag.x + offsets[i].x, uv_to_frag.y + offsets[i].y - step/2.0);
     }
     Ix2 = 0.0;
     for (i=0; i<NUM_OFFSETS; i++) {
          Ix2 = Ix2 + Ix[i]*Ix[i];
          Iy2 = Iy2 + Iy[i]*Iy[i];
          Ixy = Ixy + Ix[i]*Iy[i];
     }
     //color = 10000*Ix*Ix*Iy*Iy;//1000.0*(Ix*Ix*Iy*Iy-Ix*Iy*Ix*Iy);
     color = sqrt(sqrt(Ix2*Iy2-Ixy*Ixy));
     //color = 100000.0*(Ix*Ix-Iy*Iy);
}

