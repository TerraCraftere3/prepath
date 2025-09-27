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

#define DEMO_IMPORT_SPONZA          // Sponza
#define DEMO_IMPORT_SPONZA_CURTAINS // Curtains Sponza Addon
// #define DEMO_IMPORT_SPONZA_TREES    // Tree Sponza Addon
// #define DEMO_IMPORT_SPONZA_IVY      // Ivy Sponza Addon
// #define DEMO_IMPORT_SAN_MIGUEL      //
#define DEMO_IMPORT_ERATO // Statue
// #define DEMO_IMPORT_DRAGON // Dragon
// #define DEMO_IMPORT_GALLERY // Gallery

void printExtension(const std::string &name, int indent = 1)
{
    PREPATH_LOG_INFO("{}> {}", std::string(indent, '    '), name);
}

void printExtensions()
{
    PREPATH_LOG_INFO("Loaded extension into client: ");
    printExtension("Scenes", 1);
#ifdef DEMO_IMPORT_SPONZA
    printExtension("Sponza Base", 2);
#ifdef DEMO_IMPORT_SPONZA_CURTAINS
    printExtension("Sponza Curtains", 3);
#endif
#ifdef DEMO_IMPORT_SPONZA_TREES
    printExtension("Sponza Ivy", 3);
#endif
#ifdef DEMO_IMPORT_SPONZA_IVY
    printExtension("Sponza Tree", 3);
#endif
#endif
#ifdef DEMO_IMPORT_SAN_MIGUEL
    printExtension("San Miguel", 2);
#endif
#ifdef DEMO_IMPORT_ERATO
    printExtension("Erato", 2);
#endif
#ifdef DEMO_IMPORT_DRAGON
    printExtension("Dragon", 2);
#endif
#ifdef DEMO_IMPORT_GALLERY
    printExtension("Gallery", 2);
#endif
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

    printExtensions();

    // ---- DEMO SETUP ----
    auto renderer = Prepath::Renderer();
    auto scene = Prepath::Scene();
    std::vector<std::string> faces{
        "models/textures/right.jpg",
        "models/textures/left.jpg",
        "models/textures/top.jpg",
        "models/textures/bottom.jpg",
        "models/textures/front.jpg",
        "models/textures/back.jpg"};
    scene.skybox = loadSkybox(faces);
    auto settings = Prepath::RenderSettings();
    settings.culling = false;
    settings.cam.Position = glm::vec3(0, 1.5f, 4.0f);
    settings.cam.updateCameraVectors();

#ifdef DEMO_IMPORT_SPONZA
    bool showSponza = true;
    auto sponza_meshes = loadModelWithCache("models/NewSponza_Main_glTF_003.gltf");
    for (auto mesh : sponza_meshes)
    {
        mesh->modelMatrix = glm::rotate(mesh->modelMatrix, glm::radians(-90.0f), glm::vec3(.0f, 1.0f, .0f));
        mesh->hidden = !showSponza;
        scene.addMesh(mesh);
    }

#ifdef DEMO_IMPORT_SPONZA_CURTAINS
    bool showCurtains = true;
    auto curtains_meshes = loadModelWithCache("models/NewSponza_Curtains_glTF.gltf");
    for (auto mesh : curtains_meshes)
    {
        mesh->modelMatrix = glm::rotate(mesh->modelMatrix, glm::radians(-90.0f), glm::vec3(.0f, 1.0f, .0f));
        mesh->hidden = !showCurtains;
        scene.addMesh(mesh);
    }
#endif

#ifdef DEMO_IMPORT_SPONZA_IVY
    bool showIvy = false;
    auto ivy_meshes = loadModelWithCache("models/NewSponza_IvyGrowth_glTF.gltf");
    for (auto mesh : ivy_meshes)
    {
        mesh->modelMatrix = glm::rotate(mesh->modelMatrix, glm::radians(-90.0f), glm::vec3(.0f, 1.0f, .0f));
        mesh->hidden = !showIvy;
        scene.addMesh(mesh);
    }
#endif

#ifdef DEMO_IMPORT_SPONZA_TREES
    bool showTree = false;
    auto tree_meshes = loadModelWithCache("models/NewSponza_CypressTree_glTF.gltf");
    for (auto mesh : tree_meshes)
    {
        mesh->modelMatrix = glm::rotate(mesh->modelMatrix, glm::radians(-90.0f), glm::vec3(.0f, 1.0f, .0f));
        mesh->hidden = !showTree;
        scene.addMesh(mesh);
    }
#endif
#endif

#ifdef DEMO_IMPORT_SAN_MIGUEL
    bool showSanMiguel = false;
    auto san_miguel_meshes = loadModelWithCache("models/san-miguel.gltf");
    for (auto mesh : san_miguel_meshes)
    {
        mesh->hidden = !showSanMiguel;
        scene.addMesh(mesh);
    }
#endif

#ifdef DEMO_IMPORT_ERATO
    bool showErato = false;
    auto erato_meshes = loadModelWithCache("models/erato.obj");
    for (auto mesh : erato_meshes)
    {
        mesh->modelMatrix = glm::scale(mesh->modelMatrix, glm::vec3(0.2f));
        mesh->hidden = !showErato;
        scene.addMesh(mesh);
    }
#endif

#ifdef DEMO_IMPORT_GALLERY
    bool showGallery = false;
    auto gallery_meshes = loadModelWithCache("models/gallery.obj");
    for (auto mesh : gallery_meshes)
    {
        mesh->hidden = !showGallery;
        scene.addMesh(mesh);
    }
#endif

#ifdef DEMO_IMPORT_DRAGON
    bool showDragon = true;
    auto dragon_mat = Prepath::Material::generateMaterial();
    dragon_mat->albedo = loadTexture("models/textures/marble_0017_color_2k.jpg");
    dragon_mat->normal = loadTexture("models/textures/marble_0017_normal_opengl_2k.png");
    dragon_mat->roughness = loadTexture("models/textures/marble_0017_roughness_2k.jpg");
    dragon_mat->ao = loadTexture("models/textures/marble_0017_ao_2k.jpg");
    dragon_mat->tint = glm::vec3(152.0f / 255.0f, 241.0f / 255.0f, 115.0f / 255.0f);

    auto dragon_meshes = loadModelWithCache("models/StandfordDragon.obj");
    for (auto mesh : dragon_meshes)
    {
        mesh->material = dragon_mat;
        mesh->hidden = !showDragon;
        scene.addMesh(mesh);
    }
#endif

    scene.lightDir = glm::vec3(0.1f, 0.8f, 0.3f);

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
        ImGui::DragInt("Display Textures", &settings.showTexture, 0.1f, 0);
        ImGui::Checkbox("Culling", &settings.culling);
        ImGui::SeparatorText("Camera");
        ImGui::SliderFloat("Speed", &cameraController.moveSpeed, 10.0f, 50.0f);
        ImGui::Text("Yaw: %.1f, Pitch: %.1f", settings.cam.Yaw, settings.cam.Pitch);
        ImGui::Text("Position: %.1f, %.1f, %.1f", settings.cam.Position.x, settings.cam.Position.y, settings.cam.Position.z);
        ImGui::SeparatorText("Scene");
#ifdef DEMO_IMPORT_SPONZA
        if (ImGui::Checkbox("Show Sponza", &showSponza))
        {
            for (auto mesh : sponza_meshes)
                mesh->hidden = !showSponza;
            if (!showSponza)
            {
#ifdef DEMO_IMPORT_SPONZA_CURTAINS
                showCurtains = false;
                for (auto mesh : curtains_meshes)
                    mesh->hidden = true;
#endif
#ifdef DEMO_IMPORT_SPONZA_IVY
                showIvy = false;
                for (auto mesh : ivy_meshes)
                    mesh->hidden = true;
#endif
#ifdef DEMO_IMPORT_SPONZA_TREES
                showTree = false;
                for (auto mesh : tree_meshes)
                    mesh->hidden = true;
#endif
            }
            scene.updateBounds();
        }
        if (showSponza)
        {
#ifdef DEMO_IMPORT_SPONZA_CURTAINS
            if (ImGui::Checkbox("Show Sponza (Curtains)", &showCurtains))
            {
                for (auto mesh : curtains_meshes)
                    mesh->hidden = !showCurtains;
                scene.updateBounds();
            }
#endif
#ifdef DEMO_IMPORT_SPONZA_IVY
            if (ImGui::Checkbox("Show Sponza (Ivy)", &showIvy))
            {
                for (auto mesh : ivy_meshes)
                    mesh->hidden = !showIvy;
                scene.updateBounds();
            }
#endif
#ifdef DEMO_IMPORT_SPONZA_TREES
            if (ImGui::Checkbox("Show Sponza (Trees)", &showTree))
            {
                for (auto mesh : tree_meshes)
                    mesh->hidden = !showTree;
                scene.updateBounds();
            }
#endif
        }
#endif
#ifdef DEMO_IMPORT_DRAGON
        if (ImGui::Checkbox("Show Stanford Dragon", &showDragon))
        {
            for (auto mesh : dragon_meshes)
            {
                mesh->hidden = !showDragon;
                scene.updateBounds();
            }
        }
#endif
#ifdef DEMO_IMPORT_SAN_MIGUEL
        if (ImGui::Checkbox("Show San Miguel", &showSanMiguel))
        {
            for (auto mesh : san_miguel_meshes)
            {
                mesh->hidden = !showSanMiguel;
                scene.updateBounds();
            }
        }
#endif
#ifdef DEMO_IMPORT_ERATO
        if (ImGui::Checkbox("Show Erato", &showErato))
        {
            for (auto mesh : erato_meshes)
            {
                mesh->hidden = !showErato;
                scene.updateBounds();
            }
        }
#endif
#ifdef DEMO_IMPORT_GALLERY
        if (ImGui::Checkbox("Show Gallery", &showGallery))
        {
            for (auto mesh : gallery_meshes)
            {
                mesh->hidden = !showGallery;
                scene.updateBounds();
            }
        }
#endif
        int meshIndex = 0;
        for (auto &mesh : scene.getMeshes())
        {
            ImGui::PushID(meshIndex);

            std::stringstream ss;
            ss.imbue(std::locale(""));
            ss << mesh->getTriangleCount();

            if (mesh->hidden)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

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

            if (mesh->hidden)
                ImGui::PopStyleColor();

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