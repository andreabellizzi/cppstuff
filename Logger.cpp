#include <Logger.hpp>
#include <exception>

namespace Logging {
    Logger* Log;
}

using namespace Logging;

std::string Logger::buildFinalMsg(const std::string& msg, const std::string& fmt) const 
{
    return fmt + msg + commonFmt;
}

void Logger::logToFile(const std::string &msg)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logToFile && logFile.is_open())
    {
        logFile << msg << std::endl;
    }
}

void Logger::logToConsole(const std::string &msg)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logToConsole)
    {
        std::cout << msg << std::endl;
    }
}

void Logger::debug(const std::string &msg)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logLevel <= Logger::LogLevel::DEBUG)
    {
        std::string finalMsg = buildFinalMsg(msg, debugFmt);
        logToConsole(finalMsg);
        logToFile(finalMsg);
    }
}

void Logger::info(const std::string &msg)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logLevel <= Logger::LogLevel::INFO)
    {
        std::string finalMsg = buildFinalMsg(msg, infoFmt);
        logToConsole(finalMsg);
        logToFile(finalMsg);
    }
}

void Logger::warn(const std::string &msg)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logLevel <= Logger::LogLevel::WARNING)
    {
        std::string finalMsg = buildFinalMsg(msg, warnFmt);
        logToConsole(finalMsg);
        logToFile(finalMsg);
    }
}

void Logger::error(const std::string &msg)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logLevel <= Logger::LogLevel::ERROR)
    {
        std::string finalMsg = buildFinalMsg(msg, errorFmt);
        logToConsole(finalMsg);
        logToFile(finalMsg);
    }
}

void Logger::openLogFile(const std::string &file)
{
    if (logFile.is_open()) {
        logFile.close();
    }
    logFile.open(file);
}

void Logger::config(const std::string &file, bool logToFile, bool logToConsole, Logger::LogLevel logLevel)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    openLogFile(file);
    _logToFile = logToFile;
    _logToConsole = logToConsole;
    _logLevel = logLevel;
}

void Logger::setLogFile(const std::string &file)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    openLogFile(file);
}


