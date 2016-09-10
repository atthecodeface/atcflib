#version 330
out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_to_draw;

#define NUM_WEIGHTS 25
const vec2 offsets[NUM_WEIGHTS] = vec2[](
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
     float sum, sum_sq;
     float mean, variance;
     for (i=0; i<NUM_WEIGHTS; i++) {
          float I;
          I = intensity_at(uv_to_frag.x + offsets[i].x, uv_to_frag.y + offsets[i].y);
          sum += I;
          sum_sq += I*I;
     }
     mean = sum/NUM_WEIGHTS;
     variance = sum*sum - sum_sq;
     //color = 0.5 + 5*(intensity_at(uv_to_frag.x, uv_to_frag.y) - mean) / sqrt(variance);
     color = abs(NUM_WEIGHTS*(intensity_at(uv_to_frag.x, uv_to_frag.y) - mean) / sqrt(variance) );
}

