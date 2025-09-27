#version 330 core
out vec4 FragColor;

uniform vec3 uTint;
uniform vec3 uLightDir;

uniform sampler2D uDepthMap;
uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uAOMap;

uniform int uDebugTexture; // 0 = normal render, >0 = debug view

in vec3 WorldNormal;
in vec3 WorldPos;
in vec4 WorldPosLightSpace;
in vec2 TexCoord;
in mat3 TBN;

vec3 getNormalFromMap() {
  vec3 tangentNormal = texture(uNormalMap, TexCoord).rgb;
  tangentNormal = tangentNormal * 2.0 - 1.0;
  return normalize(TBN * tangentNormal);
}

float ShadowCalculationPCF(vec4 fragPosLightSpace) {
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;
  float closestDepth = texture(uDepthMap, projCoords.xy).r;
  float currentDepth = projCoords.z;
  float bias = max(0.07 * (1.0 - dot(getNormalFromMap(), uLightDir)), 0.005);

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
  if(uDebugTexture == 1) {
    FragColor = vec4(texture(uAlbedoMap, TexCoord).rgb, 1.0);
    return;
  }
  if(uDebugTexture == 2) {
    vec3 normal = getNormalFromMap();
    FragColor = vec4(normal * 0.5 + 0.5, 1.0);
    return;
  }
  if(uDebugTexture == 3) {
    FragColor = vec4(vec3(texture(uRoughnessMap, TexCoord).g), 1.0);
    return;
  }
  if(uDebugTexture == 4) {
    FragColor = vec4(vec3(texture(uMetallicMap, TexCoord).b), 1.0);
    return;
  }
  if(uDebugTexture == 5) {
    FragColor = vec4(vec3(texture(uAOMap, TexCoord).r), 1.0);
    return;
  }
  if(uDebugTexture == 6) {
    float shadow = ShadowCalculationPCF(WorldPosLightSpace);
    FragColor = vec4(vec3(shadow), 1.0);
    return;
  }

  vec3 normal = getNormalFromMap();
  vec3 color = texture(uAlbedoMap, TexCoord).rgb * uTint;
  float shadow = ShadowCalculationPCF(WorldPosLightSpace);
  vec3 directLight = max(dot(normal, uLightDir), 0.0) * (1.0 - shadow) * color;
  vec3 indirectLight = 0.2 * (1.0 - max(dot(normal, uLightDir), 0.0)) * color;
  vec3 ambient = 0.05 * color;

  vec3 lighting = ambient + directLight + indirectLight;
  FragColor = vec4(lighting, 1.0);
}
