// texture_src is the main texture, indexed by uv_to_frag
// texture_base is indexed by uv_base_x and uv_base_y, which need to be converted to 0->1 from 0->1023

out vec4 color;
in vec2 uv_to_frag; // from vertex shader
uniform usampler2D texture_src;
uniform usampler2D texture_base;
uniform float uv_base_x;
uniform float uv_base_y;

void main(){
    float sum;
    float angle_diff[8];
    ivec2 src_xy;
    ivec2 base_xy;
    uvec4 src_dft, base_dft;

    // 275 598 should be a good match in src to 272,643
    // 274 603? should be a good match in src to 272,651
    // 272 598? should be a good match in src to 268,647
    base_xy = ivec2( int(uv_base_x), int(uv_base_y));
    src_xy = ivec2( int(1024.0*uv_to_frag.x), int(1024.0*uv_to_frag.y));

    base_dft = texelFetch(texture_base, base_xy, 0);
    src_dft  = texelFetch(texture_src, src_xy, 0);

    float p0,a0, p1,a1, diff, adiff, rotation;
    diff = 1.0;

    unpack_power_angle(src_dft.y, p0, a0);
    unpack_power_angle(base_dft.y, p1, a1);
    diff = diff * power_diff(p0,p1);
    rotation = flt_angle_diff(a0, a1);
    color.b = diff/10;

    unpack_power_angle(src_dft.w, p0, a0);
    unpack_power_angle(base_dft.w, p1, a1);
    diff = diff * power_diff(p0,p1);
    adiff = flt_angle_diff(a0, a1);
    adiff = 1.0-flt_angle_diff_scale_abs(rotation, 3.0, adiff)/4.0;
    diff = adiff*diff;
    color.g = adiff;

    unpack_power_angle(src_dft.z, p0, a0);
    unpack_power_angle(base_dft.z, p1, a1);
    diff = diff * power_diff(p0,p1);
    adiff = flt_angle_diff(a0, a1);
    adiff = 1.0-flt_angle_diff_scale_abs(rotation, 2.0, adiff)/4.0;
    diff = adiff*diff;
    color.r = adiff;

    unpack_power_angle(src_dft.x, p0, a0);
    unpack_power_angle(base_dft.x, p1, a1);
    diff = diff * power_diff(p0,p1);
    color.b = diff;
    vec2 v = flt_angle_cs(rotation);
    diff = (diff<0)?0:diff;
    color.r = v.x*diff;
    color.g = v.y*diff;
}
