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

// Cache serialization functions
void writeCachedMaterial(std::ofstream &file, const CachedMaterial &mat)
{
    writeString(file, mat.albedoPath);
    writeString(file, mat.normalPath);
    writeString(file, mat.metallicPath);
    writeString(file, mat.roughnessPath);
    writeString(file, mat.emissivePath);
    writeString(file, mat.aoPath);
    writeBinary(file, mat.diffuseColor);
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
    readBinary(file, mat.diffuseColor);
    readBinary(file, mat.metallic);
    readBinary(file, mat.roughness);
    return mat;
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

std::shared_ptr<Prepath::Texture> loadTexture(const std::string &path)
{
    using namespace Prepath;
    namespace fs = std::filesystem;

    std::string newPath = path.substr(1, path.size() - 1);

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

std::vector<std::shared_ptr<Prepath::Mesh>> loadModelWithCache(const std::string &path)
{
    using namespace Prepath;
    namespace fs = std::filesystem;

    PREPATH_LOG_INFO("Loading model with cache: {}", path.c_str());

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
        PREPATH_LOG_INFO("Loading model from cache");
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
                textureCache[texPath] = loadTexture(modelDir + texPath);
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

            return meshes;
        }
    }

    // Load from original file and create cache
    PREPATH_LOG_INFO("Loading model from file and creating cache");

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

    return meshes;
}