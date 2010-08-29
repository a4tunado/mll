#include "logger.h"

#include <cassert>
#include <stdarg.h>

using namespace mll;

Logger& Logger::Inst() {
    static Logger singleton;
    return singleton;
}

Logger::Logger() : level_(LOG_ALL) { }

Logger::~Logger() {
    for (std::set<FILE*>::iterator it = handlers_.begin()
        ; it != handlers_.end()
        ; ++it) {
        fclose(*it);
    }
}

void Logger::SetLevel(int level) {
    level_ = level;
}

int Logger::GetLevel() const {
    return level_;
}

void Logger::AddHandler(FILE* file) {
    if (handlers_.find(file) == handlers_.end()) {
        handlers_.insert(file);
    }
}
void Logger::Write(int level, const char* format, ...) {
    if (!(level & level_)) return;

    char* ptr = buf_;

    switch (level) {
        case LOG_ERR: ptr += sprintf(ptr, "ERR: "); break;
        case LOG_WRN: ptr += sprintf(ptr, "WRN: "); break;
        case LOG_INF: ptr += sprintf(ptr, "INF: "); break;
        default /*LOG_DBG*/: ptr += sprintf(ptr, "DBG: "); break;        
    }

    va_list args;
    va_start(args, format);
    ptr += vsprintf(ptr, format, args);
    va_end(args);

    *ptr = '\n'; *++ptr = '\0';

    assert((ptr - buf_) < sizeof(buf_));

    for (std::set<FILE*>::iterator it = handlers_.begin()
        ; it != handlers_.end()
        ; ++it) {
        fwrite(buf_, sizeof(*buf_), ptr - buf_, *it);
        fflush(*it);
    }
}

