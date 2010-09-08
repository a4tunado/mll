#ifndef LOGGER_H_
#define LOGGER_H_

#include <cstdio>
#include <set>
#include <exception>

#define LOGE(...) Logger::Inst().Write(mll::Logger::LOG_ERR, __VA_ARGS__)
#define LOGW(...) Logger::Inst().Write(mll::Logger::LOG_WRN, __VA_ARGS__)
#define LOGI(...) Logger::Inst().Write(mll::Logger::LOG_INF, __VA_ARGS__)
#define LOGD(...) Logger::Inst().Write(mll::Logger::LOG_DBG, __VA_ARGS__)

#define LOGF(...) do { LOGE(__VA_ARGS__); throw std::runtime_error(""); } while (0)

#define LOGLEV(lev) Logger::Inst().SetLevel(lev)
#define LOGHDR(hdr) Logger::Inst().AddHandler(hdr)

#define LOGSTR(str) ((str).c_str())

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
