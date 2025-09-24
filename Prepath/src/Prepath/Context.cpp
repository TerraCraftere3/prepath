#include "Context.h"

namespace Prepath
{
    Context &Context::getGlobalContext()
    {
        static Context instance;
        return instance;
    }

    Context::Context()
    {
        m_Loggers[LogLevel::Info] = [](const std::string &msg)
        { std::cout << "[INFO] " << msg << "\n"; };
        m_Loggers[LogLevel::Warn] = [](const std::string &msg)
        { std::cout << "[WARN] " << msg << "\n"; };
        m_Loggers[LogLevel::Error] = [](const std::string &msg)
        { std::cerr << "[ERROR] " << msg << "\n"; };
        m_Loggers[LogLevel::Fatal] = [](const std::string &msg)
        { std::cerr << "[FATAL] " << msg << "\n"; };
    }

    void Context::log(LogLevel level, const std::string &msg)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        if (m_Loggers[level])
        {
            m_Loggers[level](msg);
        }
    };
}