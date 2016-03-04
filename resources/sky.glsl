#version 330 core

uniform vec3 direction;
uniform vec3 sundir;

#ifdef VERTEX

layout(location = 0) in vec2 vert;

out vec3 dir;

void main() {
    dir = vec3(vert, 1.0);
}

#endif

#ifdef FRAGMENT

in vec3 dir;

out vec4 color;

void main() {
    color = vec4(dir * sundir, 1.0);
}

#endif
