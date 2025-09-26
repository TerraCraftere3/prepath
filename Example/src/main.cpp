#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Prepath/Lib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "lodepng.h"

struct CameraController
{
    float moveSpeed = 20.0f;
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    void updateVectors(const glm::vec3 &position, const glm::vec3 &target)
    {
        front = glm::normalize(target - position);
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};

CameraController cameraController;

bool keys[1024] = {false};

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void processInput(Prepath::RenderSettings &settings, float deltaTime)
{
    float velocity = cameraController.moveSpeed * deltaTime;
    float rotationSpeed = 90.0f * deltaTime; // degrees per second

    glm::vec3 target = settings.cam.Position + settings.cam.Front;
    cameraController.updateVectors(settings.cam.Position, target);

    // Movement
    if (keys[GLFW_KEY_W])
        settings.cam.Position += cameraController.front * velocity;
    if (keys[GLFW_KEY_S])
        settings.cam.Position -= cameraController.front * velocity;
    if (keys[GLFW_KEY_A])
        settings.cam.Position -= cameraController.right * velocity;
    if (keys[GLFW_KEY_D])
        settings.cam.Position += cameraController.right * velocity;
    if (keys[GLFW_KEY_SPACE])
        settings.cam.Position += cameraController.worldUp * velocity;
    if (keys[GLFW_KEY_LEFT_SHIFT])
        settings.cam.Position -= cameraController.worldUp * velocity;

    // Rotation using arrow keys
    if (keys[GLFW_KEY_UP])
        settings.cam.Pitch += rotationSpeed;
    if (keys[GLFW_KEY_DOWN])
        settings.cam.Pitch -= rotationSpeed;
    if (keys[GLFW_KEY_LEFT])
        settings.cam.Yaw -= rotationSpeed;
    if (keys[GLFW_KEY_RIGHT])
        settings.cam.Yaw += rotationSpeed;

    // Clamp pitch to avoid gimbal lock
    if (settings.cam.Pitch > 89.0f)
        settings.cam.Pitch = 89.0f;
    if (settings.cam.Pitch < -89.0f)
        settings.cam.Pitch = -89.0f;

    settings.cam.updateCameraVectors();
}

std::shared_ptr<Prepath::Texture> loadTexture(const std::string &path)
{
    using namespace Prepath;
    namespace fs = std::filesystem;

    PREPATH_LOG_INFO("Loading texture: {}", path.c_str());

    std::string binPath = path + ".bin";
    int width = 0, height = 0;

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
    unsigned char *data = stbi_load(path.c_str(), &width, &height, nullptr, 4); // force RGBA
    if (!data)
    {
        PREPATH_LOG_FATAL("Failed to load texture: {}", path.c_str());
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

std::vector<std::shared_ptr<Prepath::Mesh>> loadModel(const std::string &path)
{
    using namespace Prepath;
    PREPATH_LOG_INFO("Loading model {}", path.c_str());

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_FlipUVs);

    if (!scene || !scene->HasMeshes())
    {
        std::cerr << "Assimp error loading " << path << ": "
                  << importer.GetErrorString() << std::endl;
        return {};
    }

    std::unordered_map<aiMaterial *, std::shared_ptr<Material>> materialCache;
    std::unordered_map<aiMaterial *, MeshData> groupedMeshes;

    processNode(scene->mRootNode, scene, glm::mat4(1.0f), groupedMeshes);

    std::vector<std::shared_ptr<Mesh>> meshes;

    for (auto &[aiMat, data] : groupedMeshes)
    {
        auto prepathMesh = Mesh::generateMesh(
            data.positions, data.normals, data.texCoords,
            &data.tangents, &data.bitangents);

        std::shared_ptr<Material> mat;

        auto it = materialCache.find(aiMat);
        if (it != materialCache.end())
        {
            mat = it->second;
        }
        else
        {
            mat = Material::generateMaterial();

            aiString texPath;
            if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
                mat->albedo = loadTexture(texPath.C_Str());

            if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS ||
                aiMat->GetTexture(aiTextureType_HEIGHT, 0, &texPath) == AI_SUCCESS)
                mat->normal = loadTexture(texPath.C_Str());

            materialCache[aiMat] = mat;
        }

        prepathMesh->material = mat;
        meshes.push_back(prepathMesh);
    }

    return meshes;
}

int main()
{
    // ---- INIT CODE ----
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(800, 600, "Prepath Demo", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    // glfwSwapInterval(0); // Disable VSync
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ---- DEMO CONTEXT ----
    auto &ctx = Prepath::Context::getGlobalContext();

    ctx.setLogger(Prepath::LogLevel::Info, [](const std::string &msg)
                  { spdlog::info("{}", msg); });
    ctx.setLogger(Prepath::LogLevel::Warn, [](const std::string &msg)
                  { spdlog::warn("{}", msg); });
    ctx.setLogger(Prepath::LogLevel::Error, [](const std::string &msg)
                  { spdlog::error("{}", msg); });
    ctx.setLogger(Prepath::LogLevel::Fatal, [](const std::string &msg)
                  { spdlog::critical("{}", msg); });
    ctx.setShaderPath("shader");

    // ---- DEMO SETUP ----
    auto renderer = Prepath::Renderer();
    auto scene = Prepath::Scene();
    auto settings = Prepath::RenderSettings();
    settings.cam.Position = glm::vec3(0, 3.0f, 4.0f);
    settings.cam.updateCameraVectors();

    auto sponza_meshes = loadModel("NewSponza_Main_glTF_003.gltf");
    for (auto mesh : sponza_meshes)
    {
        mesh->modelMatrix = glm::rotate(mesh->modelMatrix, glm::radians(90.0f), glm::vec3(.0f, 1.0f, .0f));
        scene.addMesh(mesh);
    }

    scene.lightDir = glm::vec3(0.1f, 0.8f, 0.5f);

    // ---- RUNTIME CODE ----
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();
        processInput(settings, deltaTime);
        glfwGetWindowSize(window, &settings.width, &settings.height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({20, 20});
        ImGui::SetNextWindowSizeConstraints(
            ImVec2(0, settings.height - 40),
            ImVec2(FLT_MAX, settings.height - 40));
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("Debug");
        ImGui::SeparatorText("Statistics");
        auto stats = renderer.getStatistics();
        ImGui::Text("Draw Calls: %d", stats.drawCallCount);
        {
            std::stringstream ss;
            ss.imbue(std::locale(""));
            ss << stats.triangleCount;
            ImGui::Text("Triangles: %s", ss.str().c_str());
        }
        {
            std::stringstream ss;
            ss.imbue(std::locale(""));
            ss << stats.vertexCount;
            ImGui::Text("Vertices: %s", ss.str().c_str());
        }
        ImGui::Text("Delta Time: %.3f ms", deltaTime);
        ImGui::SeparatorText("Settings");
        ImGui::Checkbox("Display Wireframe", &settings.wireframe);
        ImGui::Checkbox("Display Bounds", &settings.bounds);
        ImGui::Checkbox("Culling", &settings.culling);
        ImGui::SeparatorText("Camera");
        ImGui::SliderFloat("Speed", &cameraController.moveSpeed, 10.0f, 50.0f);
        ImGui::Text("Position: %.1f, %.1f, %.1f", settings.cam.Position.x, settings.cam.Position.y, settings.cam.Position.z);
        ImGui::SeparatorText("Scene");
        int meshIndex = 0;
        for (auto &mesh : scene.getMeshes())
        {
            ImGui::PushID(meshIndex);

            std::stringstream ss;
            ss.imbue(std::locale(""));
            ss << mesh->getTriangleCount();

            if (ImGui::TreeNode("Mesh", "Mesh (%s Triangles)", ss.str().c_str()))
            {
                auto mat = mesh->material;
                ImGui::ColorEdit3("Tint", glm::value_ptr(mat->tint));
                ImGui::Text("Textures: ");
                ImGui::Image(mat->albedo->getID(), ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::SameLine();
                ImGui::Image(mat->normal->getID(), ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));

                ImGui::TreePop();
            }

            ImGui::PopID();
            meshIndex++;
        }
        ImGui::SeparatorText("Shadows");
        ImGui::DragFloat3("Light Direction", glm::value_ptr(scene.lightDir), 0.1f, -1.0f, 1.0f);
        ImGui::Image(renderer.getDepthTex(), ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();

        ImGui::Render();

        // ---- Render Code ----
        renderer.render(scene, settings);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // ---- SHUTDOWN CODE ----
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}