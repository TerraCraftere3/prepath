#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
uniform mat4 uLightSpace;
uniform mat3 uNormalMatrix;

out vec3 WorldPos;
out vec2 TexCoord;
out vec4 WorldPosLightSpace;
out mat3 TBN;

void main() {
    mat4 uMVP = uProjection * uView * uModel;
    gl_Position = uMVP * vec4(aPos, 1.0);

    WorldPos = vec3(uModel * vec4(aPos, 1.0));
    TexCoord = aTexCoord;
    WorldPosLightSpace = uLightSpace * vec4(WorldPos, 1.0);

    // Construct TBN matrix
    vec3 T = normalize(uNormalMatrix * aTangent);
    vec3 B = normalize(uNormalMatrix * aBitangent);
    vec3 N = normalize(uNormalMatrix * aNormal);
    TBN = mat3(T, B, N);
}
