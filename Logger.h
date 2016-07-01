
#ifndef LOGGER_H
#define LOGGER_H
#include <fstream>
#include <string>

class Logger {
public:
    /**
     * \param f The filename to write to
     */
    Logger(std::string f);

    /**
     * Flush the buffer to the file
     */
    void flush();

    std::ofstream file_stream;
};

void LOG(std::string msg);

Logger& default_log();
#endif
