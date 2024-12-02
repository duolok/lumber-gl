#version 330 core
layout(location = 0) in vec2 aPos;

uniform float uTime; 
uniform vec2 uOrigin; 

void main() {
    vec2 position = aPos;

    float verticalOffset = max(0.0, 0.1 * sin(uTime * 0.3)); 
    position.y += verticalOffset + 0.1;

    // horizontal wiggle
    position.x += 0.02 * sin(uTime * 6.0 + position.y * 15.0);

    position += uOrigin;

    position *= 0.4; 
    gl_Position = vec4(position, 0.0, 1.0);
}