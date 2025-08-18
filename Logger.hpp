#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <Singleton.hpp>

class Logger : public Singleton<Logger> {
    friend class Singleton<Logger>; // (CRTP) curiously recurring template pattern, needed because Singleton<T> creates T, in this case Logger, but Logger constructor is private, so Singleton<Logger> need to access it
 
public:
   enum class LogLevel {
        DEBUG   = 0U,
        INFO    = 1U,
        WARNING = 2U,
        ERROR   = 3U
    };
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
    void config(const std::string& file, bool logToFile = false, bool logToConsole = true, LogLevel logLevel = Logger::LogLevel::INFO);    
    bool isLogToConsole() const noexcept {return _logToConsole;};
    void setLogToConsole(bool logToConsole) noexcept {_logToConsole = logToConsole;};
    bool isLogToFile() const noexcept {return _logToFile;};
    void setLogToFile(bool logToFile) noexcept {_logToFile = logToFile;};
    LogLevel getLogLevel() const noexcept {return _logLevel;};
    void setLogLevel(LogLevel logLevel) noexcept {_logLevel = logLevel;};
    void setLogFile(const std::string& file);


private:
    Logger() : 
        commonFmt(""),
        infoFmt("[INFO] "),
        warnFmt("[WARN] "),
        errorFmt("[ERROR] "),
        debugFmt("[DEBUG] "),
        _logLevel(Logger::LogLevel::ERROR) {
            //enabling exception
            logFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        };
    std::ofstream logFile;
    std::string commonFmt; //TODO: create function to create format, like timestamp, function calling, filename ecc
    std::string debugFmt;
    std::string infoFmt;
    std::string warnFmt;
    std::string errorFmt;
    LogLevel _logLevel;
    bool _logToConsole = true;
    bool _logToFile    = false;
    std::recursive_mutex _mtx;
    const std::string buildFinalMsg(const std::string& msg, const std::string& fmt) const;
    void logToFile(const std::string& msg);
    void logToConsole(const std::string& msg);
    void openLogFile(const std::string& file);

    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    };
};

