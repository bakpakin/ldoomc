#version 330 core

uniform mat4 u_mvp;
uniform sampler2D u_diffuse; 

#ifdef VERTEX

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

smooth out vec2 t;

void main() {
    t = texcoord;
    gl_Position = u_mvp * vec4(vertex, 1.0);
}

#endif

#ifdef FRAGMENT

smooth in vec2 t;

out vec4 color;

void main() {
    color = texture(u_diffuse, t);
}

#endif
