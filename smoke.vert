#version 330 core
layout(location = 0) in vec2 aPos;    // Smoke square position
layout(location = 1) in vec2 offset; // Particle offset

uniform float uTime; // Pass time for wiggling effect

void main() {
    vec2 wigglyOffset = vec2(offset.x + 0.05 * sin(uTime * 5.0 + offset.y * 10.0), offset.y);
    gl_Position = vec4(aPos + wigglyOffset, 0.0, 1.0);
}
