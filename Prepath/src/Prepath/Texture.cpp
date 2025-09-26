#include "Texture.h"
#include "Error.h"

namespace Prepath
{
    Texture::Texture()
    {
        glGenTextures(1, &m_ID);
        glBindTexture(GL_TEXTURE_2D, m_ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    Texture::~Texture()
    {
        glDeleteTextures(1, &m_ID);
    }

    void Texture::setData(unsigned char *data, unsigned int width, unsigned int height, int channels)
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    std::shared_ptr<Texture> Texture::generateTexture(unsigned char *data, unsigned int width, unsigned int height, int channels)
    {
        auto tex = std::make_shared<Texture>();
        tex->setData(data, width, height, channels);
        return tex;
    }

}