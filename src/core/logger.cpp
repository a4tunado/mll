#include "logger.h"

#include <ctime>
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
        ; it != handlers_.end(); ++it) {
        fclose(*it);
    }
}

void Logger::SetLevel(int level) {
    level_ = level;
}

int Logger::GetLevel() const {
    return level_;
}

FILE* Logger::AddHandler(FILE* file) {
    if (handlers_.find(file) == handlers_.end()) {
        handlers_.insert(file);
		return file;
    }
	return NULL;
}

void Logger::Write(int level, const char* format, ...) {
    if (!(level & level_)) return;

    char* ptr = buf_;
	time_t timer = time(NULL);

    ptr += strftime(ptr, sizeof(buf_)
					, "%y.%m.%d %H:%M:%S "
					, gmtime(&timer));

	if (level & LOG_ERR)           { ptr += sprintf(ptr, "ERR: "); }
	else if (level & LOG_WRN)      { ptr += sprintf(ptr, "WRN: "); }
    else if (level & LOG_INF)      { ptr += sprintf(ptr, "INF: "); }
    else /*if (level & LOG_DBG)*/  { ptr += sprintf(ptr, "DBG: "); }

    va_list args;
    va_start(args, format);
    ptr += vsprintf(ptr, format, args);
    va_end(args);

    *ptr = '\n'; *++ptr = '\0';

    assert((ptr - buf_) < sizeof(buf_));

    for (std::set<FILE*>::iterator it = handlers_.begin()
        ; it != handlers_.end(); ++it) {
        fwrite(buf_, sizeof(*buf_), ptr - buf_, *it);
        fflush(*it);
    }
}

