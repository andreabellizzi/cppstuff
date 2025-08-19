#include <Logger.hpp>
#include <exception>
#include <stdexcept>
#include <chrono>
#include <ctime>
#include <iomanip>


namespace Logging {
    Logger* Log;
}

using namespace Logging;

Logger::Logger() : 
    commonFmt("[%(level)] - %(msg)"),
    debugFmt("DEBUG"),
    infoFmt("INFO"),
    warnFmt("WARN"),
    errorFmt("ERROR"),
    timeFmt("%Y-%m-%d %H:%M:%S"),
    _logLevel(Logger::LogLevel::ERROR) 
{
    //enabling exception
    logFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    __parseFormat(commonFmt);
}

Logger::~Logger()
{
    if (logFile.is_open()) {
        logFile.close();
    }
};

std::string Logger::__buildFinalMsg(const std::string& msg, const std::string& levelFmt, const std::source_location& location) const 
{

    std::string finalString;
    std::vector<FmtId>::const_iterator fmtIter = _fmtToUseVec.begin();

    for (const std::string& iter : _userFmtVec) 
    {
        if (iter == "%")
        {
            finalString += __fromFmt2String(*fmtIter, msg, levelFmt, location);
            ++fmtIter;
        }
        else
        {
            finalString += iter;
        }
    }

    return finalString;
}

void Logger::__logToFile(const std::string &msg)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logToFile && logFile.is_open())
    {
        logFile << msg << std::endl;
    }
}

void Logger::__logToConsole(const std::string &msg)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logToConsole)
    {
        std::cout << msg << std::endl;
    }
}

void Logger::debug(const std::string &msg, const std::source_location& location)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logLevel <= Logger::LogLevel::DEBUG)
    {
        std::string finalMsg = __buildFinalMsg(msg, debugFmt, location);
        __logToConsole(finalMsg);
        __logToFile(finalMsg);
    }
}

void Logger::info(const std::string &msg, const std::source_location& location)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logLevel <= Logger::LogLevel::INFO)
    {
        std::string finalMsg = __buildFinalMsg(msg, infoFmt, location);
        __logToConsole(finalMsg);
        __logToFile(finalMsg);
    }
}

void Logger::warn(const std::string &msg, const std::source_location& location)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logLevel <= Logger::LogLevel::WARNING)
    {
        std::string finalMsg = __buildFinalMsg(msg, warnFmt, location);
        __logToConsole(finalMsg);
        __logToFile(finalMsg);
    }
}

void Logger::error(const std::string &msg, const std::source_location& location)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    if (_logLevel <= Logger::LogLevel::ERROR)
    {
        std::string finalMsg = __buildFinalMsg(msg, errorFmt, location);
        __logToConsole(finalMsg);
        __logToFile(finalMsg);
    }
}

void Logger::log(const LogLevel level, const std::string &msg, const std::source_location& location)
{
    // generic log function
    switch(level)
    {
        case LogLevel::DEBUG:   debug(msg, location); break;
        case LogLevel::INFO:    info(msg, location); break;
        case LogLevel::WARNING: warn(msg, location); break;
        case LogLevel::ERROR:   error(msg, location); break;
        default: break;
    }
}

void Logger::__openLogFile(const std::string &file)
{
    if (logFile.is_open()) {
        logFile.close();
    }
    logFile.open(file);
}

void Logger::config(const std::string &file, bool logToFile, bool logToConsole, Logger::LogLevel logLevel)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    __openLogFile(file);
    _logToFile = logToFile;
    _logToConsole = logToConsole;
    _logLevel = logLevel;
}

void Logger::setLogFile(const std::string &file)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    __openLogFile(file);
}

void Logger::__parseFormat(const std::string &fmt)
{
    enum class ParseState {USER_FMT, MSG_FMT, OPEN_BRAC};
    ParseState state = ParseState::USER_FMT;
    _fmtToUseVec.clear();
    _userFmtVec.clear();
    std::string tmpFmt;

    for (char c : fmt) 
    {
        switch (state) 
        {
        case ParseState::USER_FMT:
        {
            if (c != '%')
            {
                tmpFmt += c;
            }
            else
            {
                // add user message got until now
                _userFmtVec.push_back(tmpFmt);
                _userFmtVec.push_back("%");
                tmpFmt.clear(); //clear, we'll use it as buffer for got msg format %(<msg format>)
                state = ParseState::OPEN_BRAC;
            }
        }
        break;
        case ParseState::OPEN_BRAC:
        {
            if (c != '(')
            {
                throw std::invalid_argument("error during parsing format, ( not found after %, bad format: " + fmt);
            }
            else
            {
                state = ParseState::MSG_FMT;
            }
        }
        break;
        case ParseState::MSG_FMT:
        {
            if (c != ')')
            {
                if (tmpFmt.size() < _max_fmt_param_char)
                {
                    tmpFmt += c;
                }
                else
                {
                    throw std::invalid_argument("error during parsing format, unrecognized format param: " + tmpFmt);
                }
            }
            else
            {
                //what format is?
                FmtId _fmt = __string2fmtid(tmpFmt);
                if (_fmt != FmtId::NO_FMT)
                {
                    _fmtToUseVec.push_back(_fmt);
                }
                else
                {
                    throw std::invalid_argument("error during parsing format, unrecognized format param: " + tmpFmt);
                }
                tmpFmt.clear();
                state = ParseState::USER_FMT;
            }
        }
        break;
        default:
        //unreachable
        break;
        }
    }

    if (_fmtToUseVec.size() == 0)
    {
        throw std::invalid_argument("error during parsing format, at least %(msg) need to be used: " + fmt);
    }
}

Logger::FmtId Logger::__string2fmtid(const std::string &fmt)
{
    // like python format
    // %(msg) -> print the message
    // %(level) -> print the message level
    // %(file) -> print the file name
    // %(lineno) -> print the file line number
    // %(func) -> print the func name
    // %(time) -> print the time in timeformat
    for (const auto& pair : _fmtIdMap)
    {
        if (pair.second == fmt)
        {
            return pair.first;
        }
    }
    return FmtId::NO_FMT;
}

std::string Logger::__fromFmt2String(FmtId fmtId, const std::string &msg, const std::string& levelFmt, const std::source_location &location) const
{
    //TODO: do it better
    std::string timeStr;
    if(fmtId == FmtId::TIME) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm = *std::localtime(&now_c);
        std::ostringstream oss;
        oss << std::put_time(&local_tm, timeFmt.c_str());
        timeStr = oss.str();
    }

    switch (fmtId)
    {
    case FmtId::MSG:
        return msg;
    case FmtId::LEVEL:
        return levelFmt;
    case FmtId::FILE:
        return location.file_name();
    case FmtId::LINENO:
        return std::to_string(location.line());
    case FmtId::FUNC:
        return location.function_name();
    case FmtId::TIME:
        return timeStr;
    default:
        break;
    }
    return std::string();
}

void Logger::setFormat(const std::string &fmt)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    commonFmt = fmt;
    __parseFormat(fmt);
}
