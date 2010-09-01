#ifndef LOGGER_H_
#define LOGGER_H_

#include <cstdio>
#include <set>

#define LOGE(fmt, ...) Logger::Inst().Write(mll::Logger::LOG_ERR, fmt, __VA_ARGS__)
#define LOGW(fmt, ...) Logger::Inst().Write(mll::Logger::LOG_WRN, fmt, __VA_ARGS__)
#define LOGI(fmt, ...) Logger::Inst().Write(mll::Logger::LOG_INF, fmt, __VA_ARGS__)
#define LOGD(fmt, ...) Logger::Inst().Write(mll::Logger::LOG_DBG, fmt, __VA_ARGS__)

#define LOGLEVEL(level) Logger::Inst().SetLevel(level)
#define LOGHANDLER(file) Logger::Inst().AddHandler(file)

namespace mll {

class Logger {
public:
	enum { LOG_ERR = 0x1, LOG_WRN = 0x2, LOG_INF = 0x4, LOG_DBG = 0x8, LOG_ALL = 0xf };
	static Logger& Inst();
	virtual ~Logger();
	void SetLevel(int level);
    int GetLevel() const;
    FILE* AddHandler(FILE* file);
	void Write(int level, const char* format, ...);
private:
        Logger();
	int level_;
        char buf_[1024];
        std::set<FILE*> handlers_;
};

}

#endif // LOGGER_H_
