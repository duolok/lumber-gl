#version 330 core
in vec3 ourColor; 
out vec4 FragColor;

uniform float time; 

void main() {
    float pulsate = sin(time); 
    vec3 pulseColor = mix(vec3(1.0, 0.5, 0.0), vec3(1.0, 1.0, 0.0), pulsate);
    FragColor = vec4(pulseColor, 1.0);
}
