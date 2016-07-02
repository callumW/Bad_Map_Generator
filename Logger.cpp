#include "Logger.h"
#include <stdexcept>

Logger::Logger(std::string f)
{
    file_stream.open(f, std::ofstream::out|std::ofstream::trunc);

    if (!file_stream) {
        throw std::runtime_error("Failed to open log");
    }
}

void Logger::flush()
{
    file_stream.flush();
}

void LOG(std::string msg)
{
    default_log().file_stream << msg << '\n';
    default_log().flush();
}

Logger& default_log()
{
    static Logger lg{"generate.log"};
    return lg;
}
