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
uniform float uTransitionProgress; // Transition progress (0.0 to 1.0)
uniform vec3 lightStartColor;      // Color at the start of transition
uniform vec3 lightEndColor;        // Color at the end of transition

void main() {
    vec3 color = ourColor;

    // Handle lighting transitions
    if (uLightEnabled && uRoomIndex == uSelectedRoom) {
        color = mix(lightStartColor, lightEndColor, uTransitionProgress);
    }
    // Apply texture if enabled
	if (uUseTexture && uRoomIndex == uSelectedRoom) {
		vec4 textureColor = texture(uCharacterTexture, TexCoord);
		// Blend the texture color with the calculated color and apply transparency
		FragColor = mix(vec4(color, uAlpha), textureColor, uAlpha);
	}


    // Handle transparency
    else if (uTransparent) {
        FragColor = vec4(color, uAlpha); // Apply transparency
    } 
    // Default color output
    else {
        FragColor = vec4(color, 1.0); // Fully opaque
    }
}
