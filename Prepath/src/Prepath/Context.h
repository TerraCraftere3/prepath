#pragma once
#include <iostream>
#include <functional>
#include <string>
#include <memory>
#include <mutex>
#include <filesystem>
#include <fstream>
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
            std::lock_guard<std::mutex> lock(m_LoggerMutex);
            m_Loggers[level] = std::move(func);
        }

        // ---- Path Methods ----
        void setShaderPath(const std::string &path)
        {
            std::lock_guard<std::mutex> lock(m_ShaderPathMutex);
            std::filesystem::path cwd = std::filesystem::current_path();
            m_ShaderPath = (cwd / path).string();
        }

        std::string getShaderPath()
        {
            std::lock_guard<std::mutex> lock(m_ShaderPathMutex);
            return m_ShaderPath;
        }

        std::string getShaderPath(const std::string &inputName)
        {
            std::lock_guard<std::mutex> lock(m_ShaderPathMutex);
            return (std::filesystem::path(m_ShaderPath) / inputName).string();
        }

        std::string readShader(const std::string &inputName)
        {
            std::string filePath = getShaderPath(inputName);
            std::ifstream file(filePath, std::ios::in | std::ios::binary);
            if (!file)
            {
                throw std::runtime_error("Failed to open shader file: " + filePath);
            }

            std::ostringstream contents;
            contents << file.rdbuf();
            return contents.str();
        }

    private:
        Context();

        void log(LogLevel level, const std::string &msg);

        std::mutex m_ShaderPathMutex;
        std::string m_ShaderPath = "/NO_PATH";
        std::mutex m_LoggerMutex;
        std::unordered_map<LogLevel, LogFunction> m_Loggers;
    };

}

#define PREPATH_LOG_INFO(fmt, ...) ::Prepath::Context::getGlobalContext().info(std::format(fmt, ##__VA_ARGS__))
#define PREPATH_LOG_WARN(fmt, ...) ::Prepath::Context::getGlobalContext().warn(std::format(fmt, ##__VA_ARGS__))
#define PREPATH_LOG_ERROR(fmt, ...) ::Prepath::Context::getGlobalContext().error(std::format(fmt, ##__VA_ARGS__))
#define PREPATH_LOG_FATAL(fmt, ...) ::Prepath::Context::getGlobalContext().fatal(std::format(fmt, ##__VA_ARGS__))

#define PREPATH_GET_SHADER(path) ::Prepath::Context::getGlobalContext().getShaderPath(path)
#define PREPATH_READ_SHADER(path) ::Prepath::Context::getGlobalContext().readShader(path)
#define PREPATH_GENERATE_SHADERVF(vertexPath, fragmentPath) Shader::generateShader(PREPATH_READ_SHADER(vertexPath).c_str(), PREPATH_READ_SHADER(fragmentPath).c_str())
#define PREPATH_GENERATE_SHADERVGF(vertexPath, geometryPath, fragmentPath) Shader::generateShader(PREPATH_READ_SHADER(vertexPath).c_str(), PREPATH_READ_SHADER(geometryPath).c_str(), PREPATH_READ_SHADER(fragmentPath).c_str())