#pragma once
#include <string>
#include <memory>
#include <mutex>
#include <format>
#include <glad/glad.h>

#include "Context.h"

namespace Prepath
{
    class Texture
    {
    public:
        Texture();
        ~Texture();
        void setData(unsigned char *data, unsigned int width, unsigned int height, int channels);
        unsigned int getID() { return m_ID; }

        static std::shared_ptr<Texture> generateTexture(unsigned char *data, unsigned int width, unsigned int height, int channels = 4);

    private:
        unsigned int m_ID;
        unsigned int m_Width;
        unsigned int m_Height;
        unsigned int m_Channels;
    };

}