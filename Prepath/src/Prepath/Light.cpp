#include "Light.h"
#include "Error.h"
#include <iostream>

namespace Prepath
{
    PointLight::~PointLight()
    {
        if (m_DepthFramebuffer)
        {
            glDeleteFramebuffers(1, &m_DepthFramebuffer);
            m_DepthFramebuffer = 0;
        }

        if (m_DepthCubemap)
        {
            glDeleteTextures(1, &m_DepthCubemap);
            m_DepthCubemap = 0;
        }

        if (m_DepthTextureFace)
        {
            glDeleteTextures(1, &m_DepthTextureFace);
            m_DepthTextureFace = 0;
        }
    }

    GLenum PointLight::copyCubemapFaceToTexture(GLenum face)
    {
        GLuint tempFBO;
        glGenFramebuffers(1, &tempFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, face, m_DepthCubemap, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            PREPATH_LOG_ERROR("ERROR: Temp FBO for cubemap face copy is not complete!");
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &tempFBO);
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, m_DepthTextureFace);

        glCopyTexSubImage2D(
            GL_TEXTURE_2D,          // destination
            0,                      // mip level
            0, 0,                   // xoffset, yoffset in dest
            0, 0,                   // x, y from src
            PREPATH_SHADOWMAP_SIZE, // width
            PREPATH_SHADOWMAP_SIZE  // height
        );

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &tempFBO);

        return m_DepthTextureFace;
    }

    void PointLight::setupLight()
    {
        glGenTextures(1, &m_DepthTextureFace);
        glBindTexture(GL_TEXTURE_2D, m_DepthTextureFace);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     PREPATH_SHADOWMAP_SIZE, PREPATH_SHADOWMAP_SIZE,
                     0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenTextures(1, &m_DepthCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubemap);

        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_DEPTH_COMPONENT,
                PREPATH_SHADOWMAP_SIZE, PREPATH_SHADOWMAP_SIZE,
                0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glGenFramebuffers(1, &m_DepthFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_DepthFramebuffer);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemap, 0);

        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            PREPATH_LOG_ERROR("ERROR: PointLight shadow framebuffer is not complete!");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    std::shared_ptr<PointLight> Prepath::Light::generatePointLight()
    {
        auto light = std::make_shared<PointLight>();
        light->setupLight();
        return light;
    }
}