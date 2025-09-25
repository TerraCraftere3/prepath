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

std::shared_ptr<Prepath::Mesh> loadModel(std::string path)
{
    using namespace Prepath;
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_FlipUVs);

    if (!scene || !scene->HasMeshes())
    {
        std::cerr << "Assimp error loading " << path << ": "
                  << importer.GetErrorString() << std::endl;
        return nullptr;
    }

    aiMesh *mesh = scene->mMeshes[0]; // take first mesh for now

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;

    positions.reserve(mesh->mNumVertices);
    normals.reserve(mesh->mNumVertices);
    texCoords.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        positions.emplace_back(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z);

        if (mesh->HasNormals())
        {
            normals.emplace_back(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z);
        }
        else
        {
            normals.emplace_back(0.0f, 0.0f, 1.0f);
        }

        if (mesh->HasTextureCoords(0))
        {
            texCoords.emplace_back(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y);
        }
        else
        {
            texCoords.emplace_back(0.0f, 0.0f);
        }
    }

    return Mesh::generateMesh(positions, normals, texCoords);
}

int main()
{
    // ---- INIT CODE ----
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(800, 600, "Prepath Demo", nullptr, nullptr);
    glfwMakeContextCurrent(window);
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
    settings.cam.Position = glm::vec3(0, 0, 6.0f);
    settings.cam.updateCameraVectors();

    auto mat = Prepath::Material::createMaterial();
    mat->tint = glm::vec3(1.0f);

    auto cube = loadModel("teapot.obj");
    cube->modelMatrix = glm::scale(cube->modelMatrix, glm::vec3(0.5f));
    cube->modelMatrix = glm::translate(cube->modelMatrix, glm::vec3(0.0f, -1.5f, 0.0f));
    cube->material = mat;

    auto floor = Prepath::Mesh::generateQuad();
    floor->modelMatrix = glm::translate(floor->modelMatrix, glm::vec3(.0f, -1.0f, .0f));
    floor->modelMatrix = glm::scale(floor->modelMatrix, glm::vec3(50.0f));
    floor->material = mat;

    scene.addMesh(cube);
    scene.addMesh(floor);
    scene.lightDir = glm::vec3(1.0f, 1.0f, -1.0f);

    // ---- RUNTIME CODE ----
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        glfwPollEvents();
        glfwGetWindowSize(window, &settings.width, &settings.height);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({20, 20});
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("Debug");
        ImGui::SeparatorText("Statistics");
        auto stats = renderer.getStatistics();
        ImGui::Text("Draw Calls: %d", stats.drawCallCount);
        ImGui::Text("Triangles: %d", stats.triangleCount);
        ImGui::Text("Vertices: %d", stats.vertexCount);
        ImGui::Text("Delta Time: %.3f ms", deltaTime);
        ImGui::SeparatorText("Settings");
        ImGui::Checkbox("Wireframe", &settings.wireframe);
        ImGui::Checkbox("Culling", &settings.culling);
        ImGui::SeparatorText("Shadows");
        ImGui::Image(renderer.getDepthTex(), ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();

        ImGui::Render();

        // ---- Render Code ----
        cube->modelMatrix = glm::rotate(cube->modelMatrix, deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
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