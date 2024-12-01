#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 ourColor;

uniform vec2 uPos; // Translation position
uniform bool uFlip; // Whether to flip the VAO horizontally

void main() {
    vec3 newPosition = aPos;

    // Apply translation to keep the dog in place
    newPosition += vec3(uPos, 0.0);

    gl_Position = vec4(newPosition, 1.0);
    ourColor = aColor; 
}
