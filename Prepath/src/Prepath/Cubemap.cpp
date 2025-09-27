#include "Cubemap.h"
#include "Error.h"

namespace Prepath
{
    Cubemap::Cubemap()
    {
        glGenTextures(1, &m_ID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    Cubemap::~Cubemap()
    {
        glDeleteTextures(1, &m_ID);
    }

    void Cubemap::setData(unsigned char *data[6], unsigned int width, unsigned int height, int channels)
    {
        m_Width = width;
        m_Height = height;
        m_Channels = channels;

        GLenum format = GL_RGB;
        if (channels == 1)
            format = GL_RED;
        else if (channels == 3)
            format = GL_RGB;
        else if (channels == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

        for (unsigned int i = 0; i < 6; i++)
        {
            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                format,
                width,
                height,
                0,
                format,
                GL_UNSIGNED_BYTE,
                data[i]);
        }

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    std::shared_ptr<Cubemap> Cubemap::generateTexture(unsigned char *data[6], unsigned int width, unsigned int height, int channels)
    {
        auto tex = std::make_shared<Cubemap>();
        tex->setData(data, width, height, channels);
        return tex;
    }

}
