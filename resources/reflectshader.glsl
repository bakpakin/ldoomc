#version 330 core

uniform mat4 u_m;
uniform mat4 u_mvp;
uniform sampler2D u_diffuse;
uniform samplerCube u_envmap;
uniform vec3 u_cameraPosition;

#ifdef VERTEX

layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texcoord;

smooth out vec3 normal;
smooth out vec3 position;
smooth out vec2 texcoord;

void main() {
    texcoord = a_texcoord;
    normal = mat3(transpose(inverse(u_m))) * a_normal;
    position = vec3(u_m * vec4(a_vertex, 1.0));
    gl_Position = u_mvp * vec4(a_vertex, 1.0);
}

#endif

#ifdef FRAGMENT

smooth in vec2 texcoord;
smooth in vec3 position;
smooth in vec3 normal;

out vec4 color;

void main() {
    vec3 I = normalize(position - u_cameraPosition);
    vec3 R = reflect(I, normalize(normal));
    color = texture(u_diffuse, texcoord) * texture(u_envmap, R);
}

#endif
