#include "Logging.h"

#include <ctime>
#include <iostream>
#include <fstream>

using namespace std;
void Logging::Log(Logging_Type t, std::string tag, std::string msg) {


    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    time (&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer,80,"%d-%m-%Y %H:%M:%S",timeinfo);
    std::string strTime(buffer);


    if (log_to_console) {
        cout << "[" << strTime << "] (" << char(t) << ") " << tag << ": " << msg << endl; 
    }

    if(log_to_file && file_fd.is_open()) {
        file_fd << '[' << strTime << "] (" << char(t) << ") " << tag << ": " << msg << endl;
    }
}

void Logging::Init() {
    log_to_console = true;
    log_to_file = false;
}

void Logging::Init(std::string log_file_path) {
    log_to_console = true;
    log_to_file = true;
    file_path = log_file_path;
    cout << "Opening log file " << file_path << endl;
    file_fd.open (file_path, std::fstream::out | std::fstream::app);
    if (!file_fd.is_open()) {
        cout << "Unable to open log file." << endl;
    }
}