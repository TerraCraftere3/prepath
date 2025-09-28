#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uModel;

out vec4 WorldPos;

void main() {
    WorldPos = uModel * vec4(aPos, 1.0);
}