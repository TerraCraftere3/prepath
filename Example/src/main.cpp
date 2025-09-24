#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform vec3 uLightDir;

out float brightness;

void main() {
    vec3 normal = mat3(transpose(inverse(uModel))) * aNormal;
    brightness = max(dot(normalize(normal), normalize(-uLightDir)), 0.0);
    gl_Position = uMVP * vec4(aPos, 1.0);
})";

const char *fragmentShaderSource = R"(
#version 330 core
in float brightness;
out vec4 FragColor;
uniform vec3 uColor;
void main() {
    vec3 shaded = uColor * (0.2 + 0.8 * brightness);
    FragColor = vec4(shaded, 1.0);
})";

float vertices[] = {
    // positions        // normals
    -0.5f,-0.5f,-0.5f,  0.0f,  0.0f, -1.0f,
     0.5f,-0.5f,-0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, 0.5f,-0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, 0.5f,-0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f, 0.5f,-0.5f,  0.0f,  0.0f, -1.0f,
    -0.5f,-0.5f,-0.5f,  0.0f,  0.0f, -1.0f,

    -0.5f,-0.5f, 0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,-0.5f, 0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, 0.5f, 0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, 0.5f, 0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,-0.5f, 0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f, 0.5f, 0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, 0.5f,-0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,-0.5f,-0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,-0.5f,-0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,-0.5f, 0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, 0.5f, 0.5f, -1.0f,  0.0f,  0.0f,

     0.5f, 0.5f, 0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, 0.5f,-0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,-0.5f,-0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,-0.5f,-0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,-0.5f, 0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, 0.5f, 0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f,-0.5f,-0.5f,  0.0f, -1.0f,  0.0f,
     0.5f,-0.5f,-0.5f,  0.0f, -1.0f,  0.0f,
     0.5f,-0.5f, 0.5f,  0.0f, -1.0f,  0.0f,
     0.5f,-0.5f, 0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f,-0.5f, 0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f,-0.5f,-0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f, 0.5f,-0.5f,  0.0f, 1.0f,  0.0f,
     0.5f, 0.5f,-0.5f,  0.0f, 1.0f,  0.0f,
     0.5f, 0.5f, 0.5f,  0.0f, 1.0f,  0.0f,
     0.5f, 0.5f, 0.5f,  0.0f, 1.0f,  0.0f,
    -0.5f, 0.5f, 0.5f,  0.0f, 1.0f,  0.0f,
    -0.5f, 0.5f,-0.5f,  0.0f, 1.0f,  0.0f,
};

unsigned int createShader(unsigned int type, const char *source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

unsigned int createProgram(const char *vsSrc, const char *fsSrc)
{
    unsigned int vs = createShader(GL_VERTEX_SHADER, vsSrc);
    unsigned int fs = createShader(GL_FRAGMENT_SHADER, fsSrc);
    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int main()
{
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(800, 600, "Rotating Cube", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int shader = createProgram(vertexShaderSource, fragmentShaderSource);

    float angle = 0.0f;
    float speed = 1.0f;
    float color[3] = {1.0f, 0.5f, 0.2f};

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Controls");
        ImGui::SliderFloat("Speed", &speed, 0.0f, 5.0f);
        ImGui::ColorEdit3("Color", color);
        ImGui::End();

        ImGui::Render();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        angle += speed * 0.01f;

        glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 1.0f, 0.0f));
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 100.0f);
        glm::mat4 mvp = proj * view * model;

        glUseProgram(shader);
        glUniformMatrix4fv(glGetUniformLocation(shader, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform3fv(glGetUniformLocation(shader, "uColor"), 1, color);

        glm::vec3 lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f));
        glUniform3fv(glGetUniformLocation(shader, "uLightDir"), 1, glm::value_ptr(lightDir));
        glUniformMatrix4fv(glGetUniformLocation(shader, "uModel"), 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}