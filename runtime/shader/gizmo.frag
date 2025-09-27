#version 330 core
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec3 uTint;

in vec2 TexCoords;

void main() {
    vec4 texColor = texture(uTexture, TexCoords);
    FragColor = vec4(texColor.rgb * uTint, texColor.a);
}
