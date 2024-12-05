#version 330 core

// Input vertex attributes
layout(location = 0) in vec3 aPos;      // Position attribute
layout(location = 1) in vec3 aColor;    // Color attribute
layout(location = 2) in vec2 aTexCoord; // Texture coordinate attribute

// Output to the fragment shader
out vec3 ourColor;
out vec2 TexCoord;

void main() {
    // Set the position of the current vertex
    gl_Position = vec4(aPos, 1.0);

    // Pass the color to the fragment shader
    ourColor = aColor;

    // Pass the texture coordinates to the fragment shader
    TexCoord = aTexCoord;
}
