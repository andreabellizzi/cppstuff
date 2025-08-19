# cppstuff
Repository to store useful, maybe for me only, cpp stuff

## Logger class
Very simple logging class depending on Singleton.hpp, it allows print to console, print to file, and define different level of logging.
It is declared inside a namespace Logging, it defines a global Log variable so you can avoid to get always the instance but just include Logger.hpp.
How to use it:
Initialize the Logging::Log variable first and then use it everywhere is included Logger.hpp. E.g.
```
#include <Logger.hpp>

int main() {
    Logging::Log = &Logging::Logger::instance();
    Logging::Log->setLogToConsole(true);
    Logging::Log->setLogToFile(true);
    Logging::Log->setFormat("[%(level)][%(file):%(func):%(lineno)][%(time)]-%(msg)");
    Logging::Log->setLogLevel(Logging::Logger::LogLevel::WARNING);
    Logging::Log->setLogFile("trylog.txt");
    Logging::Log->info("try info");
    Logging::Log->error("try error");

    return 0;
}
```
> TODO: to add commonFmt handling, to specify format like timestamp, function name of calling, etc.
