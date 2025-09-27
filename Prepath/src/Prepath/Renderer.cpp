#include "Renderer.h"
#include "Error.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>

namespace Prepath
{
    Renderer::Renderer()
    {
        m_Shader = Shader::generateShader(PREPATH_READ_SHADER("default.vert").c_str(), PREPATH_READ_SHADER("default.frag").c_str());
        m_DepthShader = Shader::generateShader(PREPATH_READ_SHADER("depth.vert").c_str(), PREPATH_READ_SHADER("depth.frag").c_str());
        m_BoundsShader = Shader::generateShader(PREPATH_READ_SHADER("bounds.vert").c_str(), PREPATH_READ_SHADER("bounds.frag").c_str());
        m_SkyboxShader = Shader::generateShader(PREPATH_READ_SHADER("skybox.vert").c_str(), PREPATH_READ_SHADER("skybox.frag").c_str());

        m_BoundsMesh = Mesh::generateCube(0.5f);
        m_SkyboxMesh = Mesh::generateCube(1.0f);

        glGenFramebuffers(1, &m_DepthFBO);
        glGenTextures(1, &m_DepthTex);
        glBindTexture(GL_TEXTURE_2D, m_DepthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     PREPATH_SHADOWMAP_SIZE, PREPATH_SHADOWMAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTex, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::render(const Scene &scene, const RenderSettings &settings)
    {
        // Depth
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::mat4 view = settings.cam.getViewMatrix();
        glm::mat4 projection = settings.cam.getProjectionMatrix(float(settings.width) / settings.height);

        AABB worldBounds = scene.bounds; // Assuming scene has overall bounds

        // Calculate light space matrix based on scene bounds
        glm::vec3 sceneCenter = (worldBounds.min + worldBounds.max) * 0.5f;
        glm::vec3 sceneSize = worldBounds.max - worldBounds.min;

        // Position light at scene center + light direction offset
        float lightDistance = glm::length(sceneSize) * 0.5f; // Distance from scene center
        glm::vec3 lightPos = sceneCenter + glm::normalize(scene.lightDir) * lightDistance;

        glm::mat4 lightView = glm::lookAt(lightPos,
                                          sceneCenter, // Look at scene center
                                          glm::vec3(0.0f, 1.0f, 0.0f));

        // Create orthographic projection that encompasses the entire scene
        float maxExtent = glm::max(glm::max(sceneSize.x, sceneSize.y), sceneSize.z) * 0.5f;
        float padding = maxExtent * 0.1f; // 10% padding to avoid edge artifacts
        float orthoSize = maxExtent + padding;

        float near_plane = 0.1f;
        float far_plane = lightDistance + maxExtent + padding;

        glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize,
                                               -orthoSize, orthoSize,
                                               near_plane, far_plane);
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        // ---- SHADOWS ----
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glViewport(0, 0, PREPATH_SHADOWMAP_SIZE, PREPATH_SHADOWMAP_SIZE);
            glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            glFrontFace(GL_CCW);
            renderScene(scene, projection, view, lightSpaceMatrix, m_DepthShader);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // ---- SCENE ----
        {
            if (settings.culling)
            {
                glEnable(GL_CULL_FACE);
                glFrontFace(GL_CCW);
            }
            else
            {
                glDisable(GL_CULL_FACE);
            }
            if (settings.wireframe)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glViewport(0, 0, settings.width, settings.height);
            glCullFace(GL_BACK);
            renderScene(scene, projection, view, lightSpaceMatrix, m_Shader, settings.cam.Position, settings.showTexture);
        }

        // ---- BOUNDS ----
        if (settings.bounds)
        {
            m_BoundsShader->bind();
            m_BoundsShader->setUniformMat4f("uView", view);             // View Matrix
            m_BoundsShader->setUniformMat4f("uProjection", projection); // Projection Matrix
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            for (auto mesh : scene.getMeshes())
            {
                const AABB &bounds = mesh->bounds;

                glm::vec3 worldMin = glm::vec3(mesh->modelMatrix * glm::vec4(mesh->bounds.min, 1.0f));
                glm::vec3 worldMax = glm::vec3(mesh->modelMatrix * glm::vec4(mesh->bounds.max, 1.0f));

                glm::vec3 center = (worldMin + worldMax) * 0.5f;
                glm::vec3 size = (worldMax - worldMin);

                glm::mat4 model = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), size);
                m_BoundsShader->setUniformMat4f("uModel", model);

                m_BoundsMesh->draw();

                m_Statistics.drawCallCount += m_BoundsMesh->getDrawCallCount();
                m_Statistics.triangleCount += m_BoundsMesh->getTriangleCount();
                m_Statistics.vertexCount += m_BoundsMesh->getVertexCount();
            }
            glEnable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // ---- SKYBOX ----
        {
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);
            glDisable(GL_CULL_FACE);
            m_SkyboxShader->bind();
            glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
            m_SkyboxShader->setUniformMat4f("uView", viewNoTranslation);
            m_SkyboxShader->setUniformMat4f("uProjection", projection);
            glBindTexture(GL_TEXTURE_CUBE_MAP, scene.skybox->getID());
            m_SkyboxMesh->draw();
            glDepthMask(GL_TRUE);
            m_Statistics.drawCallCount += m_SkyboxMesh->getDrawCallCount();
            m_Statistics.triangleCount += m_SkyboxMesh->getTriangleCount();
            m_Statistics.vertexCount += m_SkyboxMesh->getVertexCount();
        }
    }

    void Renderer::renderScene(const Scene &scene, const glm::mat4 &projection,
                               const glm::mat4 &view, const glm::mat4 &lightSpace,
                               std::shared_ptr<Shader> shader, const glm::vec3 &uCameraPos, int uDebugTexture)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader->bind();
        shader->setUniformMat4f("uView", view);             // View Matrix
        shader->setUniformMat4f("uProjection", projection); // Projection Matrix
        shader->setUniformMat4f("uLightSpace", lightSpace); // Light Space Matrix (LView * LProjection)
        shader->setUniform3f("uLightDir", scene.lightDir);  // Light Dir [-1->1]

        m_Statistics.drawCallCount = 0;
        m_Statistics.triangleCount = 0;
        m_Statistics.vertexCount = 0;

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_DepthTex);
        shader->setUniform1i("uDepthMap", 0);
        shader->setUniform1i("uDebugTexture", uDebugTexture);
        shader->setUniform3f("uCameraPos", uCameraPos);

        for (auto mesh : scene.getMeshes())
        {
            shader->setUniformMat4f("uModel", mesh->modelMatrix);
            glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(mesh->modelMatrix)));
            shader->setUniformMat3f("uNormalMatrix", normalMatrix);
            if (mesh->material)
            {
                auto mat = mesh->material;
                shader->setUniform3f("uTint", mat->tint);

                glActiveTexture(GL_TEXTURE0 + 1);
                glBindTexture(GL_TEXTURE_2D, mat->albedo->getID());
                shader->setUniform1i("uAlbedoMap", 1);

                glActiveTexture(GL_TEXTURE0 + 2);
                glBindTexture(GL_TEXTURE_2D, mat->normal->getID());
                shader->setUniform1i("uNormalMap", 2);

                glActiveTexture(GL_TEXTURE0 + 3);
                glBindTexture(GL_TEXTURE_2D, mat->roughness->getID());
                shader->setUniform1i("uRoughnessMap", 3);

                glActiveTexture(GL_TEXTURE0 + 4);
                glBindTexture(GL_TEXTURE_2D, mat->metal->getID());
                shader->setUniform1i("uMetallicMap", 4);

                glActiveTexture(GL_TEXTURE0 + 5);
                glBindTexture(GL_TEXTURE_2D, mat->ao->getID());
                shader->setUniform1i("uAOMap", 5);
            }
            mesh->draw();
            m_Statistics.drawCallCount += mesh->getDrawCallCount();
            m_Statistics.triangleCount += mesh->getTriangleCount();
            m_Statistics.vertexCount += mesh->getVertexCount();
        }
    }

    RenderSettings::RenderSettings()
    {
    }
} // namespace Prepath