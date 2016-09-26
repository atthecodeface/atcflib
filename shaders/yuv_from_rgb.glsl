out vec3 color;
in vec2 uv_to_frag;
uniform sampler2D texture_0;

void main()
{
     color = yuv_at(texture_0, uv_to_frag.x, uv_to_frag.y);
}

