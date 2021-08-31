#pragma once

#include <Logger.hpp>

#ifdef NDEBUG
#define DEBUG_FUNCTION
#else
#define DEBUG_FUNCTION logger LOGGER_ACCESS debug(__PRETTY_FUNCTION__) << "Entered", LOGGER_ENDL
#endif
