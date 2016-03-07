#version 330 core

uniform mat4 u_vp;
uniform samplerCube u_skybox;

#ifdef VERTEX

layout(location = 0) in vec3 vert;

out vec3 texcoord;

void main() {
    vec4 pos = (u_vp * vec4(vert, 1.0)).xyww;
    texcoord = vert;
    gl_Position = pos;
}

#endif

#ifdef FRAGMENT

out vec4 color;
in vec3 texcoord;

void main() {
    color = texture(u_skybox, texcoord);
}

#endif
