out float color;
in vec2 uv_to_frag;
uniform sampler2D texture_src;

void main()
{
     color = hue_at(texture_src, uv_to_frag.x, uv_to_frag.y);
}

