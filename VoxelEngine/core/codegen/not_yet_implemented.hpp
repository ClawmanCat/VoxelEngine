#pragma once

#include <exception>
#include <string>


#define VE_NOT_YET_IMPLEMENTED \
throw std::runtime_error { std::string { "Method " } + __func__ + " is not yet implemented." };