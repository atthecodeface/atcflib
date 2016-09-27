// texture_0/1/2/3 are the main textures, indexed by uv_to_frag

out vec4 color;
in vec2 uv_to_frag; // from vertex shader
uniform sampler2D texture_0;
uniform sampler2D texture_1;
uniform sampler2D texture_2;
uniform sampler2D texture_3;

const ivec2 discrete_circle_offsets_4_32[32] = ivec2[](
    ivec2(4,0),
    ivec2(4,1),
    ivec2(4,2),
    ivec2(3,2),
    ivec2(3,3),
    ivec2(2,3),
    ivec2(2,4),
    ivec2(1,4),

    ivec2(0,4),
    ivec2(-1,4),
    ivec2(-2,4),
    ivec2(-2,3),
    ivec2(-3,3),
    ivec2(-3,2),
    ivec2(-4,2),
    ivec2(-4,1),

    ivec2(-4,0),
    ivec2(-4,-1),
    ivec2(-4,-2),
    ivec2(-3,-2),
    ivec2(-3,-3),
    ivec2(-2,-3),
    ivec2(-2,-4),
    ivec2(-1,-4),

    ivec2(0,-4),
    ivec2(1,-4),
    ivec2(2,-4),
    ivec2(2,-3),
    ivec2(3,-3),
    ivec2(3,-2),
    ivec2(4,-2),
    ivec2(4,-1) 
    );

const ivec2 discrete_circle_offsets_4_16[16] = ivec2[](
    ivec2(4,0),
    ivec2(4,2),
    ivec2(3,3),
    ivec2(2,4),

    ivec2(0,4),
    ivec2(-2,4),
    ivec2(-3,3),
    ivec2(-4,2),

    ivec2(-4,0),
    ivec2(-4,-2),
    ivec2(-3,-3),
    ivec2(-2,-4),

    ivec2(0,-4),
    ivec2(2,-4),
    ivec2(3,-3),
    ivec2(4,-2)
    );

void main(){
    ivec2 src_xy;
    ivec2 base_xy;
    vec2 max_dxy_sum;
    float max_dxy_l2;

    max_dxy_l2 = 0;
    max_dxy_sum = vec2(0.0,0.0);

    src_xy = ivec2( int(1024.0*uv_to_frag.x), int(1024.0*uv_to_frag.y));

    for (int a=0; a<NUM_OFFSETS; a++) {
        ivec2 dxy;
        int dx, dy;
        vec2 src0_data, src1_data, src2_data, src3_data;
        vec2 dxy_sum;
        float dxy_l2;
        
        dxy = DISCRETE_CIRCLE_OFS[a];
        dx = dxy.x;
        dy = dxy.y;
        src0_data  = texelFetch(texture_0, src_xy+ivec2( dx, dy), 0).rg;
        src1_data  = texelFetch(texture_1, src_xy+ivec2(-dy, dx), 0).rg;
        src2_data  = texelFetch(texture_2, src_xy+ivec2(-dx,-dy), 0).rg;
        src3_data  = texelFetch(texture_3, src_xy+ivec2( dy,-dx), 0).rg;

        dxy_sum = src0_data + src1_data + src2_data + src3_data;
        dxy_l2  = (dxy_sum.x*dxy_sum.x + dxy_sum.y*dxy_sum.y);

        if (dxy_l2 > max_dxy_l2) {
            max_dxy_l2  = dxy_l2;
            max_dxy_sum = dxy_sum;
        }
    }
    color.r = sqrt(max_dxy_l2);
    color.r = max_dxy_l2;
    color.gb = max_dxy_sum/4;
}
