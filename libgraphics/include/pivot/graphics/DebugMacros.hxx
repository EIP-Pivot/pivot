#pragma once

#include <Logger.hpp>

#ifdef NDEBUG
    #define DEBUG_FUNCTION
#else
    #define DEBUG_FUNCTION logger LOGGER_ACCESS trace(::function_name()) << "Entered";
#endif
