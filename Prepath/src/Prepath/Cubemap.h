#pragma once
#include <string>
#include <memory>
#include <mutex>
#include <format>
#include <glad/glad.h>

#include "Context.h"

namespace Prepath
{
    class Cubemap
    {
    public:
        Cubemap();
        ~Cubemap();
        void setData(unsigned char *data[6], unsigned int width, unsigned int height, int channels);
        unsigned int getID() { return m_ID; }

        static std::shared_ptr<Cubemap> generateTexture(unsigned char *data[6], unsigned int width, unsigned int height, int channels = 4);

    private:
        unsigned int m_ID;
        unsigned int m_Width;
        unsigned int m_Height;
        unsigned int m_Channels;
    };

}