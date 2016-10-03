// Input vertex data, different for all executions of this shader.
layout(location=0) in vec3 vertex;
layout(location=1)   in vec2 uv;

out vec2 uv_to_frag;

#ifndef GL_POSITION
#define GL_POSITION vec4(x*2.0-1.0, y*2.0-1.0, 0.0f, 1.0f)
#endif

#ifndef UV_TO_FRAG
#define UV_TO_FRAG (uv)
#endif

#ifdef EXTRA_VERTEX_UNIFORMS
EXTRA_VERTEX_UNIFORMS
#endif

void main(){
    float x,y;
    x = vertex.x;
    y = vertex.y;
    gl_Position = GL_POSITION;
    uv_to_frag = UV_TO_FRAG;
}

