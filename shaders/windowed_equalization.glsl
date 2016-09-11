// Requires
// -DNUM_OFFSETS=25
// -DOFFSETS=offsets_2d_25
// or
// -DNUM_OFFSETS=81
// -DOFFSETS=offsets_2d_81

out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_src;

void main()
{
     int i;
     float sum, sum_sq;
     float mean, variance;
     for (i=0; i<NUM_OFFSETS; i++) {
          float I;
          I = texture(texture_src, vec2(uv_to_frag.x + OFFSETS[i].x, uv_to_frag.y + OFFSETS[i].y)).r;
          sum += I;
          sum_sq += I*I;
     }
     mean = sum/NUM_OFFSETS;
     variance = sum*sum - sum_sq;
     //color = 0.5 + 5*(intensity_at(uv_to_frag.x, uv_to_frag.y) - mean) / sqrt(variance);
     color = abs(NUM_OFFSETS*(texture(texture_src, vec2(uv_to_frag.x, uv_to_frag.y)).r - mean) / sqrt(variance) );
}

