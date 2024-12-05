#version 330 core

in vec3 ourColor;
in vec2 TexCoord; // Add texture coordinates
out vec4 FragColor;

uniform bool uTransparent;         // Indicates transparency mode
uniform bool uLightEnabled;        // Is lighting active
uniform int uSelectedRoom;         // Room index with active light
uniform int uRoomIndex;            // Index of the current room
uniform bool uUseTexture;          // Indicates if texture should be applied
uniform sampler2D uCharacterTexture; // Texture sampler for character
uniform float uAlpha;              // Alpha for transparency
uniform float uTime;               // Current time for pulsing effect
uniform vec3 lightStartColor;      // Color at the start of pulsing
uniform vec3 lightEndColor;        // Color at the peak of pulsing

void main() {
    vec3 color = ourColor;

    // Handle lighting transitions and pulsing
    if (uLightEnabled && uRoomIndex == uSelectedRoom) {
        // Create a sine-based pulsing effect using time
        float pulse = 0.5 + 0.5 * sin(uTime * 3.0); // Pulses between 0.5 and 1.0
        color = mix(lightStartColor, lightEndColor, pulse); // Interpolate colors based on pulse
    }

    // Apply texture if enabled
    if (uUseTexture && uRoomIndex == uSelectedRoom) {
        vec4 textureColor = texture(uCharacterTexture, TexCoord);
        FragColor = mix(vec4(color, uAlpha), textureColor, uAlpha);
    }
    // Handle transparency
    else if (uTransparent) {
        FragColor = vec4(color, uAlpha);
    } 
    // Default color output
    else {
        FragColor = vec4(color, 1.0); // Fully opaque
    }
}
