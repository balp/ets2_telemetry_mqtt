/// C++ Wrapper of SCSLogging.
//
#ifndef SCSLOG_HPP
#define SCSLOG_HPP
#include <scssdk.h>
extern scs_log_t game_log;

class Logger {
    private:
        scs_log_t _game_log = NULL;
    public:
        Logger() {}
        void setGameLog(scs_log_t game_log) {
            _game_log = game_log;
        }
        void message(const char* msg) {
            if(game_log) {
                game_log(SCS_LOG_TYPE_message, msg);
            }
        }
        void warning(const char* msg) {
            if(game_log) {
                game_log(SCS_LOG_TYPE_warning, msg);
            }
        }
        void error(const char* msg) {
            if(game_log) {
                game_log(SCS_LOG_TYPE_error, msg);
            }
        }
};


#endif // SCSLOG_HPP
