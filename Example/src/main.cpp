#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>
#include "Prepath/Lib.h"

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

    // ---- DEMO CODE ----
    auto &ctx = Prepath::Context::getGlobalContext();

    {
        ctx.setLogger(Prepath::LogLevel::Info, [](const std::string &msg)
                      { spdlog::info("{}", msg); });
        ctx.setLogger(Prepath::LogLevel::Warn, [](const std::string &msg)
                      { spdlog::warn("{}", msg); });
        ctx.setLogger(Prepath::LogLevel::Error, [](const std::string &msg)
                      { spdlog::error("{}", msg); });
        ctx.setLogger(Prepath::LogLevel::Fatal, [](const std::string &msg)
                      { spdlog::critical("{}", msg); });
    }

    auto renderer = Prepath::Renderer();
    auto scene = Prepath::Scene();
    // ---- RUNTIME CODE ----
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({20, 20});
        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::Begin("Debug");
        ImGui::End();

        ImGui::Render();

        // ---- Render Code ----
        renderer.render(scene);

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