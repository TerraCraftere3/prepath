#version 330 core
in vec4 FragPos;

uniform vec3 uCameraPos;
uniform float uRange;

void main() {
    float lightDistance = length(FragPos.xyz - uCameraPos);

    gl_FragDepth = clamp(lightDistance / uRange, 0.0, 1.0);
}