#pragma once

#include <fstream>
#include <string>

enum Logging_Type : char{
    LOGGING_ERROR = 'E',
    LOGGING_WARNING = 'W',
    LOGGING_DEBUG = 'D',
    LOGGING_INFO = 'I',
    LOGGING_VERBOSE = 'V',
};


class Logging {
    private:
    bool log_to_console;
    bool log_to_file;
    std::string file_path;
    std::fstream file_fd;
    Logging() {}
    ~Logging() {}

    public:
    void Log(Logging_Type t, std::string tag, std::string msg);
    void Init();
    void Init(std::string log_file_path);

    static Logging& instance() {
        static Logging l;
        return l;
    }
};