#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <source_location>
#include <vector>
#include <map>
#include <Singleton.hpp>

namespace Logging {

class Logger; //forward declaration to declare extern global Log at the start of header and make it more visible
extern Logger* Log;

class Logger : public Singleton<Logger> {
    friend class Singleton<Logger>; // (CRTP) curiously recurring template pattern, needed because Singleton<T> creates T, in this case Logger, but Logger constructor is private, so Singleton<Logger> need to access it
 
public:
   enum class LogLevel {
        DEBUG   = 0U,
        INFO    = 1U,
        WARNING = 2U,
        ERROR   = 3U
    };

    void debug(const std::string& msg, const std::source_location& location = std::source_location::current());
    void info(const std::string& msg, const std::source_location& location = std::source_location::current());
    void warn(const std::string& msg, const std::source_location& location = std::source_location::current());
    void error(const std::string& msg, const std::source_location& location = std::source_location::current());
    void log(const LogLevel level, const std::string& msg, const std::source_location& location = std::source_location::current());
    void config(const std::string& file, bool logToFile = false, bool logToConsole = true, LogLevel logLevel = Logger::LogLevel::INFO);    
    bool isLogToConsole() const noexcept {return _logToConsole;};
    void setLogToConsole(bool logToConsole) noexcept {_logToConsole = logToConsole;};
    bool isLogToFile() const noexcept {return _logToFile;};
    void setLogToFile(bool logToFile) noexcept {_logToFile = logToFile;};
    LogLevel getLogLevel() const noexcept {return _logLevel;};
    void setLogLevel(LogLevel logLevel) noexcept {_logLevel = logLevel;};
    void setLogFile(const std::string& file);
    void setFormat(const std::string& fmt);

private:

    Logger();
    ~Logger();
    // like python format
    // %(msg) -> print the message
    // %(level) -> print the message level
    // %(file) -> print the file name
    // %(lineno) -> print the file line number
    // %(func) -> print the func name
    // %(time) -> print the time in timeformat
    enum class FmtId {
        MSG = 0,
        LEVEL,
        FILE,
        LINENO,
        FUNC,
        TIME,
        NO_FMT
    };
    std::ofstream logFile;
    std::string commonFmt; // TODO: create function to create format, like timestamp, function calling, filename ecc
    std::string debugFmt;
    std::string infoFmt;
    std::string warnFmt;
    std::string errorFmt;
    std::string timeFmt;
    LogLevel _logLevel;
    bool _logToConsole = true;
    bool _logToFile    = false;
    std::recursive_mutex _mtx;
    std::vector<std::string> _userFmtVec;
    std::map<FmtId, std::string> _fmtIdMap = {
        {FmtId::MSG, "msg"},
        {FmtId::LEVEL, "level"},
        {FmtId::FILE, "file"},
        {FmtId::LINENO, "lineno"},
        {FmtId::FUNC, "func"},
        {FmtId::TIME, "time"}
    };
    std::vector<FmtId> _fmtToUseVec;
    const unsigned int _max_fmt_param_char = sizeof("lineno") - 1; //no line terminator
    std::string __buildFinalMsg(const std::string& msg, const std::string& levelFmt, const std::source_location& location) const;
    void __logToFile(const std::string& msg);
    void __logToConsole(const std::string& msg);
    void __openLogFile(const std::string& file);
    void __parseFormat(const std::string& fmt);
    FmtId __string2fmtid(const std::string& fmt);
    std::string __fromFmt2String(FmtId fmtId, const std::string& msg, const std::string& levelFmt, const std::source_location& location) const;
};

}
