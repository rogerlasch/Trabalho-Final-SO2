


#ifndef SO2LOG_H
#define SO2LOG_H

#include"fmt/core.h"

void log_write(const std::string& s);
#define _log(str, ...) log_write(fmt::format(str, __VA_ARGS__))
#endif
