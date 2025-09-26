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
    int width = 0, height = 0, nrChannels = 0;
    PREPATH_LOG_INFO("Loading texture {}", path.c_str());

    // ---- FILE EXTENSIONS ----
    std::string ext;
    size_t dot = path.find_last_of('.');
    if (dot != std::string::npos)
        ext = path.substr(dot + 1);

    // ---- PNG IMAGES ----
    if (ext == "png" || ext == "PNG")
    {
        std::vector<unsigned char> image;
        unsigned w, h;
        unsigned error = lodepng::decode(image, w, h, path);

        if (error)
        {
            PREPATH_LOG_FATAL("LodePNG failed to load {}: {}", path.c_str(), lodepng_error_text(error));
            unsigned char fallback[3] = {255, 0, 255};
            return Texture::generateTexture(fallback, 1, 1, 3);
        }

        return Texture::generateTexture(image.data(), w, h, 4); // RGBA
    }

    // ---- ANY IMAGE ----
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        auto tex = Texture::generateTexture(data, width, height, nrChannels);
        stbi_image_free(data);
        return tex;
    }
    else
    {
        PREPATH_LOG_FATAL("stb_image failed to load {}!!!", path.c_str());
        unsigned char fallback[3] = {255, 0, 255};
        return Texture::generateTexture(fallback, 1, 1, 3);
    }
}
const char *getTextureTypeName(aiTextureType type)
{
    switch (type)
    {
    case aiTextureType_DIFFUSE:
        return "Diffuse";
    case aiTextureType_SPECULAR:
        return "Specular";
    case aiTextureType_AMBIENT:
        return "Ambient";
    case aiTextureType_EMISSIVE:
        return "Emissive";
    case aiTextureType_HEIGHT:
        return "Height";
    case aiTextureType_NORMALS:
        return "Normals";
    case aiTextureType_SHININESS:
        return "Shininess";
    case aiTextureType_OPACITY:
        return "Opacity";
    case aiTextureType_DISPLACEMENT:
        return "Displacement";
    case aiTextureType_LIGHTMAP:
        return "Lightmap";
    case aiTextureType_REFLECTION:
        return "Reflection";
    default:
        return "Unknown";
    }
}

void printMaterialTextures(aiMaterial *aiMat)
{
    if (!aiMat)
        return;

    // Array of all texture types you might want to check
    aiTextureType types[] = {
        aiTextureType_DIFFUSE,
        aiTextureType_SPECULAR,
        aiTextureType_AMBIENT,
        aiTextureType_EMISSIVE,
        aiTextureType_HEIGHT,
        aiTextureType_NORMALS,
        aiTextureType_SHININESS,
        aiTextureType_OPACITY,
        aiTextureType_DISPLACEMENT,
        aiTextureType_LIGHTMAP,
        aiTextureType_REFLECTION,
        aiTextureType_BASE_COLOR,
        aiTextureType_NORMAL_CAMERA,
        aiTextureType_EMISSION_COLOR,
        aiTextureType_METALNESS,
        aiTextureType_DIFFUSE_ROUGHNESS,
        aiTextureType_AMBIENT_OCCLUSION,
        aiTextureType_UNKNOWN};

    for (auto type : types)
    {
        unsigned int count = aiMat->GetTextureCount(type);
        if (count > 0)
        {
            PREPATH_LOG_INFO("Texture type {} has {} texture(s):", getTextureTypeName(type), count);
            for (unsigned int i = 0; i < count; ++i)
            {
                aiString path;
                if (aiMat->GetTexture(type, i, &path) == AI_SUCCESS)
                {
                    PREPATH_LOG_INFO("\t{}", path.C_Str());
                }
            }
        }
    }
}

// VERY SKETCHY METHOD
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

    // Group vertex data by material
    struct MeshData
    {
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec3> bitangents;
    };
    std::unordered_map<aiMaterial *, MeshData> groupedMeshes;

    for (unsigned int m = 0; m < scene->mNumMeshes; m++)
    {
        aiMesh *mesh = scene->mMeshes[m];
        aiMaterial *aiMat = scene->mMaterials[mesh->mMaterialIndex];

        MeshData &data = groupedMeshes[aiMat];

        data.positions.reserve(data.positions.size() + mesh->mNumVertices);
        data.normals.reserve(data.normals.size() + mesh->mNumVertices);
        data.texCoords.reserve(data.texCoords.size() + mesh->mNumVertices);
        data.tangents.reserve(data.tangents.size() + mesh->mNumVertices);
        data.bitangents.reserve(data.bitangents.size() + mesh->mNumVertices);

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            data.positions.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            data.tangents.emplace_back(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            data.bitangents.emplace_back(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);

            if (mesh->HasNormals())
                data.normals.emplace_back(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            else
                data.normals.emplace_back(0.0f, 0.0f, 1.0f);

            if (mesh->HasTextureCoords(0))
                data.texCoords.emplace_back(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            else
                data.texCoords.emplace_back(0.0f, 0.0f);
        }
    }

    std::vector<std::shared_ptr<Mesh>> meshes;

    // Generate one mesh per material
    for (auto &[aiMat, data] : groupedMeshes)
    {
        auto prepathMesh = Mesh::generateMesh(
            data.positions, data.normals, data.texCoords,
            &data.tangents, &data.bitangents);

        std::shared_ptr<Material> mat;

        // Material caching
        auto it = materialCache.find(aiMat);
        if (it != materialCache.end())
        {
            mat = it->second;
        }
        else
        {
            mat = Material::generateMaterial();
            printMaterialTextures(aiMat);

            if (aiMat)
            {
                aiString texPath;
                if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
                    mat->albedo = loadTexture(texPath.C_Str());

                if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &texPath) == AI_SUCCESS ||
                    aiMat->GetTexture(aiTextureType_HEIGHT, 0, &texPath) == AI_SUCCESS)
                    mat->normal = loadTexture(texPath.C_Str());
            }

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

    auto sponza_meshes = loadModel("sponza.obj");
    for (auto mesh : sponza_meshes)
    {
        mesh->modelMatrix = glm::rotate(mesh->modelMatrix, glm::radians(90.0f), glm::vec3(.0f, 1.0f, .0f));
        mesh->modelMatrix = glm::scale(mesh->modelMatrix, glm::vec3(0.05f));
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