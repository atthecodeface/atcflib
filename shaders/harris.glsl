out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_src;

void main(){
     int i;
     float Ix[NUM_OFFSETS], Iy[NUM_OFFSETS];
     float Ix2, Iy2, Ixy;
     for (i=0; i<NUM_OFFSETS; i++) {
          Ix[i] = texture(texture_src, vec2(uv_to_frag.x + offsets_2d_25[i].x + STEP/2.0, uv_to_frag.y + offsets_2d_25[i].y)).r -
                     texture(texture_src, vec2(uv_to_frag.x + offsets_2d_25[i].x - STEP/2.0, uv_to_frag.y + offsets_2d_25[i].y)).r;
          Iy[i] = texture(texture_src, vec2(uv_to_frag.x + offsets_2d_25[i].x, uv_to_frag.y + offsets_2d_25[i].y + STEP/2.0)).r -
                     texture(texture_src, vec2(uv_to_frag.x + offsets_2d_25[i].x, uv_to_frag.y + offsets_2d_25[i].y - STEP/2.0)).r;
     }
     Ix2 = 0.0;
     for (i=0; i<NUM_OFFSETS; i++) {
          Ix2 = Ix2 + Ix[i]*Ix[i];
          Iy2 = Iy2 + Iy[i]*Iy[i];
          Ixy = Ixy + Ix[i]*Iy[i];
     }
     color = sqrt(sqrt(Ix2*Iy2-Ixy*Ixy));
}

