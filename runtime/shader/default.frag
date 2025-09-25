#version 330 core
out vec4 FragColor;

uniform vec3 uTint;

in vec3 WorldNormal;
in vec3 WorldPos;
in vec2 TexCoord;

void main() {
  vec3 color = uTint;
  FragColor = vec4(WorldNormal * 0.5 + 0.5, 1.0);
}
