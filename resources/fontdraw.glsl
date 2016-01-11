#version 330 core

uniform mat4 mvp;
uniform sampler2D tex;
uniform vec4 textcolor;
uniform float smoothing;

#ifdef VERTEX

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

smooth out vec2 vtexcoord;

void main() {
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    vtexcoord = texcoord;
}

#endif

#ifdef FRAGMENT

in vec2 vtexcoord;

out vec4 fragcolor;

void main() {
    float a = texture(tex, vtexcoord).a;
    fragcolor = vec4(textcolor.rgb, smoothstep(0.5 - smoothing, 0.5 + smoothing, a) * textcolor.a);
}

#endif
