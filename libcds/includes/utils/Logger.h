//
// Created by alberto on 9/23/15.
//

#ifndef ALL_LIBCDS_LOGGER_H
#define ALL_LIBCDS_LOGGER_H
#include <iostream>
#include <string>
#include <ctime>

using namespace std;

namespace cds_utils {
    template<typename T>class Logger{
    public:
        Logger(){}

        Logger(const string &file){
            outfile.open(file, ios_base::app);
        }
        ~Logger(){
            if (outfile.is_open()) {
                outfile.close();
            }
        }
        void INFO( const T &object,const string &description=""){
            if (outfile.is_open()) {
                outfile << "#[INFO: " << getTimeAndDate() << "] " << description << endl;
                outfile << object << endl;
            }
        }
        void WARN(const T &object, const string &description){
            if (outfile.is_open()) {
                outfile << "#[WARN: " << getTimeAndDate() << "] " << description << endl;
                outfile << object << endl;
            }
        }
        void ERROR(const T&object, const string &description){
            if (outfile.is_open()) {
                outfile << "#[ERROR: " << getTimeAndDate() << "] " << description << endl;
            }
        }

        Logger<T>& setFile(const string &file){
            if (outfile.is_open()){
                outfile.close();
            }
            if (!file.empty()) {
                outfile.open(file, ios_base::app);
            }
            return *this;
        }

    private:
        ofstream outfile;
        string getTimeAndDate(){
            time_t     now = time(0);
            struct tm  tstruct;
            char       buf[80];
            tstruct = *localtime(&now);
            // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
            // for more information about date/time format
            strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
            return string(buf);
        }
    };
}
#endif //ALL_LOGGER_H
