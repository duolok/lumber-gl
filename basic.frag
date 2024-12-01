#version 330 core

in vec3 ourColor;
out vec4 FragColor;

uniform int uH;
uniform bool isFence;
uniform float dim;


void main() {

	if (isFence) {
		int stripeHeight = uH / 12;
		if (mod(gl_FragCoord.y, 10) < 8) {
			if (int(gl_FragCoord.x) / 20 % 2 == 0) {
				FragColor = vec4(ourColor, 1.0f);
			} else {
				discard;
			}
		} else {
			FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	FragColor = vec4(ourColor, 1.0f);
}
