#version 330 core

layout (location = 0) in vec3 aPos;   
layout (location = 1) in vec3 aColor; 

out vec3 ourColor;

uniform vec2 uPos;   
uniform bool uFlip;  

const vec2 dogCenter = vec2(-0.65, -0.68);

void main() {
    vec3 newPosition = aPos;

    if (uFlip) {
        newPosition.x = 2.0 * dogCenter.x - newPosition.x;
    }

    newPosition += vec3(uPos, 0.0);

    gl_Position = vec4(newPosition, 1.0);
    ourColor = aColor; 
}
