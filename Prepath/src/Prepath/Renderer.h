#pragma once
#include <iostream>
#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <format>
#include <glad/glad.h>

#include "Context.h"
#include "Scene.h"
#include "Camera.h"
#include "Shader.h"

#define PREPATH_SHADOWMAP_SIZE 1024 * 4

namespace Prepath
{
    struct RenderSettings
    {
        int width = 800;
        int height = 600;
        bool wireframe = false;
        bool culling = true;
        bool bounds = false;
        int showTexture = 0; // 0 = normal render, >0 = debug view
        Camera cam;
        RenderSettings();
    };

    struct RenderStatistics
    {
        int drawCallCount = 0;
        int vertexCount = 0;
        int triangleCount = 0;
    };

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();
        void renderGizmo(std::shared_ptr<Texture> texture, const glm::vec3 &position, const glm::vec3 &tint = glm::vec3(1.0f));
        void renderGizmoSphere(const glm::vec3 &position, const float &size, const glm::vec3 &tint = glm::vec3(1.0f));
        void render(const Scene &scene, const RenderSettings &settings);
        void renderScene(const Scene &scene, const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &lightSpace, std::shared_ptr<Shader> shader, const glm::vec3 &uCameraPos = glm::vec3(0.0f), int uDebugTexture = 0);
        unsigned int getDepthTex() { return m_DepthTex; }
        RenderStatistics getStatistics() { return m_Statistics; }

    private:
        RenderStatistics m_Statistics;
        std::shared_ptr<Texture> m_WhiteTex;
        std::shared_ptr<Shader> m_Shader;
        std::shared_ptr<Shader> m_DepthShader;
        std::shared_ptr<Shader> m_BoundsShader;
        std::shared_ptr<Shader> m_SkyboxShader;
        std::shared_ptr<Shader> m_GizmoShader;
        std::shared_ptr<Mesh> m_BoundsMesh;
        std::shared_ptr<Mesh> m_SkyboxMesh;
        std::shared_ptr<Mesh> m_GizmoMesh;
        std::shared_ptr<Mesh> m_SphereMesh;
        unsigned int m_DepthFBO;
        unsigned int m_DepthTex;
        glm::mat4 m_LastView;
        glm::mat4 m_LastProjection;
    };

}