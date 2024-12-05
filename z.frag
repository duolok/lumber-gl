
#version 330 core
in vec2 TexCoord;
in float vAlpha;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec3 uColor; // Color of the 'Z' letter

void main()
{
    float alpha = texture(uTexture, TexCoord).r * vAlpha;
    if (alpha < 0.1)
        discard;
    FragColor = vec4(uColor, alpha);
}
