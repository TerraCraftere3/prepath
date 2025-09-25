#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat3 uNormalMatrix;

out vec3 WorldNormal;
out vec3 WorldPos;
out vec2 TexCoord;

void main() {
    mat4 uMVP = uProjection * uView * uModel;

    gl_Position = uMVP * vec4(aPos, 1.0);

    WorldPos = vec3(uModel * vec4(aPos, 1.0));
    WorldNormal = normalize(uNormalMatrix * aNormal);
    TexCoord = aTexCoord;
}