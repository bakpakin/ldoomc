#version 330 core

uniform mat4 mvp;
uniform sampler2D tex;
uniform vec4 color;

#ifdef VERTEX

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

smooth out vec2 vtexcoord;

void main() {
    gl_Position = mvp * vec4(position, 0, 1);
    vtexcoord = texcoord;
}

#endif

#ifdef FRAGMENT

in vec2 vtexcoord;

const float smoothing = 1.0/16.0;

out vec4 fragcolor;

void main() {
    vec4 dandcolor = texture(tex, vtexcoord);
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, dandcolor.a) * color.a;
    fragcolor = vec4(color.rgb, alpha);
}

#endif
