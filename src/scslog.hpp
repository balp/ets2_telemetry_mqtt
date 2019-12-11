/// C++ Wrapper of SCSLogging.
//
#ifndef SCSLOG_HPP
#define SCSLOG_HPP
#include <scssdk.h>
extern scs_log_t game_log;

class Logger {
    private:
        scs_log_t _game_log;
    public:
        Logger() = default;
        void setGameLog(scs_log_t gameLog) {
            _game_log = gameLog;
        }
        void message(const char* msg) {
            if(_game_log) {
                _game_log(SCS_LOG_TYPE_message, msg);
            }
        }
        void warning(const char* msg) {
            if(_game_log) {
                _game_log(SCS_LOG_TYPE_warning, msg);
            }
        }
        void error(const char* msg) {
            if(_game_log) {
                _game_log(SCS_LOG_TYPE_error, msg);
            }
        }
};


#endif // SCSLOG_HPP
