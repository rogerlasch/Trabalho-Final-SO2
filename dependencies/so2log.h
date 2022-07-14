

//Apenas uma biblioteca b�sica de log.
//Ela usa a biblioteca de formata��o fmt, que se tornou padr�o c++ na vers�o 20.

#ifndef SO2LOG_H
#define SO2LOG_H

#include"fmt/core.h"

void log_write(const std::string& s);
#define _log(str, ...) log_write(fmt::format(str, __VA_ARGS__))
#endif
