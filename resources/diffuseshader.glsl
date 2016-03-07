#version 330 core

uniform mat4 u_mvp;
uniform sampler2D u_diffuse; 

#ifdef VERTEX

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

smooth out vec2 t;
smooth out vec4 norm;

void main() {
    t = texcoord;
    norm = vec4(normal, 1.0);
    gl_Position = u_mvp * vec4(vertex, 1.0);
}

#endif

#ifdef FRAGMENT

in vec2 t;
in vec4 norm;

out vec4 color;

void main() {
    color = texture(u_diffuse, t) * norm;
}

#endif
