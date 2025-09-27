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

#include "cache.h"

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
    std::vector<std::string> faces{
        "textures/right.jpg",
        "textures/left.jpg",
        "textures/top.jpg",
        "textures/bottom.jpg",
        "textures/front.jpg",
        "textures/back.jpg"};
    scene.skybox = loadSkybox(faces);
    auto settings = Prepath::RenderSettings();
    settings.cam.Position = glm::vec3(0, 3.0f, 4.0f);
    settings.cam.Pitch = 45;
    settings.cam.updateCameraVectors();

    auto sponza_meshes = loadModelWithCache("NewSponza_Main_glTF_003.gltf");
    for (auto mesh : sponza_meshes)
    {
        mesh->modelMatrix = glm::rotate(mesh->modelMatrix, glm::radians(90.0f), glm::vec3(.0f, 1.0f, .0f));
        scene.addMesh(mesh);
    }
    auto curtains_meshes = loadModelWithCache("NewSponza_Curtains_glTF.gltf");
    for (auto mesh : curtains_meshes)
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
            ImVec2(128 * 3.3f, settings.height - 40),
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
        ImGui::InputInt("Display Textures", &settings.showTexture, 0.5f, 0, 6);
        ImGui::Checkbox("Culling", &settings.culling);
        ImGui::SeparatorText("Camera");
        ImGui::SliderFloat("Speed", &cameraController.moveSpeed, 10.0f, 50.0f);
        ImGui::Text("Yaw: %.1f, Pitch: %.1f", settings.cam.Yaw, settings.cam.Pitch);
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
                ImGui::SameLine();
                ImGui::Image(mat->roughness->getID(), ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));

                ImGui::Image(mat->metal->getID(), ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));
                ImGui::SameLine();
                ImGui::Image(mat->ao->getID(), ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0));

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