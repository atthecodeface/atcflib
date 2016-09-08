#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location=0) in vec3 vertex;
layout(location=1)   in vec2 uv;

out vec2 uv_to_frag;
void main(){
    gl_Position = vec4(vertex.x*2.0-1.0, vertex.y*2.0-1.0, 0.0f, 1.0f);
    uv_to_frag = uv;
}
