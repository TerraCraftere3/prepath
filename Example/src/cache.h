#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Prepath/Lib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "lodepng.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <set>

// Model cache header structures
struct CachedMaterial
{
    std::string albedoPath;
    std::string normalPath;
    std::string metallicPath;
    std::string roughnessPath;
    std::string emissivePath;
    std::string aoPath;
    // Add other material properties as needed
    glm::vec3 diffuseColor = glm::vec3(1.0f);
    float metallic = 0.0f;
    float roughness = 1.0f;
};

struct CachedLight
{
    enum Type
    {
        DIRECTIONAL,
        POINT,
        SPOT
    } type;
    glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    float range = 100.0f;    // For point/spot lights
    float innerCone = 30.0f; // For spot lights
    float outerCone = 45.0f; // For spot lights
};

struct CachedMeshData
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    uint32_t materialIndex;
};

struct CachedModelData
{
    std::vector<CachedMeshData> meshes;
    std::vector<CachedMaterial> materials;
    std::vector<CachedLight> lights;
    std::vector<std::string> allTexturePaths; // ALL textures found in model
};

// Binary serialization helpers
template <typename T>
void writeBinary(std::ofstream &file, const T &value)
{
    file.write(reinterpret_cast<const char *>(&value), sizeof(T));
}

template <typename T>
void readBinary(std::ifstream &file, T &value)
{
    file.read(reinterpret_cast<char *>(&value), sizeof(T));
}

void writeVec3(std::ofstream &file, const glm::vec3 &vec)
{
    writeBinary(file, vec.x);
    writeBinary(file, vec.y);
    writeBinary(file, vec.z);
}

glm::vec3 readVec3(std::ifstream &file)
{
    glm::vec3 vec;
    readBinary(file, vec.x);
    readBinary(file, vec.y);
    readBinary(file, vec.z);
    return vec;
}

void writeString(std::ofstream &file, const std::string &str)
{
    uint32_t size = static_cast<uint32_t>(str.size());
    writeBinary(file, size);
    if (size > 0)
        file.write(str.data(), size);
}

std::string readString(std::ifstream &file)
{
    uint32_t size;
    readBinary(file, size);
    if (size == 0)
        return "";

    std::string str(size, '\0');
    file.read(str.data(), size);
    return str;
}

template <typename T>
void writeVector(std::ofstream &file, const std::vector<T> &vec)
{
    uint32_t size = static_cast<uint32_t>(vec.size());
    writeBinary(file, size);
    if (size > 0)
        file.write(reinterpret_cast<const char *>(vec.data()), size * sizeof(T));
}

template <typename T>
std::vector<T> readVector(std::ifstream &file)
{
    uint32_t size;
    readBinary(file, size);
    if (size == 0)
        return {};

    std::vector<T> vec(size);
    file.read(reinterpret_cast<char *>(vec.data()), size * sizeof(T));
    return vec;
}

inline glm::mat4 convertAssimpMatrix(const aiMatrix4x4 &m)
{
    glm::mat4 result;
    result[0][0] = m.a1;
    result[1][0] = m.a2;
    result[2][0] = m.a3;
    result[3][0] = m.a4;
    result[0][1] = m.b1;
    result[1][1] = m.b2;
    result[2][1] = m.b3;
    result[3][1] = m.b4;
    result[0][2] = m.c1;
    result[1][2] = m.c2;
    result[2][2] = m.c3;
    result[3][2] = m.c4;
    result[0][3] = m.d1;
    result[1][3] = m.d2;
    result[2][3] = m.d3;
    result[3][3] = m.d4;
    return result;
}

// Cache serialization functions
void writeCachedMaterial(std::ofstream &file, const CachedMaterial &mat)
{
    writeString(file, mat.albedoPath);
    writeString(file, mat.normalPath);
    writeString(file, mat.metallicPath);
    writeString(file, mat.roughnessPath);
    writeString(file, mat.emissivePath);
    writeString(file, mat.aoPath);
    writeVec3(file, mat.diffuseColor);
    writeBinary(file, mat.metallic);
    writeBinary(file, mat.roughness);
}

CachedMaterial readCachedMaterial(std::ifstream &file)
{
    CachedMaterial mat;
    mat.albedoPath = readString(file);
    mat.normalPath = readString(file);
    mat.metallicPath = readString(file);
    mat.roughnessPath = readString(file);
    mat.emissivePath = readString(file);
    mat.aoPath = readString(file);
    mat.diffuseColor = readVec3(file);
    readBinary(file, mat.metallic);
    readBinary(file, mat.roughness);
    return mat;
}

void writeCachedLight(std::ofstream &file, const CachedLight &light)
{
    writeBinary(file, static_cast<uint32_t>(light.type));
    writeVec3(file, light.position);
    writeVec3(file, light.direction);
    writeVec3(file, light.color);
    writeBinary(file, light.intensity);
    writeBinary(file, light.range);
    writeBinary(file, light.innerCone);
    writeBinary(file, light.outerCone);
}

CachedLight readCachedLight(std::ifstream &file)
{
    CachedLight light;
    uint32_t type;
    readBinary(file, type);
    light.type = static_cast<CachedLight::Type>(type);
    light.position = readVec3(file);
    light.direction = readVec3(file);
    light.color = readVec3(file);
    readBinary(file, light.intensity);
    readBinary(file, light.range);
    readBinary(file, light.innerCone);
    readBinary(file, light.outerCone);
    return light;
}

void writeCachedMeshData(std::ofstream &file, const CachedMeshData &mesh)
{
    writeVector(file, mesh.positions);
    writeVector(file, mesh.normals);
    writeVector(file, mesh.texCoords);
    writeVector(file, mesh.tangents);
    writeVector(file, mesh.bitangents);
    writeBinary(file, mesh.materialIndex);
}

CachedMeshData readCachedMeshData(std::ifstream &file)
{
    CachedMeshData mesh;
    mesh.positions = readVector<glm::vec3>(file);
    mesh.normals = readVector<glm::vec3>(file);
    mesh.texCoords = readVector<glm::vec2>(file);
    mesh.tangents = readVector<glm::vec3>(file);
    mesh.bitangents = readVector<glm::vec3>(file);
    readBinary(file, mesh.materialIndex);
    return mesh;
}

void writeCachedModelData(std::ofstream &file, const CachedModelData &model)
{
    // Write meshes
    uint32_t meshCount = static_cast<uint32_t>(model.meshes.size());
    writeBinary(file, meshCount);
    for (const auto &mesh : model.meshes)
    {
        writeCachedMeshData(file, mesh);
    }

    // Write materials
    uint32_t materialCount = static_cast<uint32_t>(model.materials.size());
    writeBinary(file, materialCount);
    for (const auto &mat : model.materials)
    {
        writeCachedMaterial(file, mat);
    }

    uint32_t lightCount = static_cast<uint32_t>(model.lights.size());
    writeBinary(file, lightCount);
    for (const auto &light : model.lights)
    {
        writeCachedLight(file, light);
    }

    // Write all texture paths
    uint32_t textureCount = static_cast<uint32_t>(model.allTexturePaths.size());
    writeBinary(file, textureCount);
    for (const auto &texPath : model.allTexturePaths)
    {
        writeString(file, texPath);
    }
}

CachedModelData readCachedModelData(std::ifstream &file)
{
    CachedModelData model;

    // Read meshes
    uint32_t meshCount;
    readBinary(file, meshCount);
    model.meshes.reserve(meshCount);
    for (uint32_t i = 0; i < meshCount; ++i)
    {
        model.meshes.push_back(readCachedMeshData(file));
    }

    // Read materials
    uint32_t materialCount;
    readBinary(file, materialCount);
    model.materials.reserve(materialCount);
    for (uint32_t i = 0; i < materialCount; ++i)
    {
        model.materials.push_back(readCachedMaterial(file));
    }

    // Read lights
    uint32_t lightCount;
    readBinary(file, lightCount);
    model.lights.reserve(lightCount);
    for (uint32_t i = 0; i < lightCount; ++i)
    {
        model.lights.push_back(readCachedLight(file));
    }

    // Read all texture paths
    uint32_t textureCount;
    readBinary(file, textureCount);
    model.allTexturePaths.reserve(textureCount);
    for (uint32_t i = 0; i < textureCount; ++i)
    {
        model.allTexturePaths.push_back(readString(file));
    }

    return model;
}

// Helper function to find light nodes in the scene graph
void findLightNodes(aiNode *node, const aiScene *scene, const glm::mat4 &parentTransform,
                    std::vector<CachedLight> &lights)
{
    glm::mat4 nodeTransform = parentTransform * convertAssimpMatrix(node->mTransformation);

    // Check if this node corresponds to a light
    for (unsigned int i = 0; i < scene->mNumLights; ++i)
    {
        aiLight *aiLight = scene->mLights[i];

        // Check if the light name matches the node name
        if (std::string(aiLight->mName.C_Str()) == std::string(node->mName.C_Str()))
        {
            CachedLight light;

            PREPATH_LOG_INFO("Processing light '{}' at node '{}'",
                             aiLight->mName.C_Str(), node->mName.C_Str());

            // Convert light type
            switch (aiLight->mType)
            {
            case aiLightSource_DIRECTIONAL:
                light.type = CachedLight::DIRECTIONAL;
                break;
            case aiLightSource_POINT:
                light.type = CachedLight::POINT;
                break;
            case aiLightSource_SPOT:
                light.type = CachedLight::SPOT;
                break;
            default:
                continue; // Skip unsupported light types
            }

            // For directional lights, position doesn't matter much, but we'll use the node position
            // For point/spot lights, use the node's world position
            glm::vec4 worldPos = nodeTransform * glm::vec4(aiLight->mPosition.x, aiLight->mPosition.y, aiLight->mPosition.z, 1.0f);
            light.position = glm::vec3(worldPos);

            // Transform direction by the node's rotation only (no translation)
            glm::vec3 localDir(aiLight->mDirection.x, aiLight->mDirection.y, aiLight->mDirection.z);
            glm::mat3 rotationMatrix = glm::mat3(nodeTransform);
            light.direction = glm::normalize(rotationMatrix * localDir);

            // Set color - try different color sources
            light.color = glm::vec3(0.0f); // Default to black

            // Try diffuse color first
            if (aiLight->mColorDiffuse.r > 0.0f || aiLight->mColorDiffuse.g > 0.0f || aiLight->mColorDiffuse.b > 0.0f)
            {
                light.color = glm::vec3(aiLight->mColorDiffuse.r, aiLight->mColorDiffuse.g, aiLight->mColorDiffuse.b);
            }
            // Try specular color if diffuse is black
            else if (aiLight->mColorSpecular.r > 0.0f || aiLight->mColorSpecular.g > 0.0f || aiLight->mColorSpecular.b > 0.0f)
            {
                light.color = glm::vec3(aiLight->mColorSpecular.r, aiLight->mColorSpecular.g, aiLight->mColorSpecular.b);
            }
            // Try ambient color if others are black
            else if (aiLight->mColorAmbient.r > 0.0f || aiLight->mColorAmbient.g > 0.0f || aiLight->mColorAmbient.b > 0.0f)
            {
                light.color = glm::vec3(aiLight->mColorAmbient.r, aiLight->mColorAmbient.g, aiLight->mColorAmbient.b);
            }
            // Default to white if all colors are black
            else
            {
                light.color = glm::vec3(1.0f, 1.0f, 1.0f);
            }

            // Calculate intensity from the maximum color component
            light.intensity = std::max({light.color.r, light.color.g, light.color.b});
            if (light.intensity > 1.0f)
            {
                // Normalize color and use the max as intensity
                light.color /= light.intensity;
            }
            else if (light.intensity < 0.01f)
            {
                // Very dim light, set to reasonable defaults
                light.intensity = 1.0f;
                light.color = glm::vec3(1.0f);
            }

            // Set range (for point and spot lights)
            if (light.type != CachedLight::DIRECTIONAL)
            {
                // Calculate range from attenuation
                float constant = std::max(0.001f, aiLight->mAttenuationConstant);
                float linear = aiLight->mAttenuationLinear;
                float quadratic = aiLight->mAttenuationQuadratic;

                // Calculate range where light contribution drops to ~1% (0.01)
                if (quadratic > 0.0001f)
                {
                    // Solve: intensity / (constant + linear*d + quadratic*d²) = 0.01
                    // This gives us: quadratic*d² + linear*d + (constant - intensity/0.01) = 0
                    float a = quadratic;
                    float b = linear;
                    float c = constant - (light.intensity / 0.01f);
                    float discriminant = b * b - 4.0f * a * c;

                    if (discriminant >= 0.0f)
                    {
                        light.range = (-b + sqrt(discriminant)) / (2.0f * a);
                        light.range = std::max(0.1f, std::min(light.range, 1000.0f)); // Clamp to reasonable values
                    }
                    else
                    {
                        light.range = 100.0f; // Default if calculation fails
                    }
                }
                else if (linear > 0.0001f)
                {
                    // Linear attenuation only: intensity / (constant + linear*d) = 0.01
                    light.range = (light.intensity / 0.01f - constant) / linear;
                    light.range = std::max(0.1f, std::min(light.range, 1000.0f));
                }
                else
                {
                    // Constant attenuation only - light doesn't fall off
                    light.range = 100.0f; // Use default range
                }
            }

            // Set cone angles for spot lights
            if (light.type == CachedLight::SPOT)
            {
                light.innerCone = glm::degrees(aiLight->mAngleInnerCone);
                light.outerCone = glm::degrees(aiLight->mAngleOuterCone);

                // Ensure outer cone is larger than inner cone
                if (light.outerCone <= light.innerCone)
                {
                    light.outerCone = light.innerCone + 5.0f; // Add 5 degree falloff
                }
            }

            PREPATH_LOG_INFO("Light {}: pos({:.3f}, {:.3f}, {:.3f}), dir({:.3f}, {:.3f}, {:.3f}), color({:.3f}, {:.3f}, {:.3f}), intensity({:.3f})",
                             i,
                             light.position.x, light.position.y, light.position.z,
                             light.direction.x, light.direction.y, light.direction.z,
                             light.color.r, light.color.g, light.color.b,
                             light.intensity);

            lights.push_back(light);
            break; // Found the light for this node, no need to continue searching
        }
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        findLightNodes(node->mChildren[i], scene, nodeTransform, lights);
    }
}

// Updated extractLights function
std::vector<CachedLight> extractLights(const aiScene *scene, const glm::mat4 &transform = glm::mat4(1.0f))
{
    std::vector<CachedLight> lights;

    if (!scene->mRootNode)
    {
        PREPATH_LOG_WARN("Scene has no root node, cannot extract lights");
        return lights;
    }

    PREPATH_LOG_INFO("Extracting {} lights from scene", scene->mNumLights);

    // Traverse the scene graph to find light nodes
    findLightNodes(scene->mRootNode, scene, transform, lights);

    // If we didn't find any lights through node traversal, try the direct approach
    // This handles cases where lights might not be properly associated with nodes
    if (lights.empty() && scene->mNumLights > 0)
    {
        PREPATH_LOG_WARN("No lights found through scene graph traversal, trying direct extraction");

        for (unsigned int i = 0; i < scene->mNumLights; ++i)
        {
            aiLight *aiLight = scene->mLights[i];
            CachedLight light;

            // Convert light type
            switch (aiLight->mType)
            {
            case aiLightSource_DIRECTIONAL:
                light.type = CachedLight::DIRECTIONAL;
                break;
            case aiLightSource_POINT:
                light.type = CachedLight::POINT;
                break;
            case aiLightSource_SPOT:
                light.type = CachedLight::SPOT;
                break;
            default:
                continue;
            }

            // Use raw light data
            glm::vec4 pos = transform * glm::vec4(aiLight->mPosition.x, aiLight->mPosition.y, aiLight->mPosition.z, 1.0f);
            light.position = glm::vec3(pos);

            glm::vec3 dir = glm::mat3(transform) * glm::vec3(aiLight->mDirection.x, aiLight->mDirection.y, aiLight->mDirection.z);
            light.direction = glm::normalize(dir);

            // Set color with fallbacks
            light.color = glm::vec3(aiLight->mColorDiffuse.r, aiLight->mColorDiffuse.g, aiLight->mColorDiffuse.b);
            if (light.color == glm::vec3(0.0f))
            {
                light.color = glm::vec3(1.0f); // Default to white
            }

            light.intensity = 1.0f;
            light.range = 100.0f;

            if (light.type == CachedLight::SPOT)
            {
                light.innerCone = glm::degrees(aiLight->mAngleInnerCone);
                light.outerCone = glm::degrees(aiLight->mAngleOuterCone);
            }

            lights.push_back(light);
        }
    }

    PREPATH_LOG_INFO("Successfully extracted {} lights", lights.size());
    return lights;
}

// Extract ALL textures from materials, not just used ones
std::vector<std::string> extractAllTextures(const aiScene *scene, const std::string &modelDir)
{
    std::set<std::string> textureSet; // Use set to avoid duplicates

    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial *mat = scene->mMaterials[i];

        // Check all common texture types
        aiTextureType textureTypes[] = {
            aiTextureType_DIFFUSE,
            aiTextureType_NORMALS,
            aiTextureType_HEIGHT,
            aiTextureType_SPECULAR,
            aiTextureType_SHININESS,
            aiTextureType_METALNESS,
            aiTextureType_DIFFUSE_ROUGHNESS,
            aiTextureType_AMBIENT_OCCLUSION,
            aiTextureType_EMISSIVE,
            aiTextureType_LIGHTMAP,
            aiTextureType_REFLECTION};

        for (aiTextureType texType : textureTypes)
        {
            for (unsigned int j = 0; j < mat->GetTextureCount(texType); ++j)
            {
                aiString texPath;
                if (mat->GetTexture(texType, j, &texPath) == AI_SUCCESS)
                {
                    std::string fullPath = modelDir + "/" + texPath.C_Str();
                    textureSet.insert(fullPath);
                }
            }
        }
    }

    return std::vector<std::string>(textureSet.begin(), textureSet.end());
}

// Create cached material from assimp material
CachedMaterial createCachedMaterial(aiMaterial *aiMat, const std::string &modelDir)
{
    CachedMaterial mat;

    aiString texPath;

    // Extract texture paths
    if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
        mat.albedoPath = modelDir + "/" + texPath.C_Str();

    if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS)
        mat.normalPath = modelDir + "/" + texPath.C_Str();

    if (aiMat->GetTexture(aiTextureType_METALNESS, 0, &texPath) == AI_SUCCESS)
        mat.metallicPath = modelDir + "/" + texPath.C_Str();

    if (aiMat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &texPath) == AI_SUCCESS)
        mat.roughnessPath = modelDir + "/" + texPath.C_Str();

    if (aiMat->GetTexture(aiTextureType_EMISSIVE, 0, &texPath) == AI_SUCCESS)
        mat.emissivePath = modelDir + "/" + texPath.C_Str();

    if (aiMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texPath) == AI_SUCCESS)
        mat.aoPath = modelDir + "/" + texPath.C_Str();

    // Extract material properties
    aiColor3D color;
    if (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
        mat.diffuseColor = glm::vec3(color.r, color.g, color.b);

    float value;
    if (aiMat->Get(AI_MATKEY_METALLIC_FACTOR, value) == AI_SUCCESS)
        mat.metallic = value;

    if (aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, value) == AI_SUCCESS)
        mat.roughness = value;

    return mat;
}

std::shared_ptr<Prepath::Cubemap> loadSkybox(std::vector<std::string> faces)
{
    using namespace Prepath;
    unsigned char *data[6];
    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *tex_data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (tex_data)
        {
            PREPATH_LOG_INFO("Loaded cubemap face: {}", faces[i].c_str());
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
            data[i] = tex_data;
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(tex_data);
        }
    }
    PREPATH_LOG_INFO("Loaded cubemap ({}x{})", width, height);
    return Cubemap::generateTexture(data, width, height, nrChannels);
}

std::shared_ptr<Prepath::Texture> loadTexture(const std::string &path)
{
    using namespace Prepath;
    namespace fs = std::filesystem;

    std::string newPath = path;

    PREPATH_LOG_INFO("Loading texture: {}", newPath.c_str());

    std::string binPath = newPath + ".bin";
    int width = 0, height = 0;

    if (!fs::exists(newPath))
    {
        PREPATH_LOG_INFO("Texture doesnt exist: {}", newPath.c_str());
    }

    // --- Try loading cached .bin ---
    if (fs::exists(binPath))
    {
        std::ifstream binFile(binPath, std::ios::binary);
        if (!binFile)
            PREPATH_LOG_FATAL("Failed to open cached texture: {}", binPath.c_str());

        binFile.read(reinterpret_cast<char *>(&width), sizeof(int));
        binFile.read(reinterpret_cast<char *>(&height), sizeof(int));

        std::vector<unsigned char> data(width * height * 4);
        binFile.read(reinterpret_cast<char *>(data.data()), data.size());
        binFile.close();

        return Texture::generateTexture(data.data(), width, height, 4);
    }

    // --- Decode original image ---
    unsigned char *data = stbi_load(newPath.c_str(), &width, &height, nullptr, 4); // force RGBA
    if (!data)
    {
        PREPATH_LOG_FATAL("Failed to load texture: {}", newPath.c_str());
        unsigned char fallback[4] = {255, 0, 255, 255};
        return Texture::generateTexture(fallback, 1, 1, 4);
    }

    auto tex = Texture::generateTexture(data, width, height, 4);

    // --- Write cache .bin ---
    std::ofstream binFile(binPath, std::ios::binary);
    if (binFile)
    {
        binFile.write(reinterpret_cast<const char *>(&width), sizeof(int));
        binFile.write(reinterpret_cast<const char *>(&height), sizeof(int));
        binFile.write(reinterpret_cast<const char *>(data), width * height * 4);
        binFile.close();
    }
    else
    {
        PREPATH_LOG_WARN("Failed to write cached texture: {}", binPath.c_str());
    }

    stbi_image_free(data);
    return tex;
}

// VERY SKETCHY METHODS

struct MeshData
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
};

void processMesh(aiMesh *mesh,
                 aiMaterial *aiMat,
                 const glm::mat4 &transform,
                 std::unordered_map<aiMaterial *, MeshData> &groupedMeshes)
{
    MeshData &data = groupedMeshes[aiMat];

    for (unsigned int f = 0; f < mesh->mNumFaces; f++)
    {
        const aiFace &face = mesh->mFaces[f];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            unsigned int idx = face.mIndices[j];

            // Position
            aiVector3D v = mesh->mVertices[idx];
            glm::vec4 pos = transform * glm::vec4(v.x, v.y, v.z, 1.0f);
            data.positions.emplace_back(pos.x, pos.y, pos.z);

            // Normal
            if (mesh->HasNormals())
            {
                aiVector3D n = mesh->mNormals[idx];
                glm::vec3 normal = glm::mat3(transform) * glm::vec3(n.x, n.y, n.z);
                data.normals.push_back(glm::normalize(normal));
            }
            else
            {
                data.normals.emplace_back(0.0f, 0.0f, 1.0f);
            }

            // UV
            if (mesh->HasTextureCoords(0))
            {
                aiVector3D uv = mesh->mTextureCoords[0][idx];
                data.texCoords.emplace_back(uv.x, uv.y);
            }
            else
            {
                data.texCoords.emplace_back(0.0f, 0.0f);
            }

            if (mesh->HasTangentsAndBitangents())
            {
                aiVector3D t = mesh->mTangents[idx];
                aiVector3D b = mesh->mBitangents[idx];
                glm::vec3 tangent = glm::mat3(transform) * glm::vec3(t.x, t.y, t.z);
                glm::vec3 bitangent = glm::mat3(transform) * glm::vec3(b.x, b.y, b.z);
                data.tangents.push_back(glm::normalize(tangent));
                data.bitangents.push_back(glm::normalize(bitangent));
            }
            else
            {
                data.tangents.emplace_back(1.0f, 0.0f, 0.0f);
                data.bitangents.emplace_back(0.0f, 1.0f, 0.0f);
            }
        }
    }
}

void processNode(aiNode *node,
                 const aiScene *scene,
                 const glm::mat4 &parentTransform,
                 std::unordered_map<aiMaterial *, MeshData> &groupedMeshes)
{
    glm::mat4 transform = parentTransform * convertAssimpMatrix(node->mTransformation);

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        aiMaterial *aiMat = scene->mMaterials[mesh->mMaterialIndex];
        processMesh(mesh, aiMat, transform, groupedMeshes);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, transform, groupedMeshes);
    }
}

std::pair<std::vector<std::shared_ptr<Prepath::Mesh>>, std::vector<CachedLight>> loadModelWithCache(const std::string &path)
{
    using namespace Prepath;
    namespace fs = std::filesystem;

    std::string cacheFile = path + ".modelcache";
    std::string modelDir = fs::path(path).parent_path().string();

    // Check if cache exists and is newer than model file
    bool useCache = false;
    if (fs::exists(cacheFile))
    {
        auto cacheTime = fs::last_write_time(cacheFile);
        auto modelTime = fs::last_write_time(path);
        useCache = (cacheTime >= modelTime);
    }

    CachedModelData cachedData;
    std::vector<std::shared_ptr<Mesh>> meshes;

    if (useCache)
    {
        // Load from cache
        PREPATH_LOG_INFO("Loading model from cache: {}", path.c_str());
        std::ifstream cacheStream(cacheFile, std::ios::binary);
        if (cacheStream)
        {
            cachedData = readCachedModelData(cacheStream);
            cacheStream.close();

            // Preload ALL textures (not just used ones)
            PREPATH_LOG_INFO("Preloading {} textures", cachedData.allTexturePaths.size());
            std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;
            for (const auto &texPath : cachedData.allTexturePaths)
            {
                textureCache[texPath] = loadTexture(texPath);
            }

            // Create materials
            std::vector<std::shared_ptr<Material>> materials;
            materials.reserve(cachedData.materials.size());

            for (const auto &cachedMat : cachedData.materials)
            {
                auto mat = Material::generateMaterial();

                if (!cachedMat.albedoPath.empty() && textureCache.count(cachedMat.albedoPath))
                    mat->albedo = textureCache[cachedMat.albedoPath];

                if (!cachedMat.normalPath.empty() && textureCache.count(cachedMat.normalPath))
                    mat->normal = textureCache[cachedMat.normalPath];

                if (!cachedMat.roughnessPath.empty() && textureCache.count(cachedMat.roughnessPath))
                    mat->roughness = textureCache[cachedMat.roughnessPath];

                if (!cachedMat.metallicPath.empty() && textureCache.count(cachedMat.metallicPath))
                    mat->metal = textureCache[cachedMat.metallicPath];

                if (!cachedMat.aoPath.empty() && textureCache.count(cachedMat.aoPath))
                    mat->ao = textureCache[cachedMat.aoPath];

                materials.push_back(mat);
            }

            // Create meshes
            meshes.reserve(cachedData.meshes.size());
            for (const auto &cachedMesh : cachedData.meshes)
            {
                auto mesh = Mesh::generateMesh(
                    cachedMesh.positions,
                    cachedMesh.normals,
                    cachedMesh.texCoords,
                    &cachedMesh.tangents,
                    &cachedMesh.bitangents);

                if (cachedMesh.materialIndex < materials.size())
                    mesh->material = materials[cachedMesh.materialIndex];

                meshes.push_back(mesh);
            }

            return std::make_pair(meshes, cachedData.lights);
        }
    }

    // Load from original file and create cache
    PREPATH_LOG_INFO("Loading model and creating cache: {}", path.c_str());

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_FlipUVs);

    if (!scene || !scene->HasMeshes())
    {
        PREPATH_LOG_FATAL("Failed to load model: {}", path.c_str());
        return {};
    }

    // Extract ALL textures from the model
    cachedData.allTexturePaths = extractAllTextures(scene, modelDir);
    PREPATH_LOG_INFO("Found {} unique textures in model", cachedData.allTexturePaths.size());

    // Extract ALL lights from the model
    glm::mat4 rootTransform = convertAssimpMatrix(scene->mRootNode->mTransformation);
    cachedData.lights = extractLights(scene, rootTransform);
    PREPATH_LOG_INFO("Found {} lights in model", cachedData.lights.size());

    // Preload all textures
    std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;
    for (const auto &texPath : cachedData.allTexturePaths)
    {
        textureCache[texPath] = loadTexture(texPath);
    }

    // Process materials
    std::unordered_map<aiMaterial *, uint32_t> materialIndexMap;
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial *aiMat = scene->mMaterials[i];
        materialIndexMap[aiMat] = i;
        cachedData.materials.push_back(createCachedMaterial(aiMat, modelDir));
    }

    // Process meshes (using your existing logic)
    std::unordered_map<aiMaterial *, MeshData> groupedMeshes;
    processNode(scene->mRootNode, scene, glm::mat4(1.0f), groupedMeshes);

    // Convert to cached format and create mesh objects
    std::vector<std::shared_ptr<Material>> materials;
    materials.reserve(cachedData.materials.size());

    for (const auto &cachedMat : cachedData.materials)
    {
        auto mat = Material::generateMaterial();

        if (!cachedMat.albedoPath.empty() && textureCache.count(cachedMat.albedoPath))
            mat->albedo = textureCache[cachedMat.albedoPath];

        if (!cachedMat.normalPath.empty() && textureCache.count(cachedMat.normalPath))
            mat->normal = textureCache[cachedMat.normalPath];

        materials.push_back(mat);
    }

    for (auto &[aiMat, meshData] : groupedMeshes)
    {
        CachedMeshData cachedMesh;
        cachedMesh.positions = meshData.positions;
        cachedMesh.normals = meshData.normals;
        cachedMesh.texCoords = meshData.texCoords;
        cachedMesh.tangents = meshData.tangents;
        cachedMesh.bitangents = meshData.bitangents;
        cachedMesh.materialIndex = materialIndexMap[aiMat];

        cachedData.meshes.push_back(cachedMesh);

        auto mesh = Mesh::generateMesh(
            meshData.positions,
            meshData.normals,
            meshData.texCoords,
            &meshData.tangents,
            &meshData.bitangents);

        mesh->material = materials[cachedMesh.materialIndex];
        meshes.push_back(mesh);
    }

    // Write cache file
    std::ofstream cacheStream(cacheFile, std::ios::binary);
    if (cacheStream)
    {
        writeCachedModelData(cacheStream, cachedData);
        cacheStream.close();
        PREPATH_LOG_INFO("Model cache written successfully");
    }
    else
    {
        PREPATH_LOG_WARN("Failed to write model cache: {}", cacheFile.c_str());
    }

    return std::make_pair(meshes, cachedData.lights);
}