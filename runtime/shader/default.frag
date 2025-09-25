#version 330 core
out vec4 FragColor;

uniform vec3 uTint;
uniform vec3 uLightDir;
uniform sampler2D uDepthMap;

in vec3 WorldNormal;
in vec3 WorldPos;
in vec4 WorldPosLightSpace;
in vec2 TexCoord;

float ShadowCalculationPCF(vec4 fragPosLightSpace) {
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;
  float closestDepth = texture(uDepthMap, projCoords.xy).r;
  float currentDepth = projCoords.z;
  float bias = max(0.07 * (1.0 - dot(WorldNormal, uLightDir)), 0.005);

  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(uDepthMap, 0);
  for(int x = -1; x <= 1; ++x) {
    for(int y = -1; y <= 1; ++y) {
      float pcfDepth = texture(uDepthMap, projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;
  return shadow;
}

void main() {
  vec3 color = WorldNormal * 0.5 + 0.5;
  float shadow = ShadowCalculationPCF(WorldPosLightSpace);
  float ambientOcclusion = dot(WorldNormal, vec3(0, 1, 0)) * 0.5 + 0.5;
  vec3 ambient = vec3(0.1) * ambientOcclusion;
  vec3 lighting = (ambient + (1.0 - shadow)) * color;
  FragColor = vec4(lighting, 1.0);
}
