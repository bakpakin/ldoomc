#version 330

uniform mat4 mvp;
uniform sampler2D diffuse;

#ifdef VERTEX

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

smooth out vec2 t;
smooth out vec3 n;

void main() {
    n = normal / 2 + vec3(0.5, 0.5, 0.5);
    t = texcoord;
    gl_Position = mvp * vec4(vertex, 1.0);
}

#endif

#ifdef FRAGMENT

in vec3 n;
in vec2 t;

out vec4 color;

void main() {
    color = texture(diffuse, t) * vec4(n, 1);
}

#endif
