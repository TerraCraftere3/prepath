#pragma once
#include <iostream>
#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <format>

namespace Prepath
{

    using LogFunction = std::function<void(const std::string &)>;

    enum class LogLevel
    {
        Info,
        Warn,
        Error,
        Fatal
    };

    class Context
    {
    public:
        // ---- Context Methods ----
        static Context &getGlobalContext();

        // ---- Logging Methods ----
        void info(const std::string &msg) { log(LogLevel::Info, msg); }
        void warn(const std::string &msg) { log(LogLevel::Warn, msg); }
        void error(const std::string &msg) { log(LogLevel::Error, msg); }
        void fatal(const std::string &msg) { log(LogLevel::Fatal, msg); }

        void setLogger(LogLevel level, LogFunction func)
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Loggers[level] = std::move(func);
        }

    private:
        Context();

        void log(LogLevel level, const std::string &msg);

        std::mutex m_Mutex;
        std::unordered_map<LogLevel, LogFunction> m_Loggers;
    };

}

#define PREPATH_LOG_INFO(fmt, ...) Context::getGlobalContext().info(std::format(fmt, ##__VA_ARGS__))
#define PREPATH_LOG_WARN(fmt, ...) Context::getGlobalContext().warn(std::format(fmt, ##__VA_ARGS__))
#define PREPATH_LOG_ERROR(fmt, ...) Context::getGlobalContext().error(std::format(fmt, ##__VA_ARGS__))
#define PREPATH_LOG_FATAL(fmt, ...) Context::getGlobalContext().fatal(std::format(fmt, ##__VA_ARGS__))