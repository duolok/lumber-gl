#version 330 core

layout (location = 0) in vec3 aPos;       
layout (location = 1) in vec2 aTexCoord;  

uniform float uTime;
uniform float uStartTime;
uniform vec2 uOrigin;

out vec2 TexCoord;
out float vAlpha;

void main()
{
    float elapsed = uTime - uStartTime;
    // Vertical movement upwards
    float verticalOffset = elapsed * 0.05; // Adjust speed as needed
    // Horizontal wiggle
    float horizontalOffset = 0.01 * sin(uTime * 6.0 + aPos.y * 15.0);

    vec3 position = aPos;
    position.x += horizontalOffset;
    position.y += verticalOffset;
    position.xy += uOrigin;

    gl_Position = vec4(position, 1.0);
    TexCoord = aTexCoord;
    vAlpha = 1.0 - (elapsed / 3.0); 
}
