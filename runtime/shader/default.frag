#version 330 core
out vec4 FragColor;

uniform vec3 uTint;
uniform vec3 uLightDir;
uniform vec3 uCameraPos;

uniform sampler2D uDepthMap;
uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uAOMap;

uniform int uDebugTexture; // 0 = normal render, >0 = debug view

in vec3 WorldPos;
in vec2 TexCoord;
in vec4 WorldPosLightSpace;
in mat3 TBN;
flat in int vTriangleID;

// ----------------------------------------------------------------------------
// Shadow Calculation with PCF
float ShadowCalculationPCF(vec4 fragPosLightSpace) {
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;
  if(projCoords.z > 1.0)
    return 0.0;

  float currentDepth = projCoords.z;
  float bias = 0.005; // Simple fixed bias

  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(uDepthMap, 0);
  for(int x = -1; x <= 1; ++x) {
    for(int y = -1; y <= 1; ++y) {
      float pcfDepth = texture(uDepthMap, projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
  }
  return shadow / 9.0;
}

// ----------------------------------------------------------------------------
// PBR helper functions
float DistributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float num = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = 3.14159 * denom * denom;

  return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

  float num = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);
  return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
// Normal Mapping
vec3 getNormalFromMap() {
  vec3 tangentNormal = texture(uNormalMap, TexCoord).rgb;
  tangentNormal = tangentNormal * 2.0 - 1.0; // [0,1] → [-1,1]
  return normalize(TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
void main() {
    // Debug modes
  if(uDebugTexture == 1) {
    FragColor = vec4(TexCoord, 0.0, 1.0);
    return;
  }
  if(uDebugTexture == 2) {
    FragColor = vec4(texture(uAlbedoMap, TexCoord).rgb, 1.0);
    return;
  }
  if(uDebugTexture == 3) {
    vec3 normal = getNormalFromMap();
    FragColor = vec4(normal * 0.5 + 0.5, 1.0);
    return;
  }
  if(uDebugTexture == 4) {
    FragColor = vec4(vec3(texture(uRoughnessMap, TexCoord).g), 1.0);
    return;
  }
  if(uDebugTexture == 5) {
    FragColor = vec4(vec3(texture(uMetallicMap, TexCoord).b), 1.0);
    return;
  }
  if(uDebugTexture == 6) {
    FragColor = vec4(vec3(texture(uAOMap, TexCoord).r), 1.0);
    return;
  }
  if(uDebugTexture == 7) {
    float shadow = ShadowCalculationPCF(WorldPosLightSpace);
    FragColor = vec4(vec3(shadow), 1.0);
    return;
  }
  if(uDebugTexture == 8) {
    int id = vTriangleID;
    float r = float((id * 37) % 255) / 255.0;
    float g = float((id * 59) % 255) / 255.0;
    float b = float((id * 83) % 255) / 255.0;
    FragColor = vec4(r, g, b, 1.0);
    return;
  }

    // ----------------------------------------------------------------------------
    // PBR Lighting
  vec3 albedo = texture(uAlbedoMap, TexCoord).rgb * uTint;
  float roughness = texture(uRoughnessMap, TexCoord).g;
  float metallic = texture(uMetallicMap, TexCoord).b;
  float ao = texture(uAOMap, TexCoord).r;

  vec3 N = getNormalFromMap();
  vec3 V = normalize(uCameraPos - WorldPos);
  vec3 L = normalize(uLightDir);
  vec3 H = normalize(V + L);

    // Cook–Torrance BRDF
  float NDF = DistributionGGX(N, H, roughness);
  float G = GeometrySmith(N, V, L, roughness);
  vec3 F0 = mix(vec3(0.04), albedo, metallic);
  vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

  vec3 numerator = NDF * G * F;
  float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
  vec3 specular = numerator / denominator;

  vec3 kS = F;
  vec3 kD = (1.0 - kS) * (1.0 - metallic);
  vec3 diffuse = kD * albedo / 3.14159;

  float NdotL = max(dot(N, L), 0.0);
  float shadow = ShadowCalculationPCF(WorldPosLightSpace);

  vec3 Lo = (diffuse + specular) * NdotL * (1.0 - shadow);
  vec3 ambient = 0.03 * albedo * ao;

  vec3 color = ambient + Lo;
  color = pow(color, vec3(1.0 / 2.2));
  FragColor = vec4(color, 1.0);
}
